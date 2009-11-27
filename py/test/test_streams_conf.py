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
from miville.utils import log 
from miville.streams import conf
from miville.streams import manager
from miville.streams import tools
from miville.streams import milhouse 
from miville.streams import constants 
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
        """ Database (and more generally, state) persist between tests """
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
        """ Fields have already been created previous tests. """
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
    
    def test_07_save_database(self):
        """ Save to json """
        setting = self.db.add_setting("Test setting")
        setting2 = self.db.add_setting("Test setting2")
        self.db.add_field("/test/audio/bitrate", default=1000000, desc="Dummy field")
        self.db.add_field("/test/audio/channels", default=2, desc="Dummy field")
        self.db.add_entry(setting.id, field_name="/test/audio/bitrate", value=7000)
        self.db.add_entry(setting.id, field_name="/test/audio/channels", value=1)
        self.db.add_entry(setting2.id, field_name="/test/audio/bitrate", value=8000)
        self.db.add_entry(setting2.id, field_name="/test/audio/channels", value=2)
        self.db.save_as_json('/var/tmp/test_07_save_database.json')
    
    def test_08_load_database(self):
        """ load database from json """
        # FIXME: this should test more stuff (i.e. the resulting state of self.db 
        # and not depend on the previous test having been executed
        self.db.add_field("/test/audio/bitrate", default=1000000, desc="Dummy field")
        self.db.add_field("/test/audio/channels", default=2, desc="Dummy field")
        self.db.load_from_json('/var/tmp/test_07_save_database.json')
        entries = self.db.get_entries_for_setting(4)
        try: 
            if entries["/test/audio/channels"] != 1:
                self.fail("field %s equal to %d instead of %d" % ("/test/audio/channels", entries["/test/audio/channels"], 1))
            if entries["/test/audio/bitrate"] != 7000:
                self.fail("field %s equal to %d instead of %d" % ("/test/audio/bitrate", entries["/test/audio/bitrate"], 7000))
        except Exception,e:
            self.fail("could not access entries: %s" % (e.message))
    
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
        a = tools.PortsAllocator(minimum=2000, increment=2, maximum=2010)

        # value = 2000; value < 2012; value += 2
        for value in xrange(2000, 2012, 2):
            self._tst(value, a.allocate())
        try:
            value = a.allocate()
        except tools.PortsAllocatorError, e:
            pass
        else:
            self.fail("Ports allocator should have overflown. Got value %d." % (value))
        # value = 2000; value < 2012; value += 2
        for value in xrange(2000, 2012, 2):
            a.free(value)
        try:
            a.free(100)
        except tools.PortsAllocatorError, e:
            pass
        else:
            self.fail("Trying to free value %d should have raised an error." % (100))
            
    def test_02_add_many(self):
        a = tools.PortsAllocator(minimum=2000, increment=2, maximum=2010)
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
        
class Test_01_Streamer_Problem(unittest.TestCase):
    def test_01_str(self):
        """ Mostly just to make sure the error handling stays sane """
        error = constants.STATE_ERROR
        message = "the pipeline is broken"
        details = "these would be details"

        errstr = str(tools.StreamerProblem(error, message, details))
        if errstr != '%s: %s %s' % (error, message, details):
            self.fail("stringified SteamerProblem does not match expected string %s" % (errstr))
        

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
    test_serialize.skip = 'Functionality not used yet'
    
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
    test_request.skip = 'Functionality not used yet'

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
    


_globals_05 = {}
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
        # XXX : disabled
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
        session_desc = manager.Session(contact_infos=contact_infos)
        _globals_05["session_desc"] = session_desc
        session_desc.add_stream(constants.DIRECTION_TO_ANSWERER, service_name="milhouse", entries=copy.deepcopy(entries))
        session_desc.add_stream(constants.DIRECTION_TO_OFFERER, service_name="milhouse", entries=copy.deepcopy(entries))
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

