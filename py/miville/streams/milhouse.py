#!/usr/bin/env python 
# -*- coding: utf-8 -*-
"""
Milhouse engine and stream.
"""
import os
import copy
import warnings

from twisted.internet import defer
from twisted.internet import error
from twisted.internet import protocol
from twisted.internet import reactor
from twisted.python import failure
from twisted.python import procutils
import zope.interface

from miville.streams import states
from miville.streams import conf
from miville.streams import session
from miville.streams import tools
#from miville.streams import StreamError
from miville.errors import * # StreamError
#from miville.utils import common # For ports allocator TODO: rename this and improve it.

from miville.utils import log
from miville.utils import sig

log = log.start("debug", 1, 0, "streams.milhouse") 
#log = log.start("error", 1, 0, "milhouse") 
#debug for when developing is useful

VERBOSE = False
VERY_VERBOSE = False
VERBOSE_ERROR = True
TEST_PROFILE = 0 # defined below


#------------------------------------ MILHOUSE stream --------------
#TODO

class MilhouseProcessManager(tools.ProcessManager):
    """
    We just need to override one method.
    """
    def format_output_when_crashed(self, output):
        """
        The process has crashed. Let's just keep the interesting lines to print to the uesr.
        """
        log.debug("MilhouseProcessManager.format_output_when_crashed")
        ret = ""
        for line in output:
            for txt in ("CRITICAL", "ERROR", "WARNING", "error while loading shared libraries"):
                if line.find(txt) != -1:
                    log.debug("Log : Accepting line : %s" % (line))
                    ret += line + "\n"
                else:
                    log.debug("Log : DISCARD line : %s" % (line))
        return ret

class MilhouseFactory(object):
    """
    Provides Milhouse streams.
    See miville.streams.interfaces.IService for the documentation.
    """
    # zope.interface.implements(interfaces.IService)
    name = "Milhouse" # Name of the service
    enabled = True
    
    def __init__(self):
        self.streams = {} # Dict of current streams. 
        #The ID of the contact_info should be their keys.
        self.config_db = None # Configuration client.
        self.config_fields = {} 
        self.ports_allocator = tools.PortsAllocator(minimum=10000 + conf.PORT_OFFSET, increment=10) 
        # 10 is the minimum permitted interval between milhouse.cpp ports.

    def config_init(self, config_db):
        #TODO: store config fields
        self.config_db = config_db
        setup_milhouse_fields(config_db)
        setup_milhouse_settings(config_db)
    # def _cb_started(self, result):
    #     """
    #     Deferred list callback.
    #     """
    #     return result
    
    def prepare_session(self, session_desc):
        """
        Prepares the config entries for the alice and the bob.
        
        This is called from the streams manager.
        :param session: SessionDescription
        """
        role = session_desc.role
        log.debug("MilhouseFactory.prepare_session(role=%s)" % (role))
        alice_entries = session_desc.streams_to_offerer[0].entries # FIXME
        bob_entries = session_desc.streams_to_answerer[0].entries # FIXME
        contact_infos = session_desc.contact_infos
        
        if role == session.ROLE_OFFERER:
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
        return "%s(%s)" % (contact_infos.get_id(), mode)

    def start(self, session_desc):
        """
        Starts streamers of the milhouse service.
        
        Here, we start both local sender and local receiver.
        (starting 2 streams)
        
        Return a deferred
        """
        # TODO: check if already in a session with remote
        role = session_desc.role
        contact_infos = session_desc.contact_infos
        alice_entries = session_desc.streams_to_offerer[0].entries # FIXME
        bob_entries = session_desc.streams_to_answerer[0].entries # FIXME
        if role == session.ROLE_ANSWERER: # bob
            pass
        # first, let's make sure there are no milhouse streamers
        for mode in ["send", "recv"]:
            verif_key = self._make_key(contact_infos, mode)
            contact_name = contact_infos.get_contact_name()
            if self.streams.has_key(verif_key):
                stream = self.streams[verif_key]
                if stream.state in [states.STATE_STREAMING, states.STATE_STARTING]:
                    log.error("Found milhouse stream %s %s in state %s" % (verif_key, stream, stream.state))
                    return defer.fail(failure.Failure(states.StreamError("""Could not start Milhouse Service. There is already a Milhouse stream for the contact "%s".""" % (contact_name))))
                else:
                    #XXX
                    self._clear_streams_with_contact(session_desc)
        # our callback in errback for the deffered list: 
        def _eb(reason, self, session_desc):
            contact_infos = session_desc.contact_infos
            log.error("Could not start streams with contact. %s %s" % (contact_infos.get_id(), reason.getErrorMessage()))
            # set their state as error so that we can clear them :
            for mode in ["send", "recv"]:
                key = self._make_key(contact_infos, mode)
                stream = self.streams[key]
                if stream.state != states.STATE_FAILED:
                    log.debug("Changing stream %s state from %s to %s" % (key, stream.state, states.STATE_FAILED))
                    stream.state = states.STATE_FAILED
                    # TODO: change stream description state too...
                    # TODO: send error message to remote agent
            self._clear_streams_with_contact(session_desc)
            #TODO: deallocate ports.
            #log.error("Failure in MilhouseFactory.start()'s callback. 
            return reason
        def _cb(result, self, session_desc):
            contact_infos = session_desc.contact_infos
            log.debug("successfully started both streams with %s" % (contact_infos.get_id()))
            return result
        dl = []
        log.debug("Offerer ports: %s. Answerer ports: %s" % (
            conf.path_glob(alice_entries, "port", conf.GLOB_CONTAINS),
            conf.path_glob(bob_entries, "port", conf.GLOB_CONTAINS)))
        # checking if bob's ports are valid
        answerer_ports = conf.path_glob(bob_entries, "port", conf.GLOB_CONTAINS)
        for port in answerer_ports.itervalues():
            if port == 0:
                return defer.fail(failure.Failure(states.StreamError("Answerer has not set his port numbers.")))
        # creating and starting the streamers
        for mode in ["send", "recv"]:
            key = self._make_key(contact_infos, mode)
            self.streams[key] = MilhouseStreamer(self, key)
            if role == session.ROLE_OFFERER:
                if mode == "send":
                    config_entries = alice_entries # no copy.deepcopy
                else: # receive
                    config_entries = bob_entries
            else:
                if mode == "send":
                    config_entries = bob_entries
                else: # receive
                    config_entries = alice_entries
            log.debug("STARTING. Mode: %s. Config entries ports: %s" % (mode, conf.path_glob(config_entries, "port", conf.GLOB_CONTAINS)))
            # connect to streamer's signal
            self.streams[key].signal.connect(self._on_streamer_signal)
            try:
                d = self.streams[key].start(session_desc, config_entries, mode)
            except tools.ManagedProcessError, e:
                return defer.fail(failure.Failure(states.StreamError("Milhouse Process error: %s" % (e.message))))
            #contact_infos, config_entries, mode, role)
            dl.append(d)
        deferred = tools.deferred_list_wrapper(dl)  # contains each 
        deferred.addCallback(_cb, self, session_desc)
        deferred.addErrback(_eb, self, session_desc)
        return deferred
    
    def _on_streamer_signal(self, key, streamer_identifier):
        """
        :param key: str
        :param streamer_identifier: str
        """
        if key == "crashed":
            log.debug("Milhouse streamer %s crashed !" % (streamer))
        else:
            log.error("Unknow signal key %s" % (key))

    def _clear_streams_with_contact(self, session_desc): #contact_infos):
        """
        Deletes the streams for a contact.
        Must be called when error occur, of when closing a stream.
        """
        contact_infos = session_desc.contact_infos
        for mode in ["recv", "send"]:
            key = self._make_key(contact_infos, mode)
            if self.streams.has_key(key):
                stream = self.streams[key]
                state = stream.state 
                if mode == "recv":
                    audio_port = stream.config_entries["/both/audio/port"]
                    video_port = stream.config_entries["/both/video/port"]
                    for portnum in (audio_port, video_port):
                        try:
                            self.ports_allocator.free(portnum)
                        except tools.PortsAllocatorError, e:
                            log.error(e.message)
                if state in [states.STATE_IDLE, states.STATE_FAILED, states.STATE_STOPPED]:
                    # see miville.streams.states.STATE_*
                    log.debug("deleting milhouse stream %s" % (key))
                    del self.streams[key]
                else: # STREAMING
                    # TODO: d = self.streams[key].stop(); d.addCallback.... 
                    msg = "Cannot delete milhouse stream %s. Its state is %s." % (key, state)
                    log.error("_clear_streams_with_contact: " + msg)
                    raise StreamError(msg) # !!!

    def list_streams_for_contact(self, session_desc):
        """
        Returns a list of stream instances for a contact.
        :param contact_infos: ContactInfos instance.
        """
        contact_infos = session_desc.contact_infos
        ret = []
        for mode in ["send", "recv"]:
            key = self._make_key(contact_infos, mode)
            if self.streams.has_key(key):
                ret.append(self.streams[key])
        return ret
        
    def stop(self, session_desc):
        """
        Return a deferred
        """
        contact_infos = session_desc.contact_infos
        # at this stage, a message was already sent to remote host.
        contact_id = contact_infos.get_id()
        contact_name = contact_infos.get_contact_name()
        
        send_key = self._make_key(contact_infos, "send")
        recv_key = self._make_key(contact_infos, "recv")
        #if not self.streams.has_key(send_key): 
        #    return defer.fail(failure.Failure(states.StreamError("""There is no recv Milhouse stream for the contact "%s".""" % (contact_name))))
        #elif not self.streams.has_key(recv_key): 
        #    return defer.fail(failure.Failure(states.StreamError("""There is no send Milhouse stream for the contact "%s".""" % (contact_name))))
        if not self.streams.has_key(send_key) and not self.streams.has_key(recv_key): 
            return defer.fail(failure.Failure(states.StreamError("""There is no Milhouse stream for the contact "%s".""" % (contact_name))))
        else:
            # our callback and errback
            def _eb(reason, self, session_desc):
                contact_infos = session_desc.contact_infos
                log.error("Error in MilhouseFactory.stop()'s errback. TODO: stop and erase both streams since an error occured. Error msg: %s" % (reason.getErrorMessage()))
                self._clear_streams_with_contact(session_desc)
                return reason
            def _cb(result, self, session_desc):
                contact_infos = session_desc.contact_infos
                #TODO: deallocate ports.
                log.debug("successfully stopped both streams with %s" % (contact_infos.get_id()))
                self._clear_streams_with_contact(session_desc)
                return result
            dl = []
            for mode in ["send", "recv"]:
                key = self._make_key(contact_infos, mode)
                try:
                    stream = self.streams[key]
                except KeyError, e:
                    pass
                    #deferred = defer.fail(failure.Failure(states.StreamError("""There is no %s Milhouse stream for the contact "%s".""" % (mode, contact_name))))
                else:
                    deferred = stream.stop() #TODO: what if it fails immediately?
                    #deferred.addCallback(_callback, self, contact_id, mode)
                    dl.append(deferred) 
            # our callback in errback for the deffered list: 
            deferred = tools.deferred_list_wrapper(dl) #defer.DeferredList(dl, consumeErrors=False)
            deferred.addCallback(_cb, self, session_desc)
            deferred.addErrback(_eb, self, session_desc)
            return deferred
    
    def stop_all(self):
        raise NotImplementedError("To do.")
        #def _success(result, self, wrapper, contact_id):
        #    
        #wrapper = tools.DeferredWrapper()
        #deferred = wrapper.make_deferred()
        #dl = []
        #for contact_id in self.streams.keys():
        #    dl.append(self.stop(contact_id))
        #deferred_list = defer.DeferredList(dl, consumeErrors=True)
        #deferred_list.addCallback(_success, self, )


class MilhouseStreamer(object):
    """
    Milhouse stream
    See miville.streams.interfaces.IStream
    """
    #zope.interface.implements(interfaces.IStream)
    name = "Milhouse"
    def __init__(self, service, identifier):
        """
        :param identifier: unique ID.
        :param service: MilhouseFactory instance.
        """
        self.service = service
        self.identifier = identifier
        self.state = states.STATE_IDLE # see miville.streams.states
        self.config_entries = None
        self.mode = None
        self.ports = [] # TODO
        self._contact_infos = None # TODO: remove the "_"
        self._process_manager = None # TODO: remove the "_"
        self.signal = sig.Signal() # 1st arguments is key: "crashed", 2nd is self.identifier
        
    def _on_process_event(self, key, value=None):
        log.debug("Milhouse process event: %s %s" % (key, value))
        if key == "crashed":
            log.error("Milhouse crashed.")
            self.state = states.STATE_FAILED 
            self.signal("crashed", self.identifier)
            #self.service.stop(self._contact_infos) # sends a message 
            txt = self._process_manager.stdout_logger.get_text()
            txt += self._process_manager.stderr_logger.get_text()
            log.debug(txt) # XXX very verbose !
        elif key =="stop_success":
            # TODO: trigger delayed right now instead of waiting.
            log.debug("%s:%s" % (key, value))
        # TODO: check milhouse success by looking at it stdout.
        # TODO: trigger delayed right now instead of waiting.
    
    def start(self, session, config_entries, mode):
        # set up some attributes
        self._contact_infos = session.contact_infos
        self.session = session
        self.config_entries = config_entries
        self.state = states.STATE_STARTING
        self.mode = mode
        # and now start it
        
        args = ["milhouse"] # TODO: add args
        process_codename = "%s(%s)" % (self.name, self.mode)
        all_args = config_fields_to_milhouse_opts(config_entries)
        if self.mode == "send":
            the_key = "sender"
            env_key = "sender_env"
        else:
            the_key = "receiver"
            env_key = "receiver_env"
        log.info("%s$ milhouse %s" % (process_codename, all_args[the_key]))
        args = all_args[the_key].split()
        try:
            contact_name = self._contact_infos.contact.name
        except AttributeError, e:
            pass
        else:
            args.append("-W") # hard-coded !!!
            window_title = str(contact_name)
            for forbidden in "()\"'*!@#$%^&-=+\\|/~`":
                window_title = window_title.replace(forbidden, "")
            args.append(window_title) # hard-coded !!!
        environment_variables = all_args[env_key]
        log.debug("Environment variables : %s" % (environment_variables))
        self._process_manager = MilhouseProcessManager(name=process_codename, command=args, check_delay=2.5, env=environment_variables) # very long delay
        self._process_manager.signal.connect(self._on_process_event)
        deferred = self._process_manager.start()
        return deferred
    
    def stop(self):
        def _cb(result, self):
            self.state = states.STATE_STOPPED
            return result
        process_codename = "%s(%s)" % (self.name, self.mode)
        log.info("%s stop()" % (process_codename))
        deferred = self._process_manager.stop()
        deferred.addCallback(_cb, self)
        return deferred

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
        desc="UDP/IP port for audio")
    db.add_field("/recv/audio/buffer_usec",
        default=11333,
        desc="length of receiver’s audio buffer in microseconds, must be > 10000")
    db.add_field("/recv/audio/sink",
        values=["alsasink", "pulsesink", "jackaudiosink"],
        desc="Audio receiver sink", default="jackaudiosink")
    # video fields --------------------------------------
    db.add_field("/send/video/bitrate",
        default=1000000,
        desc="Video bitrate")
    #db.add_field("/send/audio/bitrate", # TODO: quality
    #    default=1000000,
    #    desc="Audio bitrate")
    db.add_field("/both/video/codec",
        values=["h264", "theora", "mpeg4", "h263"],
        default="h264",
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
        desc="UDP/IP port for video")
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
    db.add_field("/send/video/v4l2/input", 
        default=0,
        desc="Sender's V4L2 video capture input number")
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
            "/both/audio/codec":"raw",
        })
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
    # ------------------------------- inputs ---------------------------
    setting_v4l2_0 = db.add_setting(
        name="V4L2 video input 0 NTSC", 
        desc="V4L2 video driver with default device. Input 0. NTSC.",
        entries={
            "/send/video/source":"v4l2src",
            "/send/video/v4l2/device":"/dev/video0",
            "/send/video/v4l2/input":0,
            "/send/video/v4l2/norm":"ntsc",
        })
    setting_v4l2_1 = db.add_setting(
        name="V4L2 video input 1 NTSC", 
        desc="V4L2 video driver with default device. Input 1. NTSC.",
        entries={
            "/send/video/source":"v4l2src",
            "/send/video/v4l2/device":"/dev/video0",
            "/send/video/v4l2/input":1,
            "/send/video/v4l2/norm":"ntsc",
        })
    setting_testsrc = db.add_setting(
        name="Audio/video test sources", 
        desc="Test sources for both audio and video.",
        entries = {
            "/send/audio/source":"audiotestsrc",
            "/send/video/source":"videotestsrc",
        })
    setting_jack_in = db.add_setting(
        name="Jack audio input", 
        desc="Jack audio input.", 
        entries={
            "/send/audio/source": "jackaudiosrc",
        })
    # ------------------------------- outputs ---------------------------
    setting_jack_out = db.add_setting(
        name="Jack audio output", 
        desc="Jack audio output.", 
        entries={
            "/recv/audio/sink": "jackaudiosink",
        })
    setting_x11 = db.add_setting(
        name="X11 video output", 
        desc="X11 Window video output.",
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
    # ----------------------------------- some profiles
    profile_test = db.add_profile(
        name="Test Sources Low-Quality 2-Channel raw", 
        desc="Test sources with jack and X11 sinks", 
        settings=[
            setting_video_low.id, 
            setting_x11.id,
            setting_testsrc.id, 
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
