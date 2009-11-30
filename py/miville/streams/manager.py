#!/usr/bin/env python 
# -*- coding: utf-8 -*-
"""
Managers for streams.

For the notifications to the API, we do not care if it is this miville (Alice) 
or the remote one (Bob) that started the stream, we just want to know 
we are streaming and with which configuration profile name and entries. 
Actually, for now, there are 2 different callbacks for when we are 
Alice or Bob.

Here are the things to verify to make sure a streams has started :
 * the local sender/receiver process is running
 * le remote receiver/sender process is running 
 * the local sender/receiver is streaming
 * the remote sender/receiver is streaming

Of course, we use the comchan to communicate with the remote peer. We use it
through the miville.streams.communication.

Also :

Session description classes.
Those are typically turned into dicts and sent to the remote agent for 
negotiation.
"""
from miville.utils import utc
import copy

from miville.streams import communication
from miville.errors import * # StreamError
from miville.streams import conf
from miville.streams import milhouse
from miville.streams.constants import * # STATE_*, ROLE_*, DIRECTION_*, etc.
from miville.utils import log
from miville import connectors

log = log.start("debug", True, False, "streams.manager") 

_single_manager = None # singleton

class SessionError(Exception):
    """
    Any error the SimpleSessionInitiationProtocol can raise.
    """
    pass

class Session(object):
    """
    We need to wrap information about the current session in an object.
    This will be much easier to manager.
    Contains stuff that is negotiated between the peers.
    """
    # Not used yet.
    def __init__(self, manager=None, session_id=None, time_started=None, contact_infos=None, role=ROLE_OFFERER):
        """
        :param contact_infos: ContactInfos object
        :param role: str Either "offerer" or "answerer"
        """
        self.manager = manager
        self.contact_infos = contact_infos
        self.streams_to_offerer = [] 
        self.streams_to_answerer = []
        self.role = role # the role if this agent. Is this miville offerer or answerer ?
        self.time_started = None # UTC timestamp
        self.stream_state = STATE_IDLE
        self.comment = ""
        self.session_id = None
        self.notified_api_of_start = False

        # init stuff
        if time_started is None:
            self.time_started = utc.get_current_utc_time()
        else:
            self.time_started = time_started
        if session_id is not None:
            self.session_id = session_id
        else:
            self.session_id = int(self.time_started)

    def on_rtcp_sender_started(self, value):
        """ Called when rtcp messages indicate that the Milhouse sending process has started """
        #log.debug('RTCP SENDER STARTED: %s' % (value))
        if not self.notified_api_of_start:
            self._notify_api_about_start()
            self.notified_api_of_start = True 

    def _notify_api_about_start(self):
        self.stream_state = STATE_STREAMING
        notif = {
            "contact":self.contact_infos.contact,
            "contact_name":self.contact_infos.contact.name,
            "message":'',
            "success":True,
            }
        self.manager.remote_started_streams(notif)
    
    def on_milhouse_exitted(self):
        """ Should be called when a Milhouse process has exitted """
        #log.debug('RTCP SENDER STARTED: %s' % (value))
        if self.stream_state != STATE_ERROR:
            log.info('MILHOUSE EXITTED')
            self._notify_api_about_stop()
    
    def on_could_not_start(self, message):
        """ Should be called when could not start milhouse"""
        if self.stream_state != STATE_ERROR:
            log.info('Error when starting service')
            self.stream_state = STATE_STOPPED
            notif = {
                "contact":self.contact_infos.contact,
                "contact_name":self.contact_infos.contact.name,
                "message":message,
                "success":False,
                }
            # FIXME: call something else...
            self.manager.remote_started_streams(notif)
    
    def on_milhouse_exitted_itself(self):
        """ Should be called when a Milhouse process has exitted itself, i.e. the window was closed. This
            has a weird side effect that if you're running both sessions on the same machine, it won't
            scrap the first one. """
        #log.debug('RTCP SENDER STARTED: %s' % (value))
        log.info('MILHOUSE EXITTED ITSELF')
        self._notify_api_about_stop()
        self.manager.stop(self.contact_infos.contact)

    def _notify_api_about_stop(self):
        """ Notify api of stop """
        self.stream_state = STATE_STOPPED
        notif = {
            "contact":self.contact_infos.contact,
            "contact_name":self.contact_infos.contact.name,
            "message":'Stopped',
            "success":True,
            }
        self.manager.remote_stopped_streams(notif)

    def on_problem(self, problem):
        """ Should be called when a Milhouse process has a problem """
        #log.debug('RTCP SENDER STARTED: %s' % (value))
        log.info('MILHOUSE PROBLEM')
        self.stream_state = STATE_ERROR
        notif = {
            "contact":self.contact_infos.contact,
            "contact_name":self.contact_infos.contact.name,
            "message":str(problem),
            "success":True,
            }
        self.manager.remote_stopped_streams(notif)
        self.manager.stop(self.contact_infos.contact)

    def on_rtcp_sender_connected(self, value):
        """ Called when rtcp messages indicate that the Milhouse sending process is currently sending
            packets to a Milhouse receiving process """
        #log.debug('RTCP SENDER CONNECTED: %s' % (value))
        #TODO: do something with this info
        
    def on_rtcp_receiver_connected(self, value):
        """ Called when rtcp messages indicate that the Milhouse sending process has started """
        #log.info('RTCP RECEIVER CONNECTED: %s' % (value))
        if self.stream_state != STATE_STREAMING:
            log.info('changed stream_state to STATE_STREAMING')
            self.stream_state = STATE_STREAMING
        #TODO: do something with this info
    
    def add_stream(self, stream):
        """
        Adds a stream to the session.
        """
        #log.debug("add_stream %s %s %s" % (stream.direction, service.name, stream.entries))
        if stream.direction == DIRECTION_TO_OFFERER:
            self.streams_to_offerer.append(stream)
        else:
            self.streams_to_answerer.append(stream)

    def get_stream(self, service=None, direction=DIRECTION_TO_OFFERER, number=0):
        """
        Returns a Stream instance.
        """
        if direction == DIRECTION_TO_OFFERER:
            which = self.streams_to_offerer
        else:
            which = self.streams_to_answerer
        ret = None
        current_index = 0 # FIXME: add a level... list of streams with indices.
        for stream in which: # which is a list
            if stream.service is service:
                if number == current_index:
                    ret = stream
                else:
                    current_index += 1
        if ret is None:
            raise SessionError("No stream %s with index %d" % (service.name, number))
        return ret
    
    def get_senders(self):
        mapping = {
            ROLE_OFFERER: self.streams_to_answerer, 
            ROLE_ANSWERER: self.streams_to_offerer
            }
        return mapping[self.role]
    
    def get_receivers(self):
        mapping = {
            ROLE_OFFERER: self.streams_to_offerer, 
            ROLE_ANSWERER: self.streams_to_answerer
            }
        return mapping[self.role]

class StreamsManager(object):
    """
    This is the only class that should be used from the core api.
    
    Wasn't it called StreamsManager ? 
    """
    def __init__(self, api=None):
        self.api = api
        self.config_db = conf.get_single_db()
        self.services = {} # key are class names
        self.sessions = {} # keys are created using ContactInfos.get_id()
        self.DEFAULT_PROFILE_ID = 0 # milhouse.TEST_PROFILE # FIXME
    
    def add_service(self, service):
        """
        Must be called only once at startup for each service class.

        :param service: A Service instance.
        """
        if service.enabled:
            if self.services.has_key(service.name):
                raise StreamError("There is already a service with the name \"%s\"." % (service.name))
            else:
                self.services[service.name] = service
                service.config_init(self.config_db)
    
    def _create_contact_infos_from_contact(self, contact):
        """
        Returns a ContactInfos
        
        We wrap contact infos in a class, just to be independant from the 
        miville.addressbook.Contact class.
        
        :param contact: miville.addressbook.Contact
        We must be connected to that contact in order to get our local port and addr.
        Uses Contact, the streams communication channel and the Api.
        """
        OPTION_MIVILLE_PORT = "connector_port"
        try:
            comm = communication.get_channel_for_contact(contact) 
        except StreamError:
            raise
        comm.remote_addr
        comm.my_addr
        contact_infos = conf.ContactInfos(
            remote_addr = comm.remote_addr, #"127.0.0.1",
            local_addr = comm.my_addr, #"127.0.0.1",
            remote_port = contact.port, # 2222, # com_chan port
            local_port = self.api.get_config(OPTION_MIVILLE_PORT) , #2222, # com_chan port
            contact = contact # miville.addressbook.Contact object.
        )
        return contact_infos

    def _create_session(self, contact_infos, role=None, session_id=None, time_started=None):
        """
        Returns a Session instance.
        
        It also add the created session to the sessions dict.
        If role is "bob", you must provide a request instance.
        Raises a StreamError if session already exists.
        """
        kwargs = {}
        if role == ROLE_OFFERER:
            pass #role = ROLE_OFFERER
        else: # bob
            role = ROLE_ANSWERER
            kwargs["session_id"] = session_id # TODO: remove this
            #TODO: kwargs["time_started"] = time_started
            kwargs["time_started"] = utc.get_current_utc_time() #TODO: remove this
        session_key = contact_infos.get_id()
        if self.sessions.has_key(session_key):
            #return self.sessions[session_key]
            raise StreamError("Streams manager has already a session for contact %s" % (session_key))
        log.debug("will create session desc. role %s" % (role))
        session = Session(manager=self, contact_infos=contact_infos, role=role, **kwargs)
        self.sessions[session_key] = session
        log.info("Creating streaming session with %s" % (session_key))
        return session
    
    def _get_session(self, contact_infos):
        """
        Returns session object for contact.
        Raises StreamError if no session for contact.
        """
        session_key = contact_infos.get_id()
        if not self.sessions.has_key(session_key):
            raise StreamError("Streams manager does not have a session for contact %s" % (session_key))
        return self.sessions[session_key]
        
    def _delete_session(self, contact_infos):
        """
        Delete a session if in state stopped, idle or error.
        Otherwise, raise a StreamError
        Raises StreamError if no session for contact.
        """
        session = self._get_session(contact_infos)
        session_key = contact_infos.get_id()
        log.debug("deleting session for contact %s" % (contact_infos.get_id()))
        if session.stream_state in [STATE_IDLE, STATE_STOPPED, STATE_ERROR]:
            del self.sessions[session_key]
        else:
            raise StreamError("session not deletable : in state %s" % (session.stream_state))
    
    def _get_config_entries_for_contact(self, contact):
        """
        :param contact: miville.addressbook.Contact to start streaming with.
        Returns a dict of config entries.
        """
        self.DEFAULT_PROFILE_ID = milhouse.TEST_PROFILE # FIXME
        profile_id = None
        entries = {}
        try:
            profile_id = int(contact.profile_id)
        except TypeError:
            msg = "Contact %s does not have a profile associated with it." % (contact.name)
            profile_id = -1
        except AttributeError:
            msg = "Contact does not have a profile_id attribute !"
            log.error(msg)
            profile_id = -1
        if contact.profile_id == 0 or profile_id == -1:
            contact.profile_id = self.DEFAULT_PROFILE_ID
            log.warning("setting contact %s profile ID to %d" % (contact.name, self.DEFAULT_PROFILE_ID))
        try:
            entries = self.config_db.get_entries_for_profile(contact.profile_id)
        except conf.ConfError, e:
            log.error(e.message)
            entries = self.config_db.get_entries_for_profile(self.DEFAULT_PROFILE_ID)
            log.debug("entries: %s" % (entries))
        return entries
        
    def _cb_ack(self, channel, contact):
        """Does nothing right now."""
        log.info("Got ACK from remote contact %s" % (contact.name))
        
    def _cb_start(self, channel, contact, alice_entries, bob_entries):
        """
        Got START message from remote host (we are BOB)
        
        Called from miville.streams.communication.StreamsCommunication.on_remote_message.

        :param channel: miville.streams.communication.StreamsCommunication
        :param contact: miville.addressbook.Contact object
        :param config_entries: dict of entries
        """
        log.info("r start from remote  %s %s %s" % (contact, alice_entries, bob_entries))
        contact_infos = self._create_contact_infos_from_contact(contact)
        #TODO: provide time_start and session_id from remote !!!!!!!
        try:
            session = self._create_session(session_id=None, time_started=None, contact_infos=contact_infos, role=ROLE_ANSWERER)
        except StreamError, e:
            log.error("We got start even if a session was already created. We raise an error, but it should not destroy any existing stream or negotiation process.")
            log.error("In streams.manager._cb_start() : %s" % (e.message))
            session_key = contact_infos.get_id()
            self._delete_session(contact_infos) # FIXME: should make sure streams are stopped first.
            log.warning("Deleting session with contact %s and creating a new one." % (contact.name))
            session = self._create_session(session_id=None, time_started=None, contact_infos=contact_infos, role=ROLE_ANSWERER)
        factory = self.services["milhouse"]
        session.add_stream(milhouse.MilhouseStream(direction=DIRECTION_TO_OFFERER, session=session, service=factory, entries=alice_entries)) # index 0
        session.add_stream(milhouse.MilhouseStream(direction=DIRECTION_TO_ANSWERER, session=session, service=factory, entries=bob_entries)) # index 0
        for service in self.services.itervalues():
            service.prepare_session(session)
        comm = communication.get_channel_for_contact(contact)
        comm.send_ok_start(session)
        self._start(session)
    
    def _cb_ok_start(self, channel, contact, alice_entries, bob_entries):
        """
        Alice got OK (START) message from bob
        
        Called from miville.streams.communication.StreamsCommunication.on_remote_message.
        :param channel: miville.streams.communication.StreamsCommunication
        :param contact: miville.addressbook.Contact object
        :param config_entries: dict of entries
        """
        log.info("r OK start from remote  %s %s %s" % (contact, alice_entries, bob_entries))
        contact_infos = self._create_contact_infos_from_contact(contact)
        #TODO: provide time_start and session_id from remote !!!!!!!
        try:
            session = self._get_session(contact_infos)
        except StreamError, e:
            log.critical(e.message)
        else:
            service = self.services["milhouse"]
            session.get_stream(service, DIRECTION_TO_OFFERER, 0).entries.update(alice_entries)
            session.get_stream(service, DIRECTION_TO_ANSWERER, 0).entries.update(bob_entries)
            comm = communication.get_channel_for_contact(contact)
            comm.send_ack(session)
            self._start(session)

    def _cb_stop(self, channel, contact):
        """
        Got STOP message from remote host
        
        Called from miville.streams.communication.StreamsCommunication.on_remote_message.

        :param channel: miville.streams.communication.StreamsCommunication
        :param contact: miville.addressbook.Contact object
        """
        # TODO : send OK
        log.info("Received STOP from remote  %s " % (contact))
        contact.stream_state = STATE_STOPPING # stopping !
        contact_infos = self._create_contact_infos_from_contact(contact)
        try:
            session = self._get_session(contact_infos)
        except StreamError, e:
            log.critical(e.message)
        else:
            self._stop(session)

    def _stop(self, session):
        # TODO: notify API with success/failure and message
        for service in self.services.itervalues():
            service.stop(session)
            log.debug('deleting session')
            self._delete_session(session.contact_infos)
    
    def list_streams(self):
        """
        Returns the list of all current Stream instances.
        """
        ret = []
        for session in self.sessions:
            for off in session.streams_to_offerer:
                ret.append(off)
            for ans in session.streams_to_answerer:
                ret.append(ans)
        log.debug("list_streams: %s" % (streams))
        return streams
    
    def list_streams_for_contact(self, contact):
        """
        Returns the list of streams for a contact.
        :param contact: addressbook.Contact instance.
        """
        try:
            contact_infos = self._create_contact_infos_from_contact(contact)
        except StreamError, e:
            log.error(e.message)
            return []
        try:
            session = self._get_session(contact_infos)
        except StreamError, e:
            return []
        else:
            ret = []
            for off in session.streams_to_offerer:
                ret.append(off)
            for ans in session.streams_to_answerer:
                ret.append(ans)
            return ret
        try:
            contact_name = contact_infos.contact.name
        except ValueError:
            contact_name = str(contact_infos.__dict__)
        log.debug("list_streams_for_contact: %s %s" % (contact_name, streams))
        return streams

    def start(self, contact):
        """
        Starts all the streams for the provided contact.
        
        Called from the miville core API.
        :param contact: miville.addressbook.Contact to start streaming with.
        Returns a DeferredList
        """
        #TODO: if not connected, connect
        #TODO: it is the service that should duplicate the config entries for the initiator and the responder. (A and B) and overide the port numbers accordingly. We should add it a prepare_startup method, or so.
        log.debug("manager.start")
        try:
            contact_infos = self._create_contact_infos_from_contact(contact)
        except StreamError, e: # in case we are not connected to contact.
            log.debug("We are not connected to contact? Maybe bob has errors in the comchan timing of getmyip thing.")
            raise
        config_entries = self._get_config_entries_for_contact(contact)
        comm = communication.get_channel_for_contact(contact)
        try:
            contact_name = contact_infos.contact.name
        except ValueError:
            contact_name = str(contact_infos.__dict__)
        try:
            session = self._create_session(contact_infos=contact_infos, role=ROLE_OFFERER) 
        except StreamError, e:
            log.error("In streams.manager.start() : %s" % (e.message))
            session_key = contact_infos.get_id()
            self._delete_session(contact_infos) # FIXME: should make sure streams are stopped first.
            log.warning("Deleting session with contact %s and creating a new one." % (contact.name))
            session = self._create_session(contact_infos=contact_infos, role=ROLE_OFFERER) 
        offerer_entries = copy.deepcopy(config_entries)
        answerer_entries = copy.deepcopy(config_entries)
        factory = self.services["milhouse"]
        session.add_stream(milhouse.MilhouseStream(direction=DIRECTION_TO_OFFERER, session=session, service=factory, entries=offerer_entries)) 
        session.add_stream(milhouse.MilhouseStream(direction=DIRECTION_TO_ANSWERER, session=session, service=factory, entries=answerer_entries)) 
        for service in self.services.itervalues():
            log.info("preparing service %s to stream with contact %s" % (service.name, contact_infos.get_id()))
            service.prepare_session(session)
            log.debug("alice ports: %s. bob ports: %s" % (conf.path_glob(offerer_entries, "port", conf.GLOB_CONTAINS), conf.path_glob(answerer_entries, "port", conf.GLOB_CONTAINS)))
            log.debug("config entries : %s %s" % (offerer_entries, answerer_entries))
        comm.send_start(session)
        # TODO: notify "Initiating streams with contact %s."  % (contact_name)

    def _start(self, session):
        """
        Actually start the streamers. 
        """
        try:
            for service in self.services.itervalues():
                service.start(session) # right now, there is only milhouse service.
        except StreamError, e:
            log.error("Could not start streams with reason: %s" %  (e.message))
            session.on_could_not_start(e.message) # notifies the api.
            self.stop(session.contact_infos.contact)
        # This will notify the api when rtcp messages start showing up in output
    
    def list_all_streams(self):
        """
        Returns a list of all active streams.
        """
        ret = []
        for service in self.services.itervalues():
            for identifier, stream in service.streams.iteritems():
                ret.append(stream)
        return ret
        
    def stop(self, contact):
        """
        Stops all the services for the provided contact.
        :param contact: miville.addressbook.Contact to start streaming with.
        Returns a DeferredList
        """
        contact_infos = self._create_contact_infos_from_contact(contact)
        session = self._get_session(contact_infos)
        comm = communication.get_channel_for_contact(contact)
        comm.send_stop(session)
        self._stop(session)

    def remote_stopped_streams(self, notif):
        """
        Called from the StreamsManager when remote host has stopped to stream.
        
        :param caller: Always None
        :param notif: dict with keys  
            {
            "contact":contact_infos.contact,
            "contact_name":contact_name,
            "message":msg,
            "success":False,
            }
        """
        log.info("REMOTE_STOPPED_STREAMS")
        self.api.notify(None, notif)
        notif["contact"].stream_state = STATE_STOPPED
    
    def remote_started_streams(self, notif):
        """
        Called from the StreamsManager when remote host has started to stream.
        
        :param caller: Always None
        :param notif: dict with keys  
            {
            "contact":contact_infos.contact,
            "contact_name":contact_name,
            "message":msg,
            "success":False,
            }
        """
        log.info("REMOTE_STARTED_STREAMS")
        if notif["success"]:
            notif["contact"].stream_state = STATE_STREAMING
        else:
            notif["contact"].stream_state = STATE_STOPPED
        self.api.notify(None, notif)


def _start(api):
    """
    Called only once by start() 
    Here, we define which streaming services are provided by the application.
    :rtype StreamsManager:
    """
    global _single_manager
    _single_manager = StreamsManager(api)
    # XXX: Just call the constructor for MilhouseFactory. Miville 
    # is not ready to have different services and if i see any more degrees of
    # abstraction/delegation I'm going to start crying - TM
    _single_manager.add_service(milhouse.MilhouseFactory())
    return _single_manager

def get_single_manager():
    """
    Returns the StreamsManager singleton.
    """
    global _single_manager
    if _single_manager is None:
        raise StreamError("Streams has not been initialized.")
    return _single_manager

def start(api):
    """
    Should be called only once from the core API.
    
    Starts the services. The services are responsible for registering their configuration
    fields, settings and profiles. 
    
    Registers the callbacks to the com_chan. 

    This function must be called from the API.
    """
    log.debug("streams.start")
    connectors.register_callback("streams_on_connect", communication.on_com_chan_connected, event="connect")
    connectors.register_callback("streams_on_disconnect", communication.on_com_chan_disconnected, event="disconnect")
    communication.set_api(api)
    _start(api) # this guy returns a manager...

