#!/usr/bin/env python 
# -*- coding: utf-8 -*-
#
# Miville
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Miville is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Miville.  If not, see <http://www.gnu.org/licenses/>.
"""
Milhouse engine and stream.
"""
import warnings

from miville.streams import constants
from miville.streams import conf
from miville.streams import tools
from miville.streams import proc 
from miville.streams import base
from miville import devices
from miville.errors import * # StreamError
from miville.utils import log
from miville.utils import sig

log = log.start("debug", True, True, "streams.milhouse") 

VERBOSE = False
VERY_VERBOSE = False
VERBOSE_ERROR = True
TEST_PROFILE = 0 # defined below
PORT_OFFSET = 0 # overriden in api. 

class MilhouseProcessManager(proc.ProcessManager):
    """
    Override output parsing for Milhouse process manager.
    """
    def __init__(self, name="default", log_max_size=100, command=None, verbose=False, process_protocol_class=proc.ManagedProcessProtocol, env=None):
        """ Override from base class so that we can install signals here """
        # FIXME: might be overkill to override this? would it be ok just to have problem_signal in the base class?
        proc.ProcessManager.__init__(self, name=name, log_max_size=log_max_size, 
                command=command, verbose=verbose, process_protocol_class=process_protocol_class, env=env)
        self.problem_signal = sig.Signal()
        self.crashed_signal = sig.Signal()
        # Someone higher up is connecting to these to let the UI know what's up
        self.rtcp_sender_started_signal = sig.Signal()
        self.rtcp_sender_connected_signal = sig.Signal()
        self.rtcp_receiver_connected_signal = sig.Signal()
        self.milhouse_exitted_signal = sig.Signal()

    def _parse_for_errors(self, data):
        """ Parse for problems in milhouse output. """
        for txt in ("CRITICAL", "ERROR", "error while loading shared libraries"):
            if data.find(txt) != -1:
                problem = tools.StreamerProblem(constants.STATE_ERROR, data)
                log.error(problem)
                # LocalMilhouseStreamer handles this signal in method on_process_event
                self.problem_signal(problem) 
        if data.find("WARNING") != -1:
            log.warning(data)

    def _parse_for_rtcp_messages(self, data):
        """ Parse for rtcp messages in milhouse output """
        for txt in ("BITRATE", "OCTETS-SENT", "PACKETS-SENT"):
            if data.find(txt) != -1:
                # messages split by colons, get last element
                val = data.split(':')[-1]
                #log.debug("Got RTCP SENDER-STARTED message %s:%s" % (txt, val))
                self.rtcp_sender_started_signal(txt + ':' + val) 
        for txt in ("PACKETS-LOST", "JITTER"):
            if data.find(txt) != -1:
                val = data.split(':')[-1]
                #log.debug("Got RTCP SENDER-CONNECTED message %s:%s" % (txt, val))
                self.rtcp_sender_connected_signal(txt + ':' + val) 
        for txt in ("PACKETS-RECEIVED", "OCTETS-RECEIVED"):
            if data.find(txt) != -1:
                val = data.split(':')[-1]
                #log.debug("Got RTCP RECEIVER-CONNECTED message %s:%s" % (txt, val))
                self.rtcp_receiver_connected_signal(txt + ':' + val) 

    def _parse_for_exit(self, data):
        """ This looks at milhouse's output to see if it exitted """
        for txt in ("Leaving Milhouse", "Exitting Milhouse"):
            if data.find(txt) != -1:
                log.warning("MILHOUSE HAS LEFT THE BUILDING, DOING NOTHING FOR NOW")

    def _on_out_received(self, data):
        """
        Overrides tools.ProcessManager._on_out_received
        """
        # TODO: signal when output matches milhouse problem
        # FIXME: these should be separate methods, i.e. parse_for_error, parse_for_rtcp
        log.debug("stdout: (%s): %s" % (self.name, data))
        self._parse_for_errors(data)
        self._parse_for_rtcp_messages(data)
        self._parse_for_exit(data)
        self.stdout_logger.append(data.strip()) # important
    
class MilhouseFactory(base.BaseFactory):
    """
    Provides Milhouse streams.
    See miville.streams.interfaces.IService for the documentation.
    """
    name = "milhouse" # Name of the service
    enabled = True
    
    def __init__(self):
        # TODO: move streams  to session.
        self.streams = {} # Dict of current streams. 
        #The ID of the contact_info should be their keys.
        self.config_db = None # Configuration client.
        self.config_fields = {} 
        self.ports_allocator = tools.PortsAllocator(minimum=10000 + PORT_OFFSET, increment=10) 
        # 10 is the minimum permitted interval between milhouse.cpp ports.

    def config_init(self, config_db):
        #TODO: store config fields
        self.config_db = config_db
        setup_milhouse_fields(config_db)
        setup_milhouse_settings(config_db)
    
    def prepare_session(self, session):
        """
        Prepares the config entries for the alice and the bob.
        
        This is called from the streams manager.
        :param session: SessionDescription
        """
        role = session.role
        log.debug("MilhouseFactory.prepare_session(role=%s)" % (role))
        
        service = self
        alice_entries = session.get_stream(service, constants.DIRECTION_TO_OFFERER, 0).entries
        bob_entries = session.get_stream(service, constants.DIRECTION_TO_ANSWERER, 0).entries
        contact_infos = session.contact_infos
        
        if role == constants.ROLE_OFFERER:
            #"alice": 
            p1, p2  = self.ports_allocator.allocate_many(2)
            alice_entries["/both/audio/port"] = p1 
            alice_entries["/both/video/port"] = p2
            log.info("Allocating ports: %d %d" % (p1, p2))
            p3, p4 = 0, 0 # bob is responsible for his ports allocations
            bob_entries["/both/audio/port"] = p3 
            bob_entries["/both/video/port"] = p4
            alice_addr = contact_infos.local_addr
            bob_addr = contact_infos.remote_addr
            alice_entries["/send/network/remote_address"] = alice_addr
            alice_entries["/recv/network/remote_address"] = bob_addr
            bob_entries["/send/network/remote_address"] = bob_addr
            bob_entries["/recv/network/remote_address"] = alice_addr
            log.debug("using addresses %s %s" % (alice_addr, bob_addr))
        else:
            log.debug("Allocate ports for bob.")
            p3, p4  = self.ports_allocator.allocate_many(2) # bob allocates his ports
            bob_entries["/both/audio/port"] = p3
            bob_entries["/both/video/port"] = p4
        
    def _make_key(self, contact_infos, mode):
        # TODO: move to session.
        return "%s(%s)" % (contact_infos.get_id(), mode)

    def start(self, session):
        """
        Starts streamers of the milhouse service.
        
        Here, we start both local sender and local receiver.
        (starting 2 streams)
        
        Return a deferred
        """
        if len(devices.managers["audio"].drivers["jackd"].devices) == 0:
            raise StreamError("Cannot start, jackd is not running")

        # TODO: check if already in a session with remote
        role = session.role
        contact_infos = session.contact_infos
        service = self
        to_alice = session.get_stream(service, constants.DIRECTION_TO_OFFERER, 0)
        to_bob = session.get_stream(service, constants.DIRECTION_TO_ANSWERER, 0)
    
        log.debug("Offerer ports: %s. Answerer ports: %s" % (
            conf.path_glob(to_alice.entries, "port", conf.GLOB_CONTAINS),
            conf.path_glob(to_bob.entries, "port", conf.GLOB_CONTAINS)))
        # checking if bob's ports are valid
        answerer_ports = conf.path_glob(to_bob.entries, "port", conf.GLOB_CONTAINS)
        for port in answerer_ports.itervalues():
            if port == 0:
                raise StreamError("Answerer has not set his port numbers.")
        # creating and starting the streamers
        #log.debug("STARTING. Mode: %s. Config entries ports: %s" % (mode, conf.path_glob(config_entries, "port", conf.GLOB_CONTAINS)))
        to_alice.start()
        to_bob.start()
    
    def _on_streamer_crashed(self, streamer_identifier):
        # TODO: move to session.
        """
        :param key: str
        :param streamer_identifier: str
        """
        log.debug("Milhouse streamer %s crashed !" % (streamer_identifier))

    def _clear_streams_with_contact(self, session): #contact_infos):
        # TODO: move to session.
        """
        Deletes the streams for a contact.
        Must be called when error occur, of when closing a stream.
        """
        contact_infos = session.contact_infos
        for mode in ["recv", "send"]:
            key = self._make_key(contact_infos, mode)
            if self.streams.has_key(key):
                stream = self.streams[key]
                state = stream.state 
                if mode == "recv":
                    audio_port = stream.entries["/both/audio/port"]
                    video_port = stream.entries["/both/video/port"]
                    for portnum in (audio_port, video_port):
                        try:
                            self.ports_allocator.free(portnum)
                        except tools.PortsAllocatorError, e:
                            log.error(e.message)
                if state in [constants.STATE_IDLE, constants.STATE_FAILED, constants.STATE_STOPPED]:
                    # see miville.streams.constants.STATE_*
                    log.debug("deleting milhouse stream %s" % (key))
                    del self.streams[key]
                else: # STREAMING
                    # TODO: d = self.streams[key].stop(); d.addCallback.... 
                    msg = "Cannot delete milhouse stream %s. Its state is %s." % (key, state)
                    log.error("_clear_streams_with_contact: " + msg)
                    raise StreamError(msg) # !!!

    def list_streams_for_contact(self, session):
        # TODO: move to session.
        """
        Returns a list of stream instances for a contact.
        :param session: Session instance.
        """
        contact_infos = session.contact_infos
        ret = []
        for mode in ["send", "recv"]:
            key = self._make_key(contact_infos, mode)
            if self.streams.has_key(key):
                ret.append(self.streams[key])
        return ret
        
    def stop(self, session):
        """
        stop stream
        """
        contact_infos = session.contact_infos
        # at this stage, a message was already sent to remote host.
        contact_id = contact_infos.get_id()
        contact_name = contact_infos.get_contact_name()

        sender = session.get_senders()[0]
        receiver = session.get_receivers()[0]
        try:
            sender.stop()
        except proc.ManagedProcessError, e:
            log.warning(e.message)
        try:
            receiver.stop()
        except proc.ManagedProcessError, e:
            log.warning(e.message)
        #sender.process.milhouse_exitted_signal()
        #receiver.process.milhouse_exitted_signal()

class MilhouseStream(base.Stream):
    """
    Milhouse stream
    See miville.streams.interfaces.IStream
    """
    def __init__(self, direction=None, service=None, identifier=None, session=None, mode=None, entries=None):
        """
        :param identifier: unique ID.
        :param service: MilhouseFactory instance.
        :param session: Session instance.
        """
        base.Stream.__init__(self, session=session, direction=direction, entries=entries) # call parent constructor
        self.name = 'milhouse'
        self.service = service
        self.identifier = identifier
        self.state = constants.STATE_IDLE # see miville.streams.constants
        self.process = None
        
    def _on_crashed(self, value=None):
        """
        Slot to the MilhouseProcessManager.crashed_signal
        """
        log.error("Milhouse crashed.")
        self.state = constants.STATE_FAILED 
        self.crashed_signal(self.identifier)
        #self.service.stop(self.contact_infos) # sends a message 
        txt = self.process.stdout_logger.get_text()
        txt += self.process.stderr_logger.get_text()
        log.debug(txt) # XXX very verbose !

    def start(self):
        """
        Starts the streamer.
        This is where the milhouse command line is created.
        :param mode: either STREAMER_SENDER or STREAMER_RECEIVER.
        """
        self.state = constants.STATE_STARTING
        args = ["milhouse"]
        process_codename = "%s(%s)" % (self.name, self.mode)
        all_args = config_fields_to_milhouse_opts(self.entries)
        # reads what config_fields_to_milhouse_opts returned.
        if self.mode == "send":
            the_key = "sender"
            env_key = "sender_env"
        else:
            the_key = "receiver"
            env_key = "receiver_env"
        log.info("%s$ milhouse %s" % (process_codename, all_args[the_key]))
        args = all_args[the_key].split()
        try:
            contact_name = self.session.contact_infos.contact.name
        except AttributeError, e:
            pass
        else:
            # set window title
            args.append("-W")
            window_title = str(contact_name)
            for forbidden in "()\"'*!@#$%^&-=+\\|/~`":
                window_title = window_title.replace(forbidden, "")
            args.append(window_title)
        # add env vars
        environment_variables = all_args[env_key]
        log.debug("Environment variables : %s" % (environment_variables))
        self.process = MilhouseProcessManager(name=process_codename, command=args, env=environment_variables)

        # connect process rtcp signals to our session's corresponding handlers
        self.process.rtcp_sender_started_signal.connect(self.session.on_rtcp_sender_started)
        self.process.rtcp_sender_connected_signal.connect(self.session.on_rtcp_sender_connected)
        self.process.rtcp_receiver_connected_signal.connect(self.session.on_rtcp_receiver_connected)
        self.process.milhouse_exitted_signal.connect(self.session.on_milhouse_exitted)
        self.process.exitted_itself_signal.connect(self.session.on_milhouse_exitted_itself)
        self.process.crashed_signal.connect(self._on_crashed)
        self.process.problem_signal.connect(self.session.on_problem)

        self.process.start()
    
    def stop(self):
        """
        Stops the streamer.
        """
        def _cb(result, self):
            self.state = constants.STATE_STOPPED
            return result
        process_codename = "%s(%s)" % (self.name, self.mode)
        log.info("%s stop()" % (process_codename))
        if self.process is not None:
            self.process.stop()
            self.process.milhouse_exitted_signal()
        else:
            log.warning("No milhouse process to stop")


# -------------------- functions -------------

def config_fields_to_milhouse_opts(fields):
    """
    Creates the command to start both Milhouse processes.
    A sender and receiver pair.
    :return: dict with keys "sender" and "receiver". Its values are strings.
    """
    global VERBOSE
    #TODO: return dict with list values instead of strings
    ret = {
        "sender":"milhouse --sender", 
        "receiver":"milhouse --receiver",
        "sender_env":{},
        "receiver_env":{},
    }
    opts_mapping = {
        "/both/video/codec":"videocodec", 
        "/both/video/port":"videoport", 
        "/both/audio/codec":"audiocodec", 
        "/both/audio/port":"audioport", 
        "/send/video/source":"videosource", # TODO: map the values too
        "/send/audio/source":"audiosource", # TODO: map the values too
        "/recv/audio/sink":"audiosink", # TODO: map the values too
        "/recv/video/sink":"videosink", # TODO: map the values too
        "/both/audio/numchannels":"numchannels", 
        "/recv/video/deinterlace":"deinterlace",
        "/send/video/bitrate":"videobitrate",
        # "/send/audio/bitrate":"audiobitrate",
        "/recv/network/jitterbuffer":"jitterbuffer",
        "/send/network/remote_address":"address",
        "/recv/network/remote_address":"address",
        "/recv/audio/buffer_usec":"audio-buffer-usec",
        "/send/video/v4l2/device":"videodevice",
        "/recv/video/fullscreen":"fullscreen",
        "/recv/video/display":"display",
        "/send/audio/file_location":"audiolocation",
        "/send/video/file_location":"videolocation",
        # "/send/audio/jack/device":"device", # TODO: map the values too
    }
    
    for field_name, value in fields.iteritems():
        for_receiver = True
        for_sender = True
        add_it = True
        if field_name not in opts_mapping.keys():
            # raise ConfError("Field not in Milhouse flags mapping : " + str(field_name))
            #if VERBOSE:
            warnings.warn("WARNING: Field not in Milhouse flags mapping : " + str(field_name))
        else:
            if field_name.startswith("/recv/"):
                for_sender = False
            elif field_name.startswith("/send/"):
                for_receiver = False
            # else it starts with "/both/"
            # get rid of useless fields.
            if field_name == "/recv/audio/buffer_usec":
                add_it = False
            if field_name == "/send/video/v4l2/device" and fields["/send/video/source"] != "v4l2":
                add_it = False
            if field_name == "/send/audio/file_location" and fields["/send/audio/source"] != "filesrc":
                add_it = False
            if field_name == "/send/video/file_location" and fields["/send/video/source" ]!= "filesrc":
                add_it = False
            if field_name == "/send/video/bitrate":
                add_it = False
            # add it for both sender and receiver
            if type(value) is bool: # bool flags are given with no attribute
                if value:
                    value = ''
                else:
                    add_it = False
            if field_name in ["/recv/network/jitterbuffer", "/recv/video/display"]: #XXX for now
                add_it = False
            if add_it:
                flag = opts_mapping[field_name]
                if for_sender:
                    ret["sender"] += " --%s %s" % (flag, value) # TODO: map the values too
                if for_receiver:
                    ret["receiver"] += " --%s %s" % (flag, value) # TODO: map the values too
            # ---------------- env variables
            if field_name == "/recv/video/display":
                ret["receiver_env"]["DISPLAY"] = value
    return ret # dict

# ----------------------------- The rest is just configuration fields, settings and profiles. ---

def setup_milhouse_fields(db):
    """
    Configuration fields for Milhouse service.
    """
    # network fields --------------------------------------
    db.add_field("/recv/network/jitterbuffer",
        default=30,
        desc="Length of receiver’s rtp jitterbuffers in milliseconds, must be > 1")
    # audio fields --------------------------------------
    db.add_field("/send/audio/source", 
        values=["dv1394src", "alsasrc", "audiotestsrc", 
            "filesrc", "jackaudiosrc", "pulsesrc"], 
        default="jackaudiosrc", 
        desc="Audio input")
    db.add_field("/send/audio/file_location", 
        default="",
        desc="Audio input file location. Only used with filesrc audio source") 
    db.add_field("/both/audio/numchannels",
        default=2,
        desc="Number of audio channels")
    db.add_field("/both/audio/codec", 
        values=["vorbis", "raw", "mp3"],
        default="raw",
        desc="Audio codec")
    db.add_field("/both/audio/port",
        default=10000,
        desc="RTP port for audio")
    db.add_field("/recv/audio/buffer_usec",
        default=11333,
        desc="length of receiver’s audio buffer in microseconds, must be > 10000")
    db.add_field("/recv/audio/sink",
        values=["alsasink", "pulsesink", "jackaudiosink"],
        desc="Audio receiver sink", default="jackaudiosink")
    # video fields --------------------------------------
    db.add_field("/send/video/bitrate",
        default=3000000,
        desc="Video bitrate")
    #db.add_field("/send/audio/bitrate", # TODO: quality
    #    default=1000000,
    #    desc="Audio bitrate")
    db.add_field("/both/video/codec",
        values=["h264", "theora", "mpeg4", "h263"],
        default="mpeg4",
        desc="Video codec")
    db.add_field("/send/video/source",
        values=["dv1394src", "filesrc", "videotestsrc", "v4l2src", "dc1394src"],
        default="v4l2src",
        desc="Video source")
    db.add_field("/recv/video/sink",
        values=["glimagesink", "xvimagesink", "ximagesink"], default="xvimagesink",
        desc="Receiver's video sink")
    db.add_field("/both/video/port",
        default=11000,
        desc="RTP port for video")
    db.add_field("/send/video/file_location",
        default="",
        desc="Sender's videofilesrc Video file source.")
    db.add_field("/recv/video/deinterlace",
        default=False,
        desc="Deinterlace video")
    db.add_field("/recv/video/fullscreen",
        default=False,
        desc="Display full screen on the receiver's display")
    db.add_field("/send/video/v4l2/device",
        values=["/dev/video0", "/dev/video1"],
        default="/dev/video0",
        desc="Sender's V4L2 video capture device")
    #db.add_field("/send/video/v4l2/input", 
    #    default=0,
    #    desc="Sender's V4L2 video capture input number")
    db.add_field("/send/video/v4l2/norm", 
        default="ntsc",
        values=["ntsc", "pal"],
        desc="Sender's V4L2 video standard/norm")
    # db.add_field("/send/video/jackd/device",
    #     default="",
    #     desc="Sender's audio capture device if using jackd audio source.")
    db.add_field("/recv/video/display",
        default=":0.0",
        values=[ # TODO: add comments on each entry in conf.py
            ":0.0", # Primary screen
            ":0.1", # Secondary screen
            ":10.0"], # SSH X11 forwarding
        desc="Receiver's X11 video display number")
    # TODO: remove these?
    db.add_field("/recv/network/remote_address",
        default="127.0.0.1",
        desc="The address of the sender remote peer")
    db.add_field("/send/network/remote_address",
        default="127.0.0.1",
        desc="The address of the receiver remote peer")

def setup_milhouse_settings(db):
    """
    Create lots of settings and profiles for Milhouse.
    Real world uses are here !
    Called at application startup. (only once!)
    
    * video: high-bandwidth (MPEG-4), medium-bw (MPEG-4), low-bw (H263)
    * audio: 2 channels (MP3, raw, vorbis), 8 channels (raw, vorbis) 
    """    
    global TEST_PROFILE
    #TODO: rename this
    #TODO: change settings for Group/Preset/Setting/Storage
    # TODO: bitrate for MP3
    # FIXME: put these in text file(s)
    # --------------------
    # we will have in version 0.3 :
    # profiles : 
    # --------------------------- audio codecs -----------------------------
    setting_raw_2 = db.add_setting(
        name="2 Channel Raw Audio", 
        desc="2 channels of raw audio.",
        entries={
            "/both/audio/numchannels":2,
            "/both/audio/codec":"raw",
        })
    setting_mp3_2 = db.add_setting(
        name="2 Channel MP3 Audio", 
        desc="2 channels of MP3 audio.",
        entries={
            "/both/audio/numchannels":2,
            "/both/audio/codec":"mp3",
        })
    setting_vorbis_2 = db.add_setting(
        name="2 Channel Vorbis Audio", 
        desc="2 channels of OGG Vorbis audio.",
        entries={
            "/both/audio/numchannels":2,
            "/both/audio/codec":"vorbis",
        })
    setting_raw_8 = db.add_setting(
        name="8 Channel Raw Audio", 
        desc="8 channels of raw audio.",
        entries={
            "/both/audio/numchannels":8,
            "/both/audio/codec":"raw",
        })
    setting_vorbis_8 = db.add_setting(
        name="8 Channel Vorbis Audio", 
        desc="8 channels of OGG Vorbis audio.",
        entries={
            "/both/audio/numchannels":8,
            "/both/audio/codec":"vorbis",
        })
    db.add_set("audio_codec", desc="Audio codec", settings_ids=[
        setting_raw_2.id,
        setting_mp3_2.id,
        setting_vorbis_2.id,
        setting_raw_8.id,
        setting_vorbis_8.id,
        ])
    # --------------------------- video codecs -----------------------------
    # * video: high-bandwidth (MPEG-4), medium-bw (MPEG-4), low-bw (H263)
    setting_video_high = db.add_setting(
        name="H.264 High-Bandwidth Video", 
        desc="High quality H264 at 17 Mbps",
        entries={
            "/both/video/codec":"h264",
            "/send/video/bitrate":17000000, # 17 Mbps
        })
    setting_video_medium = db.add_setting(
        name="MPEG-4 Medium-Bandwidth Video", 
        desc="Medium quality MPEG-4 at 3 Mbps",
        entries={
        "/both/video/codec":"mpeg4",
        "/send/video/bitrate":3000000, # 3 Mbps
        })
    setting_video_low = db.add_setting(
        name="H263 Low-Bandwidth Video", 
        desc="Low quality H263 at 500 kbps",
        entries={
            "/both/video/codec":"h263",
            "/send/video/bitrate":500000, # 500 kbps
        })
    db.add_set("video_codec", desc="Video codec", settings_ids=[
        setting_video_high.id,
        setting_video_medium.id,
        setting_video_low.id,
        ])
    # ------------------------------- video inputs ---------------------------
    setting_v4l2_0 = db.add_setting(
        name="V4L2 video device 0", 
        desc="V4L2 video driver with default device. Input 0. NTSC.",
        entries={
            "/send/video/source":"v4l2src",
            "/send/video/v4l2/device":"/dev/video0",
            #"/send/video/v4l2/input":0,
            #"/send/video/v4l2/norm":"ntsc",
        })
    setting_v4l2_1 = db.add_setting(
        name="V4L2 video device 1", 
        desc="V4L2 video driver with default device. Input 1. NTSC.",
        entries={
            "/send/video/source":"v4l2src",
            "/send/video/v4l2/device":"/dev/video1",
            #"/send/video/v4l2/input":1,
            #"/send/video/v4l2/norm":"ntsc",
        })
    setting_videotestsrc = db.add_setting(
        name="Video test inputs", 
        desc="Test sources for video.",
        entries = {
            "/send/video/source":"videotestsrc",
        })
    db.add_set("video_source", desc="Video input", settings_ids=[
        setting_v4l2_1.id,
        setting_v4l2_0.id,
        setting_videotestsrc.id,
        ])
    # ------------------------------- audio inputs ---------------------------
    # FIXME: separate audiotestsrc and videotestsrc
    setting_audiotestsrc = db.add_setting(
        name="Audio test inputs", 
        desc="Test sources for audio.",
        entries = {
            "/send/audio/source":"audiotestsrc",
        })
    setting_jack_in = db.add_setting(
        name="Jack audio input", 
        desc="Jack audio input.", 
        entries={
            "/send/audio/source": "jackaudiosrc",
        })
    db.add_set("audio_source", desc="Audio input", settings_ids=[
        setting_jack_in.id,
        setting_audiotestsrc.id,
        ])
    # ------------------------------- audio outputs ---------------------------
    setting_jack_out = db.add_setting(
        name="Jack audio output", 
        desc="Jack audio output.", 
        entries={
            "/recv/audio/sink": "jackaudiosink",
        })
    db.add_set("audio_sink", desc="Audio output", settings_ids=[
        setting_jack_out.id,
        ])
    # ------------------------------- video outputs ---------------------------
    setting_x11 = db.add_setting(
        name="XVideo video output", 
        desc="XVideo X11 video output.",
        entries={
            "/recv/video/sink": "xvimagesink",
            "/recv/video/display":":0.0",
        })
    setting_gl = db.add_setting(
        name="OpenGL video output", 
        desc="OpenGL Window video output.",
        entries={
            "/recv/video/sink":"glimagesink",
        })
    db.add_set("video_sink", desc="Video output", settings_ids=[
        setting_x11.id,
        setting_gl.id,
        ])
    # ----------------------------------- some profiles
    profile_test = db.add_profile(
        name="Test Sources Low-Quality 2-Channel raw", 
        desc="Test sources with jack and X11 sinks", 
        settings=[
            setting_video_low.id, 
            setting_x11.id,
            setting_videotestsrc.id, 
            setting_audiotestsrc.id, 
            setting_raw_2.id, 
            setting_jack_out.id, 
        ])
    TEST_PROFILE = profile_test.id
    profile_high = db.add_profile(
        name="High-Bandwidth 8-Channel raw",  
        desc="High-bandwith 8-channel with V4L2 and JACK", 
        settings=[
            setting_video_high.id, 
            setting_x11.id,
            setting_v4l2_0.id, 
            setting_raw_8.id, 
            setting_jack_out.id, 
            setting_jack_in.id, 
        ])
    profile_medium_8 = db.add_profile(
        name="Medium-Bandwidth 8-Channel Vorbis",  
        desc="Medium-bandwith 8-channel with V4L2 and JACK", 
        settings=[
            setting_video_medium.id, 
            setting_x11.id,
            setting_v4l2_0.id, 
            setting_vorbis_8.id, 
            setting_jack_out.id, 
            setting_jack_in.id, 
        ])
    profile_medium_2_raw = db.add_profile(
        name="Medium-Bandwidth 2-Channel raw",  
        desc="Medium-bandwith 2-channel with V4L2 and JACK", 
        settings=[
            setting_video_medium.id, 
            setting_x11.id,
            setting_v4l2_0.id, 
            setting_raw_2.id, 
            setting_jack_out.id, 
            setting_jack_in.id, 
        ])
    profile_low_2_vorbis = db.add_profile(
        name="Medium-Bandwidth 2-Channel vorbis",  
        desc="Medium-bandwith 2-channel with V4L2 and JACK", 
        settings=[
            setting_video_medium.id, 
            setting_x11.id,
            setting_v4l2_0.id, 
            setting_vorbis_2.id, 
            setting_jack_out.id, 
            setting_jack_in.id, 
        ])
    profile_low_2 = db.add_profile(
        name="Low-Bandwidth 2-Channel Vorbis",  
        desc="Medium-bandwith 2-channel with V4L2 and JACK", 
        settings=[
            setting_video_low.id, 
            setting_x11.id,
            setting_v4l2_0.id, 
            setting_vorbis_2.id, 
            setting_jack_out.id, 
            setting_jack_in.id, 
        ])
    #for set_name, s in db.sets.iteritems():
    #    print("Set %s is choice between either:" % (set_name))
    #    for setting_id in s.get_all():
    #        print("    - %s" % (db.get_setting(setting_id).name))

