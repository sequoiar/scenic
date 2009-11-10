#!/usr/bin/env python 
# -*- coding: utf-8 -*-
# """
# Interfaces for the streams.
# """
# from zope import interface
# from miville.streams import conf
# 
# class IService(interface.Interface):
#     """
#     A service provides streams between contacts.
#     
#     There should be only once service of each type. (singleton)
#     """
#     streams = interface.Attribute("""Dict of current streams. The ID of the contact_info should be their keys.""")
#     name = interface.Attribute("""Name of the service.""")
#     subject = interface.Attribute("""Subject that can notify observers. It is how other classes can be warned about changes that occur to this object. See miville.utils.observer.Subject.""")
#     config_db = interface.Attribute("""Configuration database. See miville.streams.conf.Database""")
#     enabled = interface.Attribute("""Enabled or not. Boolean static class attribute.""")
#     config_fields = interface.Attribute("""Dict of onfiguration fields. See miville.streams.conf.Field.""")
#     #TODO: notification_keys = interface.Attribute("""List of possible notification keys.""")
#     
#     def config_init(self, config_db):
#         """
#         Initializes the configuration fields for this service.
#         :rtype None:
#         """
#         pass
#     
#     def start(self, contact_infos, config_entries):
#         """
#         Starts zero or more streams of this service for the provided contact.
#         
#         :param contact_infos: miville.streams.conf.ContactInfos object.
#         :param config_profile: int Corresponds to a miville.streams.conf.Profile ID.
#         returns DeferredList
#         """
#         pass
#     
#     def stop(self, contact_infos):
#         """
#         Stops zero or more streams of this service for the provided contact.
#         
#         returns DeferredList
#         """
#         pass
#     
#     def stop_all(self):
#         """
#         Stops all streams of this service, for every contact.
#         This is useful before master shutdown.
#         
#         :param contact_infos: miville.streams.conf.ContactInfos object.
#         returns DeferredList
#         """
#         pass
#     
# class IStream(interface.Interface):
#     """
#     A stream is a transmission between this miville and an other.
#     """
#     service_name = interface.Attribute("""Name of the service this stream is of.""")
#     subject = interface.Attribute("""Subject that can notify observers.""")
#     state = interface.Attribute("""Current state. See miville.streams.__init__.STATE_*.""")
#     ports = interface.Attribute("""Ports numbers that are used by this stream.""")
#     config_entries = interface.Attribute("""Configuration entries for this stream.""")
#     subject = interface.Attribute("""Subject that can notify observers.""")
#     mode = interface.Attribute("""str Either 'send' or 'recv'. See miville.streams.MODE_*.""")
# 
#     def __init__(self, service):
#         """
#         :param service: Reference to the service who own this stream.
#         """
#         pass
# 
#     def start(self, contact_infos, config_entries, mode):
#         """
#         Starts the transmission with a contact.
#         :param contact_infos: streams.conf.ContactInfos object.
#         :param config_entries: dict
#         :param mode: str. Either 'send' or 'recv'
# 
#         returns Deferred
#         """
#         pass
#     
#     def stop(self):
#         """
#         :param contact_name: str Name of the contact to stop streaming with.
#         :param contact_addr: str
#         :param config_entries: dict
#         returns Deferred
#         """
#         pass
#     
#     def record(self, enabled=True, file_name=None):
#         """
#         Starts/stop recording
#         returns Deferred
#         """
#         pass

