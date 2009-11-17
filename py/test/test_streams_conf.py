#!/usr/bin/env python 
# -*- coding: utf-8 -*-
"""
Unit tests for miville.streams.conf and miville.streams.milhouse
Streams configuration and actual streaming. 
"""
import os
import tempfile
import pprint
import warnings
import copy

from twisted.trial import unittest
from twisted.internet import defer
from twisted.internet import reactor
from twisted.python import failure
import zope.interface

from miville.utils import observer
from miville.streams import conf
from miville.streams import manager
from miville.streams import tools
from miville.streams import session
from miville.streams import milhouse 
from miville.streams import interfaces as streams_interfaces
# TODO: from miville.streams.engines import milhouse
from miville.utils import observer 

VERBOSE = False
#VERBOSE = True

from twisted.internet import base
base.DelayedCall.debug = True
import tempfile
FILE_NAME = tempfile.mktemp()

class DummyObserver(observer.Observer):
    def __init__(self, test_case):
        observer.Observer.__init__(self)
        self.test_case = test_case
        self.called = False
    def update(self, origin, key, value):
        self.called = True
        # print("update: %s:%s:%s" % (origin, key, value))

############################## ACTUAL TESTS #####################
class Test_01_Configuration(unittest.TestCase):
    def setUp(self):
        self.db = conf.get_single_db()
    def tearDown(self):
        pass
    def test_01_field(self):
        self.db.add_field("/dummy", default=2, desc="Dummy field")
        if "/dummy" not in self.db.fields.keys():
            self.fail("/dummy field should be in fields.")
        # TODO remove field
        self.db.remove_field("/dummy")
        if "/dummy" in self.db.fields.keys():
            self.fail("/dummy field should not be in fields anymore.")

    def test_02_add_field(self):
        self.db.add_field("/test/audio/bitrate", default=1000000, desc="Dummy field")
        self.db.add_field("/test/audio/channels", default=2, desc="Dummy field")

    def test_03_add_entries_and_setting(self):
        # to create entries, we must first create a setting
        setting = self.db.add_setting("Test setting")
        self.db.add_entry(setting.id, field_name="/test/audio/bitrate", value=7000)
        self.db.add_entry(setting.id, field_name="/test/audio/channels", value=1)
        entries = self.db.get_entries_for_setting(setting.id)
        # must contain them all, nothing more.
        for field_name in entries.keys():
            if field_name not in ["/test/audio/bitrate", "/test/audio/channels"]:
                self.fail("Field name %s should not be in that setting." % (field_name))
        for field_name in ["/test/audio/bitrate", "/test/audio/channels"]:
            if field_name not in entries.keys():
                self.fail("Setting should contain " + field_name)
        try:
            self.db.remove_field("/test/audio/bitrate") # should raise error 
            # since it is used by a setting
        except conf.ConfError:
            pass # successfully raise error
        else:
            self.fail("Removing a field used by a setting should raise error.")
        self.db.delete_setting(setting.id)
        if len(self.db.settings) != 0:
            self.fail("Could not remove setting.")
        self.db.remove_field("/test/audio/bitrate") #clean things up
        self.db.remove_field("/test/audio/channels")
        try:
            self.db.delete_setting(setting.id)
        except conf.ConfError:
            pass # as expected
        else:
            self.fail("Trying to delete a setting twice should throw error.")
        if len(self.db.fields.keys()) != 0:
            self.fail("There are still fields even if all were deleted.")

    def test_05_remove_setting(self):
        a = self.db.add_setting("A")
        self.db.delete_setting(a.id)
        b = self.db.add_setting("B")
        c = self.db.add_setting("C")
        if a is b:
            self.fail("Ref to same setting instance twice.")
        if len(self.db.settings.keys()) != 2:
            self.fail("There should be 2 settings.")
        # for k, v in self.db.settings.iteritems():
        #     print k, v
        self.db.delete_setting(b.id)
        self.db.delete_setting(c.id)
        if len(self.db.settings) != 0:
            self.fail("There are still settings left.")

    def test_04_unique_id(self):
        """
        Test if the unique ID generator is OK.
        """
        #TODO: improve this test.
        #TODO: move to a test
        pool = conf.IdPool()
        
        for i in range(3):
            pool.allocate()
        if pool.get_all() != [0, 1, 2]:
            self.fail("The allocated ID should be 0, 1 and 2.")
        pool.remove(0)
        if pool.get_all() != [1, 2]:
            self.fail("The allocated ID should be 1 and 2.")
        pool.allocate()
        if pool.get_all() != [0, 1, 2]:
            self.fail("The allocated ID should be 0, 1 and 2.")
        del pool

    def test_05_max_unique_id(self): # way too long, but it works.
        pool = conf.IdPool()
        try:
            while True:
                pool.allocate()
        except conf.ConfError:
            pass
        else:
            self.fail("Id pool should raise error if trying to allocate more than the max number of ID.")

    test_05_max_unique_id.skip = "this test is way too long, but it works." 

    def test_06_path_glob(self):
        sample = {}
        sample['/egg/spam'] = 2
        sample['/egg/brie'] = 8
        sample['/egg/ham'] = 0
        sample['/cheese/cheddar'] = 1
        sample['/cheese/brie'] = 3
        sample['/cheese/mozzarella'] = 4
        # testing "GLOB_STARTSWITH"
        res = conf.path_glob(sample, "/cheese", conf.GLOB_STARTSWITH)
        #print res
        should_have = ['/cheese/brie', '/cheese/cheddar', '/cheese/mozzarella']
        should_not = ['/egg/spam', '/egg/brie', '/egg/ham']
        for k in should_have:
            if not res.has_key(k):
                self.fail("GLOB_STARTSWITH : Dict should have key %s" % (k))
        for k in should_not:
            if res.has_key(k):
                self.fail("GLOB_STARTSWITH : Dict should not have key %s" % (k))
        # testing "GLOB_CONTAINS"
        res = conf.path_glob(sample, "/brie", conf.GLOB_CONTAINS)
        #print res
        should_have = ['/cheese/brie', '/egg/brie']
        should_not = ['/egg/spam', '/chesse/mozzarella', '/cheese/cheddar', '/egg/ham']
        for k in should_have:
            if not res.has_key(k):
                self.fail("GLOB_CONTAINS : Dict should have key %s" % (k))
        for k in should_not:
            if res.has_key(k):
                self.fail("GLOB_CONTAINS : Dict should not have key %s" % (k))
    
#    def XXtest_06_serialize(self):
#        self.db.add_field("/test/audio/codec", default="mp3", desc="Dummy field")
#        serialize.save(FILE_NAME, self.db)
#    
#    def XXtest_07_unserialize(self):
#        self.db = serialize.load(FILE_NAME)
#        self.db.remove_field("/test/audio/codec") #clean things up

class Test_01_Ports_Allocator(unittest.TestCase):
    def _tst(self, expected, value):
        if value != expected:
            self.fail("Expected value %s but got %s." % (expected, value))

    def test_01_add_remove(self):
        a = tools.PortsAllocator(minimum=10, increment=2, maximum=20)

        for value in [10, 12, 14, 16, 18, 20]:
            self._tst(value, a.allocate())
        try:
            value = a.allocate()
        except tools.PortsAllocatorError, e:
            pass
        else:
            self.fail("Ports allocator should have overflown. Got value %d." % (value))
        for value in [10, 12, 14, 16, 18, 20]:
            a.free(value)
        try:
            a.free(100)
        except tools.PortsAllocatorError, e:
            pass
        else:
            self.fail("Trying to free value %d should have raised an error." % (100))
            
    def test_02_add_many(self):
        a = tools.PortsAllocator(minimum=10, increment=2, maximum=20)
        values = a.allocate_many(6)
        a.free_many(values)
        values = a.allocate_many(3)
        values = a.allocate_many(3)
        try:
            value = a.allocate()
        except tools.PortsAllocatorError, e:
            pass
        else:
            self.fail("Ports allocator should have overflown. Got value %d." % (value))
            pass
        

class Test_01_Session(unittest.TestCase):
    def setUp(self):
        pass
    def tearDown(self):
        pass
    
    def test_serialize(self):
        data = {"x":2, "y":[1,2,3], "pi":3.14159}
        txt = session.serialize(data)
        data2 = session.unserialize(txt)
        if data != data2:
            self.fail("Data has changed during seralization/unserialization.")
    
    def test_request(self):
        req = session.SimpleRequest(session.REQUEST_OK)
        req.values[session.ATTR_PROTOCOL_VERSION] = "0.1"
        req.values[session.ATTR_COMMENT] = "Hello"
        req.values[session.ATTR_CONFIG_ENTRIES] = {"/cheese":"brie", "num":2}
        txt = session.request_to_text(req)
        if VERBOSE:
            print("\n-----------")
            print(txt)
            print("-----------")
        req2 = session.text_to_request(txt)
        if req.method != req2.method:
            self.fail("Request name changed during text/obj conversion.")
        if req.values != req2.values:
            self.fail("Attributes values changed during text/obj conversion.")

class Test_02_Asynchronous(unittest.TestCase):
    """
    tests the DeferredWrapper and DelayedWrapper
    """
    def test_01_deferred(self):
        def _later(wrapper):
            wrapper.callback(True)
        wrapper = tools.DeferredWrapper()
        deferred = wrapper.make_deferred()
        reactor.callLater(0.01, _later, wrapper)
        return deferred
    def test_02_later(self):
        def _later(test_case, arg2, kwarg1=None):
            if arg2 != "arg2":
                test_case.fail("Arg #2 should be \"arg2\".")
            if kwarg1 != "kwarg1":
                test_case.fail("KW Arg #1 should be \"kwarg1\".")
            return True
        def _callback(result, test_case):
            if result is not True:
                test_case.fail("Result should be True.")
        delayed = tools.DelayedWrapper()
        deferred = delayed.call_later(0.01, _later, self, "arg2", kwarg1="kwarg1")
        deferred.addCallback(_callback, self)
        return deferred
        
    def test_03_cancel_call_later(self):
        def _later(test_case):
            test_case.fail("_later should never have been called.")
            return fail.Failure(Exception("This should never be called"))
        def _callback(result, test_case):
            if result != "Cancelled":
                test_case.fail("Result should be the string \"cancelled\".")
        delayed = tools.DelayedWrapper()
        deferred = delayed.call_later(0.1, _later, self)
        deferred.addCallback(_callback, self)
        delayed.cancel("Cancelled")
        return deferred
    def test_04_deferred_list(self):
        def _cb(result, self):
            #print("overall success:")
            #print(result)
            return result
        def _eb(reason, self):
            # reason should be a failure.
            #print("overall failure:")
            #print(reason.getErrorMessage())
            return True
        #print("creating deferreds")
        l = []
        a = defer.Deferred()
        b = defer.Deferred()
        l.append(a)
        l.append(b)
        #l.append(defer.succeed(True))
        #l.append(defer.fail(failure.Failure(Exception("error"))))
        #print("creating dl wrapper")
        d = tools.deferred_list_wrapper(l)
        d.addCallback(_cb, self)
        d.addErrback(_eb, self)
        #print("trigger the callback")
        a.callback(True)
        #print("trigger the errback")
        b.errback(Exception("Errrororo message"))
    
_globals_03 = {}
class Test_03_Streams(unittest.TestCase):
    """
    Tests the Service and Stream interfaces as well as the manager.
    """
    def setUp(self):
        global _globals_03
        self.globals = _globals_03
    
    def test_01_start_manager(self):
        class DummyApi(observer.Subject):
            def update(self, origin, key, value):
                pass
        manager.start(DummyApi())
        self.globals["manager"] = manager.get_single_manager()
    
#    def test_02_add_dummy_service(self):
#        if not isinstance(self.globals["manager"], manager.ServicesManager):
#            self.fail("should be a manager.") # XXX
#        class DummyStream():
#            zope.interface.implements(streams_interfaces.IStream)
#            service_name = "dummy"
#            def __init__(self, service):
#                pass
#            def start(self, contact_infos, config_entries, mode):
#                return defer.succeed(True)
#            def stop(self):
#                return defer.succeed(True)
#        class DummyService():
#            zope.interface.implements(streams_interfaces.IService)
#            name = "dummy"
#            enabled = True
#            def __init__(self):
#                self.streams = {}
#                self.config_fields = {} # or list ?
#                self.config_db = None
#            def config_init(self, config_db):
#                #TODO: add fields.
#                self.config_db = config_db
#            def start(self, contact_infos, config_entries):
#                #TODO: get entries for profile
#                self.streams[0] = DummyStream(self) 
#                return self.streams[0].start(contact_infos, config_entries, 'recv')
#                #return defer.succeed(True)
#            def stop(self, contact_infos):
#                return self.streams[0].stop()
#                #return defer.succeed(True)
#            def stop_all(self):
#                return defer.succeed(True)
#        api = self.globals["manager"].api
#        self.globals["manager"].add_service(DummyService())
#    test_02_add_dummy_service.skip = "This might be useless..."
#
#    def test_03_start_dummy_service(self):
#        contact_infos = conf.ContactInfos() # default
#        #profile_id = None # dummy
#        config_entries = {}
#        return self.globals["manager"].start(contact_infos, config_entries) #profile_id)
#    test_03_start_dummy_service.skip = "...since it might change."
#
#    def test_04_stop_dummy_service(self):
#        contact_infos = None # dummy
#        return self.globals["manager"].stop(contact_infos)
#        # ok, let's no go further... you get the idea. 
#        # let's do some tests with the real thing: milhouse
#    test_04_stop_dummy_service.skip = "Let us test the real thing : milhouse."

_globals_04 = {}
class Test_04_Process_Manager(unittest.TestCase):
    """
    Tests the ProcessManager
    """
    def setUp(self):
        global _globals_04
        self.globals = _globals_04
    
    def test_04_start_and_stop_mplayer(self):
        """
        Mplayer process using twisted and JACK.
        """
        def _stop_callback(result, deferred):
            deferred.callback(True)
            return True

        def _stop_err(err, deferred):
            #print("ERROR %s" % (err))
            #return True
            deferred.errback(err)
            return True #return err
            
        def _later(deferred, manager):
            deferred2 = manager.stop()
            deferred2.addCallback(_stop_callback, deferred)
            deferred2.addErrback(_stop_err, deferred)
            #deferred2.callback()
            #deferred.callback(deferred2)
            
        def _start_err(err, manager):
            # stops reactor in case of error starting process
            #print("ERROR %s" % (err))
            #reactor.stop()
            #return True
            return err

        def _start_callback(result, manager):
            DURATION = 2.0
            deferred = defer.Deferred()
            reactor.callLater(DURATION, _later, deferred, manager)
            # stops the process
            #print(str(result))
            #deferred.addCallback(_stop)
            #return True #
            return deferred
        
        MOVIEFILENAME = "/var/tmp/excerpt.ogm"
        if not os.path.exists(MOVIEFILENAME):
            warnings.warn("File %s is needed for this test." % (MOVIEFILENAME))
        else:
            # starts the process
            #manager = tools.ProcessManager(name="xeyes", command=["xeyes"])
            #  "-vo", "gl2",
            TIMEOUT = 5.0 # seconds at start/stop process
            manager = tools.ProcessManager(name="mplayer", command=["mplayer", "-ao", "jack", MOVIEFILENAME], check_delay=TIMEOUT)
            deferred = manager.start()
            deferred.addCallback(_start_callback, manager)
            deferred.addErrback(_start_err, manager)
            return deferred

#    def test_03_start_and_stop_sleep_that_dies(self):
#        """
#        Catches when the process dies.
#        """
#        def _stop_callback(result, deferred, test_case):
#            global _globals_04
#            # XXX: calling stop() when done doesnt give any error anymore
#            #if result != "NO ERROR DUDE":
#            #    msg = "The process was still running and has been killed succesfully... Stop() should have created an error."
#            #    fail = failure.Failure(Exception(msg))
#            #    deferred.errback(fail)
#            #    #test_case.fail(msg) # for some reason this doesn;t work, since we returned a deferred ! IMPORTANT
#            
#            if not _globals_04["obs"].called:
#                raise Exception("Observer never called !!")
#            return True
#                #return fail
#
#        def _stop_err(err, deferred, test_case):
#            # That's what wer expected
#            deferred.callback(True)
#            #deferred.errback(err)
#            return "NO ERROR DUDE" #return err
#            #return err
#            
#        def _later(deferred, manager, test_case):
#            deferred2 = manager.stop()
#            deferred2.addErrback(_stop_err, deferred, test_case) # order matters ! this first.
#            deferred2.addCallback(_stop_callback, deferred, test_case)
#            
#        def _start_err(err, manager, test_case):
#            return err
#
#        def _start_callback(result, manager, test_case):
#            DURATION = 4.0
#            deferred = defer.Deferred()
#            reactor.callLater(DURATION, _later, deferred, manager, test_case)
#            return deferred
#        
#        # starts the process
#        manager = tools.ProcessManager(name="sleep", command=["sleep", "2"])
#        deferred = manager.start()
#        deferred.addCallback(_start_callback, manager, self)
#        deferred.addErrback(_start_err, manager, self)
#        return deferred # only possible to fail it by calling deferred.errback() !! 
#
#    test_03_start_and_stop_sleep_that_dies.skip = "Heavy changes in process management so this test must be updated."
    
    def test_01_executable_not_found(self):
        try:
            manager = tools.ProcessManager(name="dummy", command=["you_will_not_find_me"])
        except tools.ManagedProcessError:
            pass
        else:
            self.fail("Should have thrown error since executable not possible to find.")

    def _02_slot(self, key, value=None):
        """
        Callback slot for xeyes process manager signal.
        """
        #print("Slot got triggered with %s" % (key))
        if key == "start_success":
            pass
            self.globals["xeyes_start_success"] = True
        elif key == "start_error":
            pass
        elif key == "stop_success":
            self.globals["xeyes_stop_success"] = True
            pass
        elif key == "stop_error":
            pass
        elif key == "crashed":
            pass
        else:
            self.fail("Invalid signal key for slot. %s" % (key))
        
    def test_02_start_and_stop_xeyes(self):
        """
        xeyes process using twisted
        """
        def _stop_callback(result, deferred):
            deferred.callback(True)
            if not self.globals["xeyes_start_success"]:
                self.fail("Did not get start_success for process manager.")
            if not self.globals["xeyes_stop_success"]:
                self.fail("Did not get stop_success for process manager.")
            return True
        def _stop_err(err, deferred):
            deferred.errback(err)
            return True #return err
        def _later(deferred, manager):
            deferred2 = manager.stop()
            deferred2.addCallback(_stop_callback, deferred)
            deferred2.addErrback(_stop_err, deferred)
        def _start_err(err, manager):
            return err
        def _start_callback(result, manager):
            DURATION = 2.0
            deferred = defer.Deferred()
            reactor.callLater(DURATION, _later, deferred, manager)
            return deferred
            
        # starts the process
        #global _globals_04
        #obs = DummyObserver(self)
        #_globals_04["obs"] = obs # XXX Needs to be a global since observer uses a weakref !!!
        manager = tools.ProcessManager(name="xeyes", command=["xeyes", "-geometry", "640x480"])
        #obs.append(manager.subject)

        self.globals["xeyes_start_success"] = False
        self.globals["xeyes_stop_success"] = False
        manager.signal.connect(self._02_slot)
        #print(manager.subject.observers.values())
        #d = manager.subject.observers
        #for v in d.itervalues():
        #    print v
            #print("Subjects:" + str(manager.subject.observers))
        deferred = manager.start()
        deferred.addCallback(_start_callback, manager)
        deferred.addErrback(_start_err, manager)
        return deferred

_globals_05 = {}
#_milhouse_test_deferred = defer.Deferred()
class Test_05_Milhouse(unittest.TestCase):
    # trial test/test_streams_conf.Test_05_Milhouse
    def setUp(self):
        global _globals_05
        self.globals = _globals_05
        self.db = conf.get_single_db()

    def test_01_setup_milhouse_fields(self):
        global VERBOSE
        milhouse.setup_milhouse_fields(self.db)
        for field in ["/send/audio/source", "/both/audio/codec"]:
            if field not in self.db.fields.keys():
                # if VERBOSE:
                #    print self.db.fields.keys()
                self.fail("Field %s should exist." % (field))

    def test_02_setup_stream_setting(self):
        def setup_milhouse_test_setting(db):
            """
            Simple setting for local streaming test.
            """
            setting = db.add_setting("AV test", desc="Setting to stream locally test sources.")
            id = setting.id
            db.add_entry(id, "/both/video/codec", "mpeg4")
            db.add_entry(id, "/both/audio/codec", "raw")
            db.add_entry(id, "/both/video/port", 8000)
            db.add_entry(id, "/both/audio/port", 8010)
            db.add_entry(id, "/both/audio/numchannels", 2)
            db.add_entry(id, "/recv/audio/sink", "jackaudiosink")
            db.add_entry(id, "/recv/video/sink", "xvimagesink")
            db.add_entry(id, "/send/audio/source", "audiotestsrc")
            db.add_entry(id, "/send/video/source", "videotestsrc")
            # TODO: remove addresses from conf
            db.add_entry(id, "/recv/network/remote_address", "127.0.0.1")
            db.add_entry(id, "/send/network/remote_address", "127.0.0.1")
            return setting
        setting = setup_milhouse_test_setting(self.db)
        # print(self.db.settings)
        #for k, v in self.db.settings.iteritems():
        #    print "Setting #%d = %s" % (k, v)
        # print("Chosen setting for milhouse is :")
        # print(setting)
        # print(db.get_entries_for_setting(setting.id))
        # start_milhouse(db, setting_id)
        # print("Starting over.")
        # profile = milhouse.setup_milhouse_settings(db)
        self.globals["entries"] = self.db.get_entries_for_setting(setting.id) # XXX TODO FIXME
        #if VERBOSE:
        #    print self.globals["entries"]

    def test_03_prepare_arguments(self):
        was_verbose = False
        if milhouse.VERBOSE:
            was_verbose = True
            milhouse.VERBOSE = False
        #TODO : we assume this function will be used by the future version... we'll see
        self.globals["commands"] = milhouse.config_fields_to_milhouse_opts(self.globals["entries"])
        for word in ["--videoport", "--audioport", "--numchannels", "--address"]:
            for mode in ["sender", "receiver"]:
                if self.globals["commands"][mode].find(word) == -1:
                    self.fail("Could not find string \"%s\" in the command." % (word))
        # self.globals["commands"] = dummy_opts() # just testing for now
        if was_verbose:
            milhouse.VERBOSE = True

    def test_04_start_stop_streams(self):
        def _stop_callback(result, deferred):
            deferred.callback(True)
            return True
        def _stop_err(err, deferred):
            deferred.errback(err)
            return True #return err # ????????????????????????????????????????????????
        def _later(deferred, service):
            global _globals_05
            infos = _globals_05["contact_infos"]
            session_desc = _globals_05["session_desc"]
            deferred2 = service.stop(session_desc)
            deferred2.addCallback(_stop_callback, deferred)
            deferred2.addErrback(_stop_err, deferred)
        def _eb_start(err, service):
            return err
        def _cb_start(result, service):
            DURATION = 10.0
            global VERBOSE
            #if VERBOSE:
            print("Will stop milhouse in %f seconds." % (DURATION))
            deferred = defer.Deferred()
            reactor.callLater(DURATION, _later, deferred, service)
            return deferred
        # ----- here we go: --------
        contact_infos = conf.ContactInfos() # TODO: not use default args.
        self.globals["contact_infos"] = contact_infos
        entries = self.globals["entries"]
        self.timeout = 15.0 # timeout attribute on your unit test method. 
        service = milhouse.MilhouseFactory()
        session_desc = session.SessionDescription(contact_infos=contact_infos)
        _globals_05["session_desc"] = session_desc
        session_desc.add_stream_desc(session.DIRECTION_TO_ANSWERER, service_name="Milhouse", entries=copy.deepcopy(entries))
        session_desc.add_stream_desc(session.DIRECTION_TO_OFFERER, service_name="Milhouse", entries=copy.deepcopy(entries))
        service.prepare_session(session_desc)
        # prepare_session on alice only (no bob)
        print("we care about alice : %s" % (session_desc.streams_to_offerer[0].entries))
        self.globals["service"] = service
        self.globals["milhouse_success"] = False
        service.config_init(self.db)
        # service.start should not send any message to the remote agent
        # all this work should be done at prepare_session time.
        deferred = service.start(session_desc) #contact_infos, entries, entries, role="alice") # "alpha"
        obs = DummyObserver(self)
        self.globals["obs"] = obs # XXX Needs to be a global since observer uses a weakref !!!
        obs.append(service.subject) #TODO: check if observer has been triggered.
        deferred.addCallback(_cb_start, service)
        deferred.addErrback(_eb_start, service)
        return deferred
    test_04_start_stop_streams.skip = "Too much heavy changes."

