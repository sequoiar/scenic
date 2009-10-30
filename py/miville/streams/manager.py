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

from zope import interface
from twisted.internet import reactor
from twisted.internet import defer
from twisted.python import failure

from miville.streams import communication
from miville.errors import * # StreamError
from miville.streams import conf
from miville.streams import tools
from miville.streams import milhouse
from miville.utils import observer
from miville.utils import log
from miville import connectors

log = log.start("info", True, False, "streams.manager") 

_single_manager = None # singleton

class SessionDescription(object):
    """
    We need to wrap the informations about the current session in an object.
    This will be much easier to manager.
    Contains stuff that is negociated between the peers.
    """
    # Not used yet.
    def __init__(self, contact_infos=None, alice_entries=None, bob_entries=None, role="alice"):
        """
        :param contact_infos: ContactInfos object
        :alice_entries: Dict of configuration entries for initiator peer.
        :bob_entries: Dict of configuration entries for the othe peer.
        :param role: str Either "alice" or "bob"
        """
        self.contact_infos = contact_infos
        self.alice_entries = alice_entries
        self.bob_entries = bob_entries
        self.role = role

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

class ServicesManager(observer.Observer):
    """
    This is the only class that should be used from the core api.
    
    Wasn't it called StreamsManager ? 
    
    The service manager is an observer, since it receives notifications from 
    its services.
    """
    def __init__(self, api=None):
        self.api = api
        self.config_db = conf.get_single_db()
        self.services = {} # key are class names
        
    def update(self, origin, key, value):
        """
        See miville.utils.observer.Subject.update()
        """
        pass
        # TODO: encapsulate self.api.update(origin, key, value)
    
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

    def _get_config_entries_for_contact(self, contact):
        """
        :param contact: miville.addressbook.Contact to start streaming with.
        Returns a dict of config entries.
        """
        profile_id = None
        entries = {}
        try:
            profile_id = int(contact.profile_id)
        except TypeError:
            raise StreamError("Contact %s does not have a profile associated with it." % (contact.name))
        try:
            entries = self.config_db.get_entries_for_profile(profile_id)
        except conf.ConfError, e:
            log.error(e.message)
            self.config_db.get_entries_for_profile(milhouse.TEST_PROFILE)
        # TODO: order or overriding?
        return entries
        
    def _cb_start(self, channel, contact, alice_entries, bob_entries):
        """
        Got START message from remote host
        
        Called from miville.streams.communication.StreamsCommunication.on_remote_message.

        :param channel: miville.streams.communication.StreamsCommunication
        :param contact: miville.addressbook.Contact object
        :param config_entries: dict of entries
        """
        #TODO manage deferred within local/remote hosts
        log.info("r start from remote  %s %s %s" % (contact, alice_entries, bob_entries))
        contact_infos = self._create_contact_infos_from_contact(contact)
        
        def _service_start_success(result, self, contact_infos, alice_entries, bob_entries, service):
            try:
                contact_name = contact_infos.contact.name
            except ValueError:
                contact_name = contact_infos.get_id()
            msg = "Contact %s initiated a stream of the %s service." % (contact_name, service.name)
            log.info(msg)
            try:
                notif = {
                    "contact":contact_infos.contact,
                    "contact_name":contact_name,
                    "message":msg,
                    "success":True,
                    }
                self.api.remote_started_streams(None, notif)
                #self.api.notify(None, notif, "remote_started_streams")
            except Exception, e:
                log.error(e.message)
                raise
            return result #TODO change this objects's state.
        def _service_start_failure(reason, self, contact_infos, alice_entries, bob_entries, service):
            try:
                contact_name = contact_infos.contact.name
            except ValueError:
                contact_name = contact_infos.get_id()
            msg = "Contact %s failed to initiate a stream of the %s service." % (contact_name, service.name)
            msg += "Error message:\n" + reason.getErrorMessage()
            log.info(msg)
            try:
                notif = {
                    "contact":contact_infos.contact,
                    "contact_name":contact_name,
                    "message":msg,
                    "success":False,
                    }
                self.api.remote_started_streams(None, notif)
                #self.api.notify(None, notif, "remote_started_streams") #TODO: pass a dict?
            except Exception, e:
                log.error(e.message)
                raise
            log.error("Error starting stream of the %s service. Tried by contact %s." % (service.name, contact_name))
            return reason # we do not catch the error
            #return True # we catch the error.
            #TODO change this objects's state.
        dl = []
        for service in self.services.itervalues():
            # it does nothing right now if in bob role:
            alice_entries, bob_entries = service.prepare_session(alice_entries, bob_entries, role="bob")
            service_deferred = service.start(contact_infos, alice_entries, bob_entries, "bob")
            service_deferred.addCallback(_service_start_success, self, contact_infos, alice_entries, bob_entries, service)
            service_deferred.addErrback(_service_start_failure, self, contact_infos, alice_entries, bob_entries, service)
            dl.append(service_deferred)
            
        # TODO: does that report success/failure correctly?
        deferred = tools.deferred_list_wrapper(dl)
        def _on_fail(reason):
            log.debug("We catched the error in _cb_start.")
            return True
        deferred.addErrback(_on_fail)
        return deferred
    
    def _cb_stop(self, channel, contact):
        """
        Got STOP message from remote host
        
        Called from miville.streams.communication.StreamsCommunication.on_remote_message.

        :param channel: miville.streams.communication.StreamsCommunication
        :param contact: miville.addressbook.Contact object
        """
        def _service_stop_success(result, self, contact_infos):
            try:
                contact_name = contact_infos.contact.name
            except ValueError:
                contact_name = contact_infos.get_id()
            msg = "Contact %s stopped a stream of the %s service." % (contact_name, service.name)
            log.info(msg)
            try:
                notif = {
                    'contact_name':contact_name,
                    'message':msg,
                    'contact':contact_infos.contact,
                    'success':True
                    }
                #self.api.notify(None, notif, "remote_stopped_streams")
                self.api.remote_stopped_streams(None, notif)
            except Exception, e:
                log.error(e.message)
                raise
            return result #TODO change this objects's state.
        def _service_stop_failure(reason, self, contact_infos):
            try:
                contact_name = contact_infos.contact.name
            except ValueError:
                contact_name = contact_infos.get_id()
            msg = "Contact %s failed to initiate a stream of the %s service." % (contact_name, service.name)
            msg += "Error message:\n" + reason.getErrorMessage()
            log.info(msg)
            try:
                notif = {
                    "contact":contact_infos.contact,
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
            return reason # we do not catch the error
            #TODO change this objects's state.
        log.info("Received STOP from remote  %s " % (contact))
        contact.stream_state = 3 # stopping !
        contact_infos = self._create_contact_infos_from_contact(contact)
        dl = []
        for service in self.services.itervalues():
            service_deferred = service.stop(contact_infos)
            service_deferred.addCallback(_service_stop_success, self, contact_infos)
            service_deferred.addErrback(_service_stop_failure, self, contact_infos)
            dl.append(service_deferred)
        deferred = tools.deferred_list_wrapper(dl)
        # TODO: does that report success/failure correctly?
        return deferred
    
    def list_streams(self):
        """
        Returns the list of all current Stream instances.
        """
        #TODO: use it.
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

        :param contact: miville.addressbook.Contact to start streaming with.
        Returns a DeferredList
        """
        #TODO: if not connected, connect
        #TODO: manage port numbers using a negociation.
        #TODO: asymetrical config entries (device, etc.)
        #TODO: it is the service that should duplicate the config entries for the initiator and the responder. (A and B) and overide the port numbers accordingly. We should add it a prepare_startup method, or so.
        log.debug("manager.start")
        try:
            contact_infos = self._create_contact_infos_from_contact(contact)
        except StreamError, e: # in case we are not connected to contact.
            log.debug("We are not connected to contact? Maybe bob has errors in the comchan timing of getmyip thing.")
            return defer.fail(failure.Failure(e))
            # raise
        # FIXME: this is hard-coded
        if contact.profile_id == 0:
            contact.profile_id = milhouse.TEST_PROFILE
            log.warning("setting contact %s profile ID to %d" % (contact.name, milhouse.TEST_PROFILE))
        config_entries = self._get_config_entries_for_contact(contact)
        # get communication channel.
        try:
            comm = communication.get_channel_for_contact(contact)
        except StreamError, e:
            #raise
            return defer.fail(failure.Failure(e))
        # duplicate the config entries. One dict for each stream
        alice_entries = copy.deepcopy(config_entries)
        bob_entries = copy.deepcopy(config_entries)
        dl = []
        # prepare config entries for alice and bob streams
        for service in self.services.itervalues():
            log.info("preparing service %s to stream with contact %s" % (service.name, contact_infos.get_id()))
            alice_entries, bob_entries = service.prepare_session(alice_entries, bob_entries, role="alice", contact_infos=contact_infos)
            log.debug("Alpha ports: %s. Bravo ports: %s" % (
                conf.path_glob(alice_entries, "port", conf.GLOB_CONTAINS),
                conf.path_glob(bob_entries, "port", conf.GLOB_CONTAINS)))
        # send message to "bob" remote host
        comm.send_start(contact_infos, alice_entries, bob_entries)
        # TODO: add comm_deferred as well.
        #comm_deferred = comm.send_start(contact_infos, alice_entries, bob_entries)
        # dl.append(comm_deferred)
        def _service_success(result, self, contact_infos, alice_entries, bob_entries, service):
            log.debug("successfully started stream of the %s service." % (service.name))
            return result #TODO change this object's state
        def _service_failure(reason, self, contact_infos, alice_entries, bob_entries, service):
            #log.error("Error starting services. (as alice) " + str(reason))
            log.error("Error starting stream of the %s service." % (service.name))
            return reason  #TODO change this objects's state.
        for service in self.services.itervalues():
            serv_deferred = service.start(contact_infos, alice_entries, bob_entries)
            serv_deferred.addCallback(_service_success, self, contact_infos, alice_entries, bob_entries, service)
            serv_deferred.addErrback(_service_failure, self, contact_infos, alice_entries, bob_entries, service)
            dl.append(serv_deferred)
        deferred = tools.deferred_list_wrapper(dl)
        #deferred_list = defer.DeferredList(dl, consumeErrors=False) 
        # TODO: does that report success/failure correctly?
        return deferred #_list
    
    def list_all_streams(self):
        """
        Returns a list of all active streams.
        """
        # TODO: add contact or service arg
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
        #TODO: return Deferred not DeferredList
        contact_infos = self._create_contact_infos_from_contact(contact)
        # communication channel
        comm = communication.get_channel_for_contact(contact)
        # send stop to remote host
        dl = []
        comm.send_stop(contact_infos)
        # TODO: deferred for send stop
        #comm_deferred = comm.send_stop(contact_infos)
        #dl.append(comm_deferred)
        # stop all services
        for service in self.services.itervalues():
            serv_deferred = service.stop(contact_infos)
            dl.append(serv_deferred)
        deferred = tools.deferred_list_wrapper(dl)
        # TODO: add call/errbacks to setup this object's state.
        return deferred
        
    def stop_all(self):
        """
        Stops all streams.
        returns Deferred
        """
        #TODO: send message to each connected host.
        # TODO; use tools.deferred_list_wrapper
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
    # from miville.streams import milhouse
    global _single_manager
    _single_manager = ServicesManager(api)
    to_load = [] 
    to_load.append(milhouse.MilhouseService)
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

