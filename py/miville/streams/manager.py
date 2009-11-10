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
"""
import copy
import time

from zope import interface
from twisted.internet import reactor
from twisted.internet import defer
from twisted.python import failure

from miville.streams import communication
from miville.errors import * # StreamError
from miville.streams import conf
from miville.streams import tools
from miville.streams import milhouse
from miville.streams import session
from miville.utils import log
from miville import connectors

log = log.start("debug", True, False, "streams.manager") 

_single_manager = None # singleton

class StreamingNotification(object):
    """
    Let's wrap the failure/success notification in a class.
    """
    # Not used yet.
    #TODO: pass StreamingSession object ?
    def __init__(self, contact=None, message="", details="", success=True, role="alice", stream_state=None):
        """
        :param contact: addressbook.Contact object
        :param message: str
        :param details: str
        :param success: bool
        :param stream_state: str One of the miville.streams.states.STATE*
        """
        self.contact=contact, 
        try:
            self.contact_name = contact.name
        except ValueError:
            self.contact_name = "unknown"
        self.message = message
        self.details = details
        self.success = success
        self.role = role
        self.streaming_state = stream_state

class ServicesManager(object):
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
                # Adds a subject to be observed by this Observer instance.
                
                #service.append(service.subject) # manager = self
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
        Returns a miville.streams.session.SessionDescription instance.
        
        It also add the created session to the sessions dict.
        If role is "bob", you must provide a request instance.
        Raises a StreamError if session already exists.
        """
        kwargs = {}
        if role == session.ROLE_OFFERER:
            pass #role = session.ROLE_OFFERER
        else: # bob
            role = session.ROLE_ANSWERER
            kwargs["session_id"] = session_id # TODO: remove this
            #TODO: kwargs["time_started"] = time_started
            kwargs["time_started"] = time.time() #TODO: remove this
        session_key = contact_infos.get_id()
        if self.sessions.has_key(session_key):
            raise StreamError("Streams manager has already a session for contact %s" % (session_key))
        log.debug("will create session desc. role %s" % (role))
        session_desc = session.SessionDescription(contact_infos=contact_infos, role=role, **kwargs)
        self.sessions[session_key] = session_desc
        log.info("Creating streaming session with %s" % (session_key))
        # TODO: remove those two guys
        return session_desc
    
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
        session_desc = self._get_session(contact_infos)
        session_key = contact_infos.get_id()
        log.debug("deleting session for contact %s" % (contact_infos.get_id()))
        if session_desc.stream_state in [session.STATE_IDLE, session.STATE_STOPPED, session.STATE_ERROR]:
            del self.sessions[session_key]
    
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
            #raise StreamError(
            msg = "Contact %s does not have a profile associated with it." % (contact.name)
            profile_id = -1
        except AttributeError:
            #raise StreamError("Contact %s does not have a profile associated with it." % (contact.name))
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
        # TODO: order or overriding?
        return entries
        
    def _cb_ack(self, channel, contact):
        log.info("Got ACK from remote contact %s" % (contact.name))
        
    def _cb_start(self, channel, contact, alice_entries, bob_entries):
        """
        Got START message from remote host
        
        Called from miville.streams.communication.StreamsCommunication.on_remote_message.

        :param channel: miville.streams.communication.StreamsCommunication
        :param contact: miville.addressbook.Contact object
        :param config_entries: dict of entries
        """
        log.info("r start from remote  %s %s %s" % (contact, alice_entries, bob_entries))
        contact_infos = self._create_contact_infos_from_contact(contact)
        #TODO: provide time_start and session_id from remote !!!!!!!
        try:
            session_desc = self._create_session(session_id=None, time_started=None, contact_infos=contact_infos, role=session.ROLE_ANSWERER)
        except StreamError, e:
            log.error("We got start even if a session was already created. We raise an error, but it should not destroy any existing stream or negotiation process.")
            return failure.Failure(e)
        session_desc.add_stream_desc(direction=session.DIRECTION_TO_OFFERER, service_name="Milhouse", entries=alice_entries)
        session_desc.add_stream_desc(direction=session.DIRECTION_TO_ANSWERER, service_name="Milhouse", entries=bob_entries)
        for service in self.services.itervalues():
            service.prepare_session(session_desc)
        comm = communication.get_channel_for_contact(contact)
        comm.send_ok_start(session_desc)
        return self._start(session_desc)
    
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
        try: # XXX
            session_desc = self._get_session(contact_infos)
        except StreamError, e:
            log.error("No session for remote !!")
            return failure.Failure(e)
        session_desc.streams_to_offerer[0].entries = alice_entries # FIXME
        session_desc.streams_to_answerer[0].entries = bob_entries # FIXME
        comm = communication.get_channel_for_contact(contact)
        comm.send_ack(session_desc)
        return self._start(session_desc)

    def _cb_stop(self, channel, contact):
        """
        Got STOP message from remote host
        
        Called from miville.streams.communication.StreamsCommunication.on_remote_message.

        :param channel: miville.streams.communication.StreamsCommunication
        :param contact: miville.addressbook.Contact object
        """
        comm = None # TODO : send OK
        log.info("Received STOP from remote  %s " % (contact))
        contact.stream_state = 3 # stopping !
        contact_infos = self._create_contact_infos_from_contact(contact)
        try:
            session_desc = self._get_session(contact_infos)
        except StreamError, e:
            raise
        return self._stop(session_desc)

    def _stop(self, session_desc):
        def _service_stop_success(result, self, service, session_desc):
            if session_desc.role == session.ROLE_ANSWERER:
                try:
                    contact_name = session_desc.contact_infos.contact.name
                except ValueError:
                    contact_name = session_desc.contact_infos.get_id()
                msg = "Contact %s stopped a stream of the %s service." % (contact_name, service.name)
                log.info(msg)
                try:
                    notif = {
                        'contact_name':contact_name,
                        'message':msg,
                        'contact':session_desc.contact_infos.contact,
                        'success':True
                        }
                    #self.api.notify(None, notif, "remote_stopped_streams")
                    self.api.remote_stopped_streams(None, notif)
                except Exception, e:
                    log.error(e.message)
                    raise
            
            self._delete_session(session_desc.contact_infos)
            return result #TODO change this objects's state.
        def _service_stop_failure(reason, self, service, session_desc):
            if session_desc.role == session.ROLE_ANSWERER:
                try:
                    contact_name = session_desc.contact_infos.contact.name
                except ValueError:
                    contact_name = session_desc.contact_infos.get_id()
                msg = "Contact %s failed to initiate a stream of the %s service." % (contact_name, service.name)
                msg += "Error message:\n" + reason.getErrorMessage()
                log.info(msg)
                try:
                    notif = {
                        "contact":session_desc.contact_infos.contact,
                        "contact_name":contact_name,
                        "message":msg,
                        "success":False,
                        }
                    #self.api.notify(None, notif, "remote_stopped_streams")
                    self.api.remote_stopped_streams(None, notif)
                except Exception, e:
                    log.error(e.message)
                    raise
                log.error("Error stopping stream of the %s service. Tried by contact %s." % (service.name, contact_name))
            self._delete_session(session_desc.contact_infos)
            return reason # we do not catch the error
            #TODO change this objects's state.
        dl = []
        for service in self.services.itervalues():
            service_deferred = service.stop(session_desc)
            service_deferred.addCallback(_service_stop_success, self, service, session_desc)
            service_deferred.addErrback(_service_stop_failure, self, service, session_desc)
            dl.append(service_deferred)
        deferred = tools.deferred_list_wrapper(dl)
        return deferred
    
    def list_streams(self):
        """
        Returns the list of all current Stream instances.
        """
        streams = []
        for service in self.services.itervalues():
            for stream in service.streams.itervalues():
                streams.append(stream)
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
        streams = []
        for service in self.services.itervalues():
            for stream in service.list_streams_for_contact(contact_infos):
                streams.append(stream)
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
            return defer.fail(failure.Failure(e))
        config_entries = self._get_config_entries_for_contact(contact)
        try:
            comm = communication.get_channel_for_contact(contact)
        except StreamError, e:
            return defer.fail(failure.Failure(e))
        try:
            contact_name = contact_infos.contact.name
        except ValueError:
            contact_name = str(contact_infos.__dict__)
        try:
            session_desc = self._create_session(contact_infos=contact_infos, role=session.ROLE_OFFERER) 
        except StreamError, e:
            log.error(e.message)
            raise
            return defer.fail(failure.Failure(e))
        offerer_entries = copy.deepcopy(config_entries)
        answerer_entries = copy.deepcopy(config_entries)
        session_desc.add_stream_desc(service_name="Milhouse", entries=offerer_entries, direction=session.DIRECTION_TO_OFFERER)
        session_desc.add_stream_desc(service_name="Milhouse", entries=answerer_entries, direction=session.DIRECTION_TO_ANSWERER)
        dl = []
        for service in self.services.itervalues():
            log.info("preparing service %s to stream with contact %s" % (service.name, contact_infos.get_id()))
            service.prepare_session(session_desc)
            log.debug("alice ports: %s. bob ports: %s" % (conf.path_glob(offerer_entries, "port", conf.GLOB_CONTAINS), conf.path_glob(answerer_entries, "port", conf.GLOB_CONTAINS)))
            log.debug("config entries : %s %s" % (offerer_entries, answerer_entries))
        comm.send_start(session_desc)# contact_infos, offerer_entries, answerer_entries)
        return defer.succeed("Initiating streams with contact %s."  % (contact_name))
        #return self._start(session_desc) # TODO: do this once bob answered

    def _start(self, session_desc):
        """
        Actually start the streamers. 
        """
        def _service_success(result, self, session_desc, service):
            """
            Success callback
            """
            if session_desc.role == session.ROLE_ANSWERER:
                try:
                    contact_name = session_desc.contact_infos.contact.name
                except ValueError:
                    contact_name = session_desc.contact_infos.get_id()
                msg = "Contact %s succeeded to initiate a stream of the %s service." % (contact_name, service.name)
                log.info(msg)
                try:
                    notif = {
                        "contact":session_desc.contact_infos.contact,
                        "contact_name":contact_name,
                        "message":msg,
                        "success":False,
                        }
                    self.api.remote_started_streams(None, notif)
                except Exception, e:
                    log.error(e.message)
                    raise
                log.info("successfully started stream of the %s service. Tried by contact %s." % (service.name, contact_name))
                return result 
                #TODO change this objects's state.
            else:
                log.debug("successfully started stream of the %s service." % (service.name))
                return result #TODO change this object's state
        
        def _service_failure(reason, self, session_desc, service):
            try:
                contact_name = session_desc.contact_infos.contact.name
            except ValueError:
                contact_name = session_desc.contact_infos.get_id()
            if session_desc.role == session.ROLE_ANSWERER:
                msg = "Contact %s failed to initiate a stream of the %s service." % (contact_name, service.name)
                msg += "Error message:\n" + reason.getErrorMessage()
                log.info(msg)
                try:
                    notif = {
                        "contact":session_desc.contact_infos.contact,
                        "contact_name":contact_name,
                        "message":msg,
                        "success":False,
                        }
                    self.api.remote_started_streams(None, notif)
                except ValueError, e:
                    log.error(e.message)
            else:
                #log.error("Error starting services. (as alice) " + str(reason))
                log.error("Error starting stream of the %s service." % (service.name))
            return reason  #TODO change this objects's state.

        dl = []
        for service in self.services.itervalues():
            serv_deferred = service.start(session_desc) 
            serv_deferred.addCallback(_service_success, self, session_desc, service)
            serv_deferred.addErrback(_service_failure, self, session_desc, service)
            dl.append(serv_deferred)
        deferred = tools.deferred_list_wrapper(dl)
        return deferred
    
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
        session_desc = self._get_session(contact_infos)
        comm = communication.get_channel_for_contact(contact)
        comm.send_stop(session_desc)
        return self._stop(session_desc)
        
    def stop_all(self):
        """
        Stops all streams.
        returns Deferred
        """
        raise NotImplementedError("Need to work on this.")
        #TODO: send message to each connected host.
        dl = []
        for service in self.services:
            dl.append(service.stop_all())
        return defer.DeferredList(dl, consumeErrors=True)

def _start(api):
    """
    Should be called only once from the core API.
    
    Starts the services. The services are responsible for registering their configuration
    fields, settings and profiles. 
    
    Here, we define which streaming services are provided by the application.
    :rtype ServicesManager:
    """
    global _single_manager
    _single_manager = ServicesManager(api)
    to_load = [] 
    to_load.append(milhouse.MilhouseFactory)
    for class_to_load in to_load:
        _single_manager.add_service(class_to_load())
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
    Registers the callbacks to the com_chan. 

    This function must be called from the API.
    """
    log.debug("streams.start")
    connectors.register_callback("streams_on_connect", communication.on_com_chan_connected, event="connect")
    connectors.register_callback("streams_on_disconnect", communication.on_com_chan_disconnected, event="disconnect")
    communication.set_api(api)
    #communication.check_for_milhouse()
    #manager.start(api)
    _start(api) # this guy returns a manager...

