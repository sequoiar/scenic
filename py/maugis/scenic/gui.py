#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Scenic
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Scenic is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Scenic. If not, see <http://www.gnu.org/licenses/>.

"""
Scenic GTK GUI.

Negotiation is done as follow:
------------------------------
 * {"msg":"INVITE", "videoport":10000, "audioport":11000, "sid":0, "please_send_to_port":999}
 * {"msg":"ACCEPT", "videoport":10000, "audioport":11000, "sid":0}
 * {"msg":"REFUSE", "sid":0}
 * {"msg":"ACK", "sid":0}
 * {"msg":"BYE", "sid":0}
 * {"msg":"OK", "sid":0}

Former Notes
------------
 * bug pour setter le bouton par defaut quand on change de tab. Il faut que le tab est le focus pour que ca marche. Pourtant le "print" apparait ???
"""
### CONSTANTS ###
__version__ = "0.1.0"
APP_NAME = "scenic"

### MODULES IMPORTS  ###

import sys
import os
import smtplib
import gtk
import gtk.glade
import gobject
import gettext

from twisted.internet import defer
from twisted.internet import error
from twisted.internet import reactor

from scenic import communication
from scenic import saving
from scenic import process # just for constants
from scenic.streamer import StreamerManager
from scenic import dialogs
from scenic import ports
from scenic import data
PACKAGE_DATA = os.path.dirname(data.__file__)

### MULTILINGUAL SUPPORT ###
_ = gettext.gettext
gettext.bindtextdomain(APP_NAME, os.path.join(PACKAGE_DATA, "locale"))
gettext.textdomain(APP_NAME)
gtk.glade.bindtextdomain(APP_NAME, os.path.join(PACKAGE_DATA, "locale"))
gtk.glade.textdomain(APP_NAME)

class Config(saving.ConfigStateSaving):
    """
    Class attributes are default.
    """
    # Default values
    negotiation_port = 17446 # sending/receiving TCP messages on it.
    smtpserver = "smtp.sat.qc.ca"
    emailinfo = "scenic@sat.qc.ca"
    audio_input = "jackaudiosrc"
    audio_output = "jackaudiosink"
    audio_codec = "raw"
    audio_channels = 8
    video_input = "v4l2src"
    video_device = "/dev/video0"
    video_output = "xvimagesink"
    video_codec = "mpeg4"
    video_bitrate = "3000000"
    send_video_port = 8000
    recv_video_port = 8000
    send_audio_port = send_video_port + 10
    recv_audio_port = recv_video_port + 10
    bandwidth = 30

    def __init__(self):
        config_file = 'scenic.cfg'
        if os.path.isfile('/etc/' + config_file):
            config_dir = '/etc'
        else:
            config_dir = os.environ['HOME'] + '/.scenic'
        config_file_path = os.path.join(config_dir, config_file)
        saving.ConfigStateSaving.__init__(self, config_file_path)

class Application(object):
    """
    Main application (arguably God) class
     * Contains the main GTK window
    """
    def __init__(self, kiosk_mode=False):
        self.config = Config()
        self.address_book = saving.AddressBook()
        self.streamer_manager = StreamerManager(self)
        self._has_session = False
        self.streamer_manager.state_changed_signal.connect(self.on_streamer_state_changed)
        print "Starting SIC server on port %s" % (self.config.negotiation_port)
        self.server = communication.Server(self, self.config.negotiation_port)
        self.client = None
        self.got_bye = False

        # Set the Glade file
        glade_file = os.path.join(PACKAGE_DATA, 'scenic.glade')
        if os.path.isfile(glade_file):
            glade_path = glade_file
        else:
            text = _("<b><big>Could not find the Glade file?</big></b>\n\n" \
                    "Be sure the file %s exists. Quitting.") % glade_file
            print text
            sys.exit()
        self.widgets = gtk.glade.XML(glade_path, domain=APP_NAME)
        
        # connects callbacks to widgets automatically
        cb = {}
        for n in dir(self.__class__):
            if n[0] != '_' and hasattr(self, n):
                cb[n] = getattr(self, n)
        self.widgets.signal_autoconnect(cb)

        # Get all the widgets that we use
        self.main_window = self.widgets.get_widget("main_window")
        self.main_window.set_icon_from_file(os.path.join(PACKAGE_DATA, 'scenic.png'))
        # confirm_dialog:
        self.confirm_dialog = self.widgets.get_widget("confirm_dialog")
        self.confirm_dialog.connect('delete-event', self.confirm_dialog.hide_on_delete)
        self.confirm_dialog.set_transient_for(self.main_window)
        self.confirm_dialog.set_transient_for(self.main_window)
        self.confirm_label = self.widgets.get_widget("confirm_label")
        # calling_dialog:
        self.calling_dialog = self.widgets.get_widget("calling_dialog")
        self.calling_dialog.connect('delete-event', self.on_invite_contact_cancelled)
        # error_dialog:
        self.error_dialog = self.widgets.get_widget("error_dialog")
        self.error_dialog.connect('delete-event', self.error_dialog.hide_on_delete)
        self.error_dialog.set_transient_for(self.main_window)
        # Could not connect:
        self.error_label_widget = self.widgets.get_widget("error_dialog_label")
        # invited_dialog:
        self.invited_dialog = self.widgets.get_widget("invited_dialog")
        self.invited_dialog.set_transient_for(self.main_window)
        self.invited_dialog.connect('delete-event', self.invited_dialog.hide_on_delete)
        self.invited_dialog_label_widget = self.widgets.get_widget("invited_dialog_label")
        # edit_contact_window:
        self.edit_contact_window = self.widgets.get_widget("edit_contact_window")
        self.edit_contact_window.set_transient_for(self.main_window) # child of main window
        self.edit_contact_window.connect('delete-event', self.edit_contact_window.hide_on_delete)
        self.contact_name_widget = self.widgets.get_widget("contact_name")
        self.contact_addr_widget = self.widgets.get_widget("contact_addr")
        self.contact_port_widget = self.widgets.get_widget("contact_port")
        # address book buttons and list:
        self.edit_contact_widget = self.widgets.get_widget("edit_contact")
        self.remove_contact_widget = self.widgets.get_widget("remove_contact")
        self.invite_contact_widget = self.widgets.get_widget("invite_contact")
        self.contact_list_widget = self.widgets.get_widget("contact_list")
        # local settings:
        self.negotiation_port_widget = self.widgets.get_widget("negotiation_port")
        self.conf_bandwidth_widget = self.widgets.get_widget("conf_bandwidth")
        # position of currently selected contact in list of contact:
        self.selected_contact_row = None
        self.select_contact_num = None
            
        # corresponds to the answer_invite_timeout function
        self._answer_invite_timed_out_watch = None

        # adjust the bandwidth combobox iniline with the config 
        self.init_bandwidth()
        
        # switch to Kiosk mode if asked
        if kiosk_mode:
            self.main_window.set_decorated(False)
            self.widgets.get_widget("sysBox").show()
        
        # Build the contact list view
        self.selection = self.contact_list_widget.get_selection()
        self.selection.connect("changed", self.on_contact_list_changed, None) 
        self.contact_tree = gtk.ListStore(str)
        self.contact_list_widget.set_model(self.contact_tree)
        column = gtk.TreeViewColumn(_("Contacts"), gtk.CellRendererText(), markup=0)
        self.contact_list_widget.append_column(column)
        self.init_ad_book_contact_list()
        self.init_negotiation_port()

        self.main_window.show()
        self.ports_allocator = ports.PortsAllocator()
        try:
            self.server.start_listening()
        except error.CannotListenError, e:
            print("Cannot start SIC server.")
            print str(e)
            raise
        reactor.addSystemEventTrigger("before", "shutdown", self.before_shutdown)
        
    def before_shutdown(self):
        print("The application is shutting down.")
        # TODO: stop streamers
        if self.client is not None:
            if not self.got_bye:
                self.send_bye()
                self.stop_streamers()
            self.disconnect_client()
        self.server.close()
        self.address_book.save()
        
    def on_main_window_destroyed(self, *args):
        reactor.stop()

    def on_main_tabs_switch_page(self, widget, notebook_page, page_number):
        tab = widget.get_nth_page(page_number)
        if tab == "localPan":
            self.widgets.get_widget("network_admin").grab_default()
        elif tab == "contactPan":
            self.widgets.get_widget("contactJoinBut").grab_default()

    def on_contact_list_changed(self, *args):
        tree_list, self.selected_contact_row = args[0].get_selected()
        if self.selected_contact_row:
            self.edit_contact_widget.set_sensitive(True)
            self.remove_contact_widget.set_sensitive(True)
            self.invite_contact_widget.set_sensitive(True)
            self.selected_contact_num = tree_list.get_path(self.selected_contact_row)[0]
            self.address_book.contact = self.address_book.contact_list[self.selected_contact_num]
            self.address_book.selected = self.selected_contact_num
        else:
            self.edit_contact_widget.set_sensitive(False)
            self.remove_contact_widget.set_sensitive(False)
            self.invite_contact_widget.set_sensitive(False)
            self.address_book.contact = None

    def on_contact_double_clicked(self, *args):
        """
        When a contact in the list is double-clicked, 
        shows the edit contact dialog.
        """
        self.on_edit_contact_clicked(args)

    def on_add_contact_clicked(self, *args):
        """
        Pops up a dialog to be filled with new contact infos.
        
        The add_contact buttons has been clicked.
        """
        self.address_book.new_contact = True
        self.contact_name_widget.set_text("")
        self.contact_addr_widget.set_text("")
        self.contact_port_widget.set_text("")
        self.edit_contact_window.show()

    def on_remove_contact_clicked(self, *args):
        """
        Upon confirmation, the selected contact is removed.
        """
        def on_confirm_result(result):
            if result:
                del self.address_book.contact_list[self.selected_contact_num]
                self.contact_tree.remove(self.selected_contact_row)
                self.address_book.save()
                num = self.selected_contact_num - 1
                if num < 0:
                    num = 0
                self.selection.select_path(num)
        text = _("<b><big>Delete this contact from the list?</big></b>\n\nAre you sure you want "
            "to delete this contact from the list?")
        self.show_confirm_dialog(text, on_confirm_result)

    def on_edit_contact_clicked(self, *args):
        """
        Shows the edit contact dialog.
        """
        self.contact_name_widget.set_text(self.address_book.contact["name"])
        self.contact_addr_widget.set_text(self.address_book.contact["address"])
        self.contact_port_widget.set_text(str(self.address_book.contact["port"]))
        self.edit_contact_window.show()

    def on_edit_contact_cancel_clicked(self, *args):
        """
        The cancel button in the "edit_contact" window has been clicked.
        Hides the edit_contact window.
        """
        self.edit_contact_window.hide()

    def on_edit_contact_save_clicked(self, *args):
        """
        The save button in the "edit_contact" window has been clicked.
        Hides the edit_contact window and saves the changes. (new or modified contact)
        """
        address_book = self.address_book

        def when_valid_save():
            """ Saves contact info after it's been validated and then closes the window"""
            if address_book.new_contact:
                self.contact_tree.append([
                    "<b>" + self.contact_name_widget.get_text()
                    + "</b>\n  IP: " + addr
                    + "\n  Port: " + port])
                address_book.contact_list.append({})
                self.selection.select_path(len(address_book.contact_list) - 1)
                address_book.contact = address_book.contact_list[len(address_book.contact_list) - 1]
                address_book.new_contact = False
            else:
                self.contact_tree.set_value(
                    self.selected_contact_row, 0, "<b>" + 
                    self.contact_name_widget.get_text() + 
                    "</b>\n  IP: " + addr + "\n  Port: " + port)
            address_book.contact["name"] = self.contact_name_widget.get_text()
            address_book.contact["address"] = addr
            address_book.contact["port"] = int(port)
            address_book.save()
            self.edit_contact_window.hide()

        # Validate the port number
        port = self.contact_port_widget.get_text()
        if port == "":
            port = str(self.config.negotiation_port) # set port to default
        elif (not port.isdigit()) or (int(port) not in range(10000, 65535)):
            text = _("The port number is not valid\n\nEnter a valid port number in the range of 10000-65535")
            self.show_error_dialog(text)
            return
        # Validate the address
        addr = self.contact_addr_widget.get_text()
        if len(addr) < 7:
            text = _("The address is not valid\n\nEnter a valid address\nExample: 168.123.45.32 or example.org")
            self.show_error_dialog(text)
            return
        # save it.
        when_valid_save()

    def on_network_admin_clicked(self, *args):
        """
        Opens the network-admin Gnome applet.
        """
        os.system('gksudo "network-admin"')

    def on_system_shutdown_clicked(self, *args):
        """
        Shuts down the computer.
        """
        def on_confirm_result(result):
            if result:
                os.system('gksudo "shutdown -h now"')

        text = _("<b><big>Shutdown the computer?</big></b>\n\nAre you sure you want to shutdown the computer now?")
        self.show_confirm_dialog(text, on_confirm_result)

    def on_system_reboot_clicked(self, *args):
        """
        Reboots the computer.
        """
        def on_confirm_result(result):
            if result:
                os.system('gksudo "shutdown -r now"')

        text = _("<b><big>Reboot the computer?</big></b>\n\nAre you sure you want to reboot the computer now?")
        self.show_confirm_dialog(text, on_confirm_result)

    def on_maintenance_apt_update_clicked(self, *args):
        """
        Opens APT update manager.
        """
        os.system('gksudo "update-manager"')

    def on_maintenance_send_info_clicked(self, *args):
        """
        Sends an email to SAT with this information : 
         * milhouse version
         * kernel version
         * Loaded kernel modules
        """
        def on_confirm_result(result):
            milhouse_version = "unknown"
            
            if result:
                msg = "--- milhouse_version ---\n" + milhouse_version + "\n"
                msg += "--- uname -a ---\n"
                try:
                    w, r, err = os.popen3('uname -a')
                    msg += r.read() + "\n"
                    errRead = err.read()
                    if errRead:
                        msg += errRead + "\n"
                    w.close()
                    r.close()
                    err.close()
                except:
                    msg += "Error executing 'uname -a'\n"
                msg += "--- lsmod ---\n"
                try:
                    w, r, err = os.popen3('lsmod')
                    msg += r.read()
                    errRead = err.read()
                    if errRead:
                        msg += "\n" + errRead
                    w.close()
                    r.close()
                    err.close()
                except:
                    msg += "Error executing 'lsmod'"

                fromaddr = "scenic@sat.qc.ca"
                toaddrs  = self.config.emailinfo
                toaddrs = toaddrs.split(', ')
                server = smtplib.SMTP(self.config.smtpserver)
                server.set_debuglevel(0)
                try:
                    server.sendmail(fromaddr, toaddrs, msg)
                except:
                    text = _("Could not send info.\n\nCheck your internet connection.")
                    self.show_error_dialog(text)
                server.quit()

        text = _("<b><big>Send the settings?</big></b>\n\nAre you sure you want to send your computer settings to the administrator of scenic?")
        self.show_confirm_dialog(text, on_confirm_result)

    def has_session(self):
        """
        @rettype: bool
        """
        return self._has_session

    def allocate_ports(self):
        # TODO: start_session
        self.config.recv_video_port = self.ports_allocator.allocate()
        self.config.recv_audio_port = self.ports_allocator.allocate()

    def free_ports(self):
        # TODO: stop_session
        try:
            self.ports_allocator.free(self.config.recv_video_port)
        except ports.PortsAllocatorError, e:
            print(e)
        try:    
            self.ports_allocator.free(self.config.recv_audio_port)
        except ports.PortsAllocatorError, e:
            print(e)
        
    def on_invite_contact_clicked(self, *args):
        """
        Sends an INVITE to the remote peer.
        """
        self.allocate_ports()
        if self.streamer_manager.is_busy():
            dialogs.ErrorDialog.create("Impossible to invite a contact to start streaming. A streaming session is already in progress.")
        else:
            msg = {
                "msg":"INVITE",
                "sid":0, 
                "videoport": self.config.recv_video_port,
                "audioport": self.config.recv_audio_port,
                "please_send_to_port": self.config.negotiation_port
                }
            port = self.config.negotiation_port # self.address_book.contact["port"]
            ip = self.address_book.contact["address"]

            def _on_connected(proto):
                self.client.send(msg)
                return proto
            def _on_error(reason):
                print "error trying to connect to %s:%s : %s" % (ip, port, reason)
                self.calling_dialog.hide()
                self.client = None
                return None
               
            print "sending %s to %s:%s" % (msg, ip, port) 
            self.client = communication.Client(self, port)
            deferred = self.client.connect(ip)
            deferred.addCallback(_on_connected).addErrback(_on_error)
            self.calling_dialog.show()
            # window will be hidden when we receive ACCEPT or REFUSE
    
    def on_invite_contact_cancelled(self, *args):
        """
        Sends a CANCEL to the remote peer.
        """
        msg = {
            "msg":"CANCEL",
            "sid":0
            }
        port = self.config.negotiation_port # self.address_book.contact["port"]
        ip = self.address_book.contact["address"]

        def _on_connected(proto):
            self.client.send(msg)
            self.client = None
            return proto
        def _on_error(reason):
            print "error trying to connect to %s:%s : %s" % (ip, port, reason)
            self.client = None
            return None
           
        print "SENDING %s TO %s:%s" % (msg, ip, port) 
        self.client = communication.Client(self, port)
        deferred = self.client.connect(ip)
        deferred.addCallback(_on_connected).addErrback(_on_error)
        self.calling_dialog.hide()
        return True
        # window will be hidden when we receive ACCEPT or REFUSE

    def on_conf_bandwidth_changed(self, *args):
        """
        Changes the bandwidth setting.
        Called when the bandwidth drop-down menu value has changed.
        """
        base = 30
        num = 2 # number of choice
        step = base / num
        selection = self.conf_bandwidth_widget.get_active()
        self.config.bandwidth = (selection + 1) * step
        self.config.save()

    def on_negotiation_port_changed(self, *args):
        """
        Changes the local SIC server port number.
        """
        # call later 
        gobject.timeout_add(0, self._later_check_negotiation_port, args)
        return False

    def _later_check_negotiation_port(self, *args):
        def on_error_dialog_result(result):
            self.negotiation_port_widget.grab_focus()
            return False

        port = self.negotiation_port_widget.get_text()
        if not port.isdigit():
            self.widgets.get_widget("mainTabs").set_current_page(1)
            self.init_negotiation_port()
            text = _("The port number is not valid\n\nEnter a valid port number in the range of 1-999999")
            self.show_error_dialog(text, on_error_dialog_result)
        else:
            self.config.negotiation_port = int(port)
            self.config.save()
            self.server.change_port(self.config.negotiation_port)

    def show_error_dialog(self, text, callback=None):
        def _response_cb(widget, response_id, callback):
            if response_id != gtk.RESPONSE_DELETE_EVENT:
                widget.hide()
            if callback is not None:
                callback()
            widget.disconnect(slot1)

        self.error_label_widget.set_text(text)
        dialog = self.error_dialog
        dialog.set_modal(True)
        slot1 = dialog.connect('response', _response_cb, callback)
        dialog.show()
    
    def show_confirm_dialog(self, text, callback=None):
        def _response_cb(widget, response_id, callback):
            if response_id != gtk.RESPONSE_DELETE_EVENT:
                widget.hide()
            if callback is not None:
                callback(response_id == gtk.RESPONSE_OK)
            widget.disconnect(slot1)

        self.confirm_label.set_label(text)
        dialog = self.confirm_dialog
        dialog.set_modal(True)
        slot1 = dialog.connect('response', _response_cb, callback)
        dialog.show()

    def show_invited_dialog(self, text, callback=None):
        """ 
        We disconnect and reconnect the callbacks every time
        this is called, otherwise we'd would have multiple 
        callback invokations per response since the widget 
        stays alive 
        """
        def _response_cb(widget, response_id, callback):
            widget.hide()
            if callback is not None:
                callback(response_id == gtk.RESPONSE_OK)
            widget.disconnect(slot1)

        self.invited_dialog_label_widget.set_label(text)
        dialog = self.invited_dialog
        dialog.set_modal(True)
        slot1 = dialog.connect('response', _response_cb, callback)
        dialog.show()

    def hide_calling_dialog(self, msg="", err=""):
        """
        Hides the "calling_dialog" dialog.
        Shows an error dialog if the argument msg is set to "err", "timeout", "answTimeout", "send", "refuse" or "badAnsw".
        """
        self.calling_dialog.hide()
        text = None
        if msg == "err":
            text = _("Contact unreacheable.\n\nCould not connect to the IP address of this contact.")
        elif msg == "timeout":
            text = _("Connection timeout.\n\nCould not connect to the port of this contact.")
        elif msg == "answTimeout":
            text = _("Contact answer timeout.\n\nThe contact did not answer soon enough.")
        elif msg == "send":
            text = _("Problem sending command.\n\nError: %s") % err
        elif msg == "refuse":
            text = _("Connection refused.\n\nThe contact refused the connection.")
        elif msg == "badAnsw":
            text = _("Invalid answer.\n\nThe answer was not valid.")
        if text is not None:
            self.show_error_dialog(text)

    def init_bandwidth(self):
        base = 30
        num = 2 # number of choice
        selection = int(round((self.config.bandwidth - 1) * num / base))
        if selection < 0:
            selection = 0
        elif selection > base:
            selection = base
        self.conf_bandwidth_widget.set_active(selection)

    def init_negotiation_port(self):
        self.negotiation_port_widget.set_text(str(self.config.negotiation_port))

    def init_ad_book_contact_list(self):
        address_book = self.address_book
        address_book.contact = None
        address_book.new_contact = False
        if len(address_book.contact_list) > 0:
            for contact in address_book.contact_list:
                self.contact_tree.append(    ["<b>" + contact["name"]
                                            + "</b>\n  IP: "
                                            + contact["address"] + "\n  Port: "
                                            + str(contact["port"])] )
            self.selection.select_path(address_book.selected)
        else:
            self.edit_contact_widget.set_sensitive(False)
            self.remove_contact_widget.set_sensitive(False)
            self.invite_contact_widget.set_sensitive(False)

    def _unschedule_answer_invite_timeout(self):
        """ Unschedules our answer invite timeout function """
        if self._answer_invite_timed_out_watch is not None:
            gobject.source_remove(self._answer_invite_timed_out_watch)
            self._answer_invite_timed_out_watch = None

    def on_server_rcv_command(self, message, addr, server):
        # XXX
        msg = message["msg"]
        addr = server.get_peer_ip()
        print "Got %s from %s" % (msg, addr)
        
        if msg == "INVITE":
            # FIXME: this doesn't make sense here
            self.got_bye = False
            send_to_port = message["please_send_to_port"]
            # TODO
            # if local user doesn't respond, close dialog in 5 seconds
            
            def _on_contact_request_dialog_result(result):
                """
                User is accetping or declining an offer.
                @param result: Answer to the dialog.
                """
                self._unschedule_answer_invite_timeout()
                if result:
                    if self.client is not None:
                        self.allocate_ports()
                        self.client.send({"msg":"ACCEPT", "videoport":self.config.recv_video_port, "audioport":self.config.recv_audio_port, "sid":0})
                        # TODO: Use session to contain settings and ports
                        self.config.send_video_port = message["videoport"]
                        self.config.send_audio_port = message["audioport"]
                    else:
                        print "Error: connection lost, so we could not accept." # FIXME
                else:
                    if self.client is not None:
                        self.client.send({"msg":"REFUSE", "sid":0})
                        self.client = None
                return True
            # answer REFUSE if busy
            if self.streamer_manager.is_busy():
                print("Got invitation, but we are busy.")
                #self.client.send({"msg":"REFUSE", "sid":0})
                communication.connect_send_and_disconnect(addr, send_to_port, {'msg':'REFUSE', 'sid':0}) #FIXME: where do we get the port number from?
            else:
                print "sending to %s:%s" % (addr, send_to_port)
                self.client = communication.Client(self, send_to_port)
                self.client.connect(addr)
                # user must respond in less than 5 seconds
                self._answer_invite_timed_out_watch = gobject.timeout_add(5000, self._cl_answer_invite_timed_out, addr)
                text = _("<b><big>" + addr + " is inviting you.</big></b>\n\nDo you accept the connection?")
                self.show_invited_dialog(text, _on_contact_request_dialog_result)

        elif msg == "CANCEL":
            self._unschedule_answer_invite_timeout()
            self.client = None
            self.invited_dialog.hide()
            dialogs.ErrorDialog.create("Remote peer cancelled invitation.")
            
        elif msg == "ACCEPT":
            # FIXME: this doesn't make sense here
            self.got_bye = False
            # TODO: Use session to contain settings and ports
            if self.client is not None:
                self.hide_calling_dialog("accept")
                self.config.send_video_port = message["videoport"]
                self.config.send_audio_port = message["audioport"]
                if self.streamer_manager.is_busy():
                    dialogs.ErrorDialog.create("A streaming session is already in progress.")
                else:
                    print("Got ACCEPT. Starting streamers as initiator.")
                    self.start_streamers(addr)
                    self.client.send({"msg":"ACK", "sid":0})
            else:
                print("Error ! Connection lost.") # FIXME
        elif msg == "REFUSE":
            self.free_ports()
            self.hide_calling_dialog("refuse")
        elif msg == "ACK":
            print("Got ACK. Starting streamers as answerer.")
            self.start_streamers(addr)
        elif msg == "BYE":
            self.got_bye = True
            self.stop_streamers()
            if self.client is not None:
                print 'disconnecting client and sending BYE'
                self.client.send({"msg":"OK", "sid":0})
                self.disconnect_client()
        elif msg == "OK":
            print "received ok. Everything has an end."
            if self.client is not None:
                print 'disconnecting client'
                self.disconnect_client()

    def start_streamers(self, addr):
        self._has_session = True
        self.streamer_manager.start(addr, self.config)

    def stop_streamers(self):
        self.streamer_manager.stop()

    def on_streamers_stopped(self, addr):
        """
        We call this when all streamers are stopped.
        """
        print "on_streamers_stopped got called"
        self._has_session = False
        self.free_ports()
        
    def disconnect_client(self):
        """
        Disconnects the SIC sender.
        @rettype: L{Deferred}
        """
        def _cb(result, d1):
            self.client = None
            d1.callback(True)
        def _cl(d1):
            if self.client is not None:
                d2 = self.client.disconnect()
                d2.addCallback(_cb, d1)
            else:
                d1.callback(True)
        if self.client is not None:
            d = defer.Deferred()
            reactor.callLater(0, _cl, d)
            return d
        else: 
            return defer.succeed(True)
    
    def send_cancel(self):
        """
        Sends CANCEL
        CANCEL cancels the invite sent to the remote host.
        """
        if self.client is not None:
            self.client.send({"msg":"CANCEL", "sid":0})

    def send_bye(self):
        """
        Sends BYE
        BYE stops the streaming on the remote host.
        """
        if self.client is not None:
            self.client.send({"msg":"BYE", "sid":0})

    def on_streamer_state_changed(self, streamer, new_state):
        """
        Slot for scenic.streamer.StreamerManager.state_changed_signal
        """
        if new_state in [process.STATE_STOPPED]:
            if not self.got_bye:
                """ got_bye means our peer sent us a BYE, so we shouldn't send one back """
                print("Local StreamerManager stopped. Sending BYE")
                self.send_bye()
            
    def _cl_answer_invite_timed_out(self, addr):
        """ 
        This is called if we haven't responded to our peer's invitation within a 
        reasonable delay (hardcoded to 5000 ms)
        """
        self.invited_dialog.response(gtk.RESPONSE_NONE)
        self.invited_dialog.hide()
        text = _("%s was inviting you.\n\nBut you did not answer in reasonable delay.") % addr
        self.show_error_dialog(text)
        self.client = None
        return False

    def on_client_socket_timeout(self, client):
        # XXX
        self.hide_calling_dialog("timeout")
    
    def on_client_socket_error(self, client, err, msg):
        # XXX
        self.hide_calling_dialog(msg)
        text = _("%s: %s") % (str(err), str(msg))
        self.show_error_dialog(text)

    def on_client_connecting(self, client):
        # XXX
        """
        Slot for the sending_signal of the client.
        schedules some stuff.
        """
        # call later
        self.answ_watch = gobject.timeout_add(5000, self.client_answer_timeout, client)

    def client_answer_timeout(self, client):
        # XXX
        if self.calling_dialog.get_property('visible'):
            self.hide_calling_dialog("answTimeout")
    
