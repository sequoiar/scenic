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
   * Each peer ask for ports to send to, and of media settings as well. "video": [{"port":10000, "codec":"mpeg4", "bitrate":3000000}]
 * {"msg":"ACCEPT", "videoport":10000, "audioport":11000, "sid":0}
 * {"msg":"REFUSE", "sid":0}
 * {"msg":"CANCEL", "sid":0}
 * {"msg":"ACK", "sid":0}
 * {"msg":"BYE", "sid":0}
 * {"msg":"OK", "sid":0}

Former Notes
------------
 * bug pour setter le bouton par defaut quand on change de tab. Il faut que le tab est le focus pour que ca marche. Pourtant le "print" apparait ???
"""
### CONSTANTS ###
from scenic import version
__version__ = version.__version__
APP_NAME = "scenic"

### MODULES IMPORTS  ###

import sys
import os
import smtplib
import gtk.glade
import webbrowser
import gettext

from twisted.internet import reactor

from scenic import process # just for constants
from scenic import dialogs
from scenic import data

PACKAGE_DATA = os.path.dirname(data.__file__)

### MULTILINGUAL SUPPORT ###
_ = gettext.gettext
gettext.bindtextdomain(APP_NAME, os.path.join(PACKAGE_DATA, "locale"))
gettext.textdomain(APP_NAME)
gtk.glade.bindtextdomain(APP_NAME, os.path.join(PACKAGE_DATA, "locale"))
gtk.glade.textdomain(APP_NAME)

def _get_combobox_value(widget):
    """
    Returns the current value of a GTK ComboBox widget.
    """
    index = widget.get_active()
    tree_model = widget.get_model()
    tree_model_row = tree_model[index]
    return tree_model_row[0] 

def _set_combobox_value(widget, value=None):
    """
    Sets the current value of a GTK ComboBox widget.
    """
    index = None
    tree_model = widget.get_model()
    index = 0
    for i in iter(tree_model):
        v = i[0]
        if v == value:
            break # got it
        index += 1
    if index is None:
        widget.set_active(-1)
    else:
        widget.set_active(index)

# GUI value to milhouse value mapping:
VIDEO_CODECS = {
    "h.264": "h264",
    "h.263": "h263",
    "Theora": "theora",
    "MPEG4": "mpeg4"
    }

def format_contact_markup(contact):
    """
    Formats a contact for the Adressbook GTK widget.
    @param contact: A dict with keys "name", "address" and "port"
    @rettype: str
    @return: Pango markup for the TreeView widget.
    """
    return "<b>%s</b>\n  IP: %s\n  Port: %s" % (contact["name"], contact["address"], contact["port"])

ABOUT_LABEL = """<b><big>Scenic</big></b>
Version: %s
Copyright: SAT
Authors: Etienne Desautels, Alexandre Quessy, Tristan Matthews, Simon Piette""" % (__version__)

ABOUT_TEXT_VIEW = """
Scenic is the advanced user graphical interface for the Milhouse audio/video streamer for GNU/Linux. 

Each peer decides what to receive from the other peer. Next, one peer can invite an other one to stream high-quality audio and video.
"""

class Gui(object):
    """
    Main application (arguably God) class
     * Contains the main GTK window
    """
    def __init__(self, app, kiosk_mode=False, fullscreen=False):
        # --------------------------------------
        # TODO: move that stuff to the Application class
        self.app = app
        self.load_gtk_theme(self.app.config.theme)
        self.kiosk_mode_on = kiosk_mode
        # ---------------------------------------
        
        self._offerer_invite_timeout = None
        # Set the Glade file
        glade_file = os.path.join(PACKAGE_DATA, 'scenic.glade')
        if os.path.isfile(glade_file):
            glade_path = glade_file
        else:
            text = _("<b><big>Could not find the Glade file?</big></b>\n\n" \
                    "Be sure the file %s exists. Quitting.") % glade_file
            print(text)
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
        self.main_window.connect('delete-event', self.on_main_window_deleted)
        self.main_window.set_icon_from_file(os.path.join(PACKAGE_DATA, 'scenic.png'))
        self.main_tabs_widget = self.widgets.get_widget("mainTabs")
        self.system_tab_contents_widget = self.widgets.get_widget("system_tab_contents")
        self.main_window.connect("window-state-event", self.on_window_state_event)
        # confirm_dialog:
        self.confirm_dialog = self.widgets.get_widget("confirm_dialog")
        self.confirm_dialog.connect('delete-event', self.confirm_dialog.hide_on_delete)
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
        # position of currently selected contact in list of contact:
        self.selected_contact_row = None
        self.select_contact_index = None
        # video tab drop-down menus
        self.video_size_widget = self.widgets.get_widget("video_size")
        self.video_display_widget = self.widgets.get_widget("video_display")
        self.video_bitrate_widget = self.widgets.get_widget("video_bitrate")
        self.video_source_widget = self.widgets.get_widget("video_source")
        self.video_codec_widget = self.widgets.get_widget("video_codec")
        self.video_view_preview_widget = self.widgets.get_widget("video_view_preview")
        # about tab contents:
        self.about_label_widget = self.widgets.get_widget("about_label")
        self.about_text_view_widget = self.widgets.get_widget("about_text_view")
            
        # switch to Kiosk mode if asked
        if self.kiosk_mode_on:
            self.main_window.set_decorated(False)
        else:
            # Removes the sytem_tab 
            tab_num = self.main_tabs_widget.page_num(self.system_tab_contents_widget)
            print "Removing tab #", tab_num
            self.main_tabs_widget.remove_page(tab_num)
        
        self.is_fullscreen = False
        if fullscreen:
            self.toggle_fullscreen()
        
        # Build the contact list view
        self.selection = self.contact_list_widget.get_selection()
        self.selection.connect("changed", self.on_contact_list_changed, None) 
        self.contact_tree = gtk.ListStore(str)
        self.contact_list_widget.set_model(self.contact_tree)
        column = gtk.TreeViewColumn(_("Contacts"), gtk.CellRendererText(), markup=0)
        self.contact_list_widget.append_column(column)
        # set value of widgets.
        # TODO: get rid of those methods
        self._init_widgets_value() # XXX

        self.main_window.show()
   
    # ------------------ window events and actions --------------------

    def load_gtk_theme(self, name="Darklooks"):
        file_name = os.path.join(PACKAGE_DATA, "themes/%s/gtk-2.0/gtkrc" % (name))
        #if file_name is None:
        #    file_name = os.path.join(PACKAGE_DATA, "themes/Darklooks/gtk-2.0/gtkrc")
        # FIXME: not able to reload themes dynamically.
        if os.path.exists(file_name):
            #os.environ["GTK2_RC_FILES"] = file_name
            gtk.rc_parse(file_name)
            #gtk.rc_reset_styles(gtk.settings_get_default())
            #print "loading theme", file_name
            #gtk.rc_parse(file_name)
            #gtk.rc_reparse_all()
        else:
            print("File name not found: %s" % (file_name))
     
    def toggle_fullscreen(self):
        """
        Toggles the fullscreen mode on/off.
        """
        if self.is_fullscreen:
            self.main_window.unfullscreen()
        else:
            self.main_window.fullscreen()
    
    def on_window_state_event(self, widget, event):
        """
        Called when toggled fullscreen.
        """
        self.is_fullscreen = event.new_window_state & gtk.gdk.WINDOW_STATE_FULLSCREEN != 0
        print('fullscreen %s' % (self.is_fullscreen))
        return True
    
    def on_main_window_deleted(self, *args):
        """
        Destroy method causes appliaction to exit
        when main window closed
        """
        return self._confirm_and_quit()
        
    def _confirm_and_quit(self):
        def _cb(result):
            if result:
                print("Destroying the window.")
                self.main_window.destroy()
            else:
                print("Not quitting.")
        # If you return FALSE in the "delete_event" signal handler,
        # GTK will emit the "destroy" signal. Returning TRUE means
        # you don't want the window to be destroyed.
        # This is useful for popping up 'are you sure you want to quit?'
        # type dialogs. 
        if self.app.config.confirm_quit:
            d = dialogs.YesNoDialog.create("Really quit ?\nAll streaming processes will quit as well.\nMake sure to save your settings if desired.", parent=self.main_window)
            d.addCallback(_cb)
            return True
        else:
            _cb(True)
            return False
    
    def on_main_window_destroyed(self, *args):
        # TODO: confirm dialog!
        if reactor.running:
            print("reactor.stop()")
            reactor.stop()

    # --------------- slots for some widget events ------------

    def on_video_view_preview_toggled(self, widget):
        """
        Shows a preview of the video input.
        """
        #TODO: create a new process protocol for the preview window
        #TODO: stop it when starting to stream, if running
        #TODO: stop it when button is toggled to false.
        # It can be the user that pushed the button, or it can be toggled by the software.
        print 'video_view_preview toggled', widget.get_active()
        if widget.get_active():
            command = "milhouse --videosource v4l2src --videodevice %s --localvideo --window-title preview" % (self.app.config.video_device)
            print "spawning", command
            process.run_once(*command.split())
            dialogs.ErrorDialog.create("You must manually close the preview window.", parent=self.main_window)
        else:
            print "stopping preview"

    def on_main_tabs_switch_page(self, widget, notebook_page, page_number):
        tab = widget.get_nth_page(page_number)
        if tab == "localPan":
            self.widgets.get_widget("network_admin").grab_default() # FIXME
        elif tab == "contactPan":
            self.widgets.get_widget("contactJoinBut").grab_default() # FIXME

    def on_contact_list_changed(self, *args):
        tree_list, self.selected_contact_row = args[0].get_selected()
        if self.selected_contact_row:
            self.edit_contact_widget.set_sensitive(True)
            self.remove_contact_widget.set_sensitive(True)
            self.invite_contact_widget.set_sensitive(True)
            self.selected_contact_index = tree_list.get_path(self.selected_contact_row)[0]
            self.app.address_book.selected_contact = self.app.address_book.contact_list[self.selected_contact_index]
            self.app.address_book.selected_index = self.selected_contact_index
        else:
            self.edit_contact_widget.set_sensitive(False)
            self.remove_contact_widget.set_sensitive(False)
            self.invite_contact_widget.set_sensitive(False)
            self.app.address_book.selected_contact = None

    # ---------------------- slots for addressbook widgets events --------
    
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
        self.app.address_book.current_contact_is_new = True
        # Update the text in the edit/new contact dialog:
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
                del self.app.address_book.contact_list[self.selected_contact_index]
                self.contact_tree.remove(self.selected_contact_row)
                num = self.selected_contact_index - 1
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
        self.contact_name_widget.set_text(self.app.address_book.selected_contact["name"])
        self.contact_addr_widget.set_text(self.app.address_book.selected_contact["address"])
        self.contact_port_widget.set_text(str(self.app.address_book.selected_contact["port"]))
        self.edit_contact_window.show() # addr

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
        def when_valid_save():
            """ Saves contact info after it's been validated and then closes the window"""
            contact = {
                "name": self.contact_name_widget.get_text(),
                "address": addr, 
                "port": int(port)
                }
            contact_markup = format_contact_markup(contact)
            if self.app.address_book.current_contact_is_new:
                self.contact_tree.append([contact_markup]) # add it to the tree list
                self.app.address_book.contact_list.append([contact_markup]) # and the internal address book
                self.selection.select_path(len(self.app.address_book.contact_list) - 1) # select it ...?
                self.app.address_book.selected_contact = self.app.address_book.contact_list[len(self.app.address_book.contact_list) - 1] #FIXME: we should not copy a dict like that
                self.app.address_book.current_contact_is_new = False # FIXME: what does that mean?
            else:
                self.contact_tree.set_value(self.selected_contact_row, 0, contact_markup)
            self.app.address_book.selected_contact = contact
            self.edit_contact_window.hide()

        # Validate the port number
        port = self.contact_port_widget.get_text()
        if port == "":
            port = str(self.app.config.negotiation_port) # set port to default
        elif not port.isdigit():
            text = _("The port number must be an integer.")
            self.show_error_dialog(text)
            return
        elif int(port) not in range(10000, 65535):
            text = _("The port number must be in the range of 10000-65535")
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

    # ---------------------------- Custom system tab buttons ---------------

    def on_network_admin_clicked(self, *args):
        """
        Opens the network-admin Gnome applet.
        """
        process.run_once("gksudo", "network-admin")

    def on_system_shutdown_clicked(self, *args):
        """
        Shuts down the computer.
        """
        def on_confirm_result(result):
            if result:
                process.run_once("gksudo", "shutdown -h now")

        text = _("<b><big>Shutdown the computer?</big></b>\n\nAre you sure you want to shutdown the computer now?")
        self.show_confirm_dialog(text, on_confirm_result)

    def on_system_reboot_clicked(self, *args):
        """
        Reboots the computer.
        """
        def on_confirm_result(result):
            if result:
                process.run_once("gksudo", "shutdown -r now")

        text = _("<b><big>Reboot the computer?</big></b>\n\nAre you sure you want to reboot the computer now?")
        self.show_confirm_dialog(text, on_confirm_result)

    def on_maintenance_apt_update_clicked(self, *args):
        """
        Opens APT update manager.
        """
        process.run_once("gksudo", "update-manager")

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
                fromaddr = self.app.config.email_info
                toaddrs  = self.app.config.email_info
                toaddrs = toaddrs.split(', ')
                server = smtplib.SMTP(self.app.config.smtpserver)
                server.set_debuglevel(0)
                try:
                    server.sendmail(fromaddr, toaddrs, msg)
                except:
                    text = _("Could not send info.\n\nCheck your internet connection.")
                    self.show_error_dialog(text)
                server.quit()
        
        text = _("<b><big>Send the settings?</big></b>\n\nAre you sure you want to send your computer settings to the administrator of scenic?")
        self.show_confirm_dialog(text, on_confirm_result)

        
    # --------------------- configuration and widgets value ------------

    def _gather_configuration(self):
        """
        Updates the configuration with the value of each widget.
        """
        print("gathering configuration")
        # VIDEO SIZE
        video_size = _get_combobox_value(self.video_size_widget)
        print ' * video_size:', video_size
        self.app.config.video_width = int(video_size.split("x")[0])
        self.app.config.video_height = int(video_size.split("x")[1])
        
        # DISPLAY
        video_display = _get_combobox_value(self.video_display_widget)
        print ' * video_display:', video_display
        self.app.config.video_display = video_display
        
        # BITRATE
        video_bitrate = _get_combobox_value(self.video_bitrate_widget)
        print ' * video_bitrate:', video_bitrate
        self.app.config.video_bitrate = int(video_bitrate.split(" ")[0]) * 1000000
        
        # VIDEO SOURCE AND DEVICE
        video_source = _get_combobox_value(self.video_source_widget)
        if video_source == "Color bars":
            self.app.config.video_source = "videotestsrc"
        elif video_source.startswith("/dev/video"): # TODO: firewire!
            self.app.config.video_device = video_source
            self.app.config.video_source = "v4l2src"
        print ' * videosource:', video_source
        
        # CODEC
        video_codec = _get_combobox_value(self.video_codec_widget)
        self.app.config.video_codec = VIDEO_CODECS[video_codec]
        print ' * video_codec:', video_codec
        
        #TODO: get toggle fullscreen (milhouse) value

    def _init_widgets_value(self):
        """
        Called once at startup.
         * Once the config file is read, 
         * Sets the value of each widget according to the data stored in the configuration file.
        """
        print("Changing widgets value according to configuration.")
        # VIDEO SIZE
        video_size = "%sx%s" % (self.app.config.video_width, self.app.config.video_height)
        _set_combobox_value(self.video_size_widget, video_size)
        print ' * video_size:', video_size
        
        # DISPLAY
        video_display = self.app.config.video_display
        _set_combobox_value(self.video_display_widget, video_display)
        print ' * video_display:', video_display
        
        # BITRATE
        video_bitrate = "%s Mbps" % (int(self.app.config.video_bitrate) / 1000000)
        _set_combobox_value(self.video_bitrate_widget, video_bitrate)
        print ' * video_bitrate:', video_bitrate
        
        # VIDEO SOURCE AND DEVICE
        if self.app.config.video_source == "videotestsrc":
            video_source = "Color bars"
        elif self.app.config.video_source == "v4l2src":
            video_source = self.app.config.video_device
        _set_combobox_value(self.video_source_widget, video_source)
        print ' * videosource:', video_source

        # CODEC
        # gets key for a value
        video_codec = VIDEO_CODECS.keys()[VIDEO_CODECS.values().index(self.app.config.video_codec)]
        _set_combobox_value(self.video_codec_widget, video_codec)
        print ' * video_codec:', video_codec

        # ADDRESSBOOK
        # Init addressbook contact list:
        self.app.address_book.selected_contact = None
        self.app.address_book.current_contact_is_new = False
        if len(self.app.address_book.contact_list) > 0:
            for contact in self.app.address_book.contact_list:
                contact_markup = format_contact_markup(contact)
                self.contact_tree.append([contact_markup])
            self.selection.select_path(self.app.address_book.selected)
        else:
            self.edit_contact_widget.set_sensitive(False)
            self.remove_contact_widget.set_sensitive(False)
            self.invite_contact_widget.set_sensitive(False)

        # ABOUT TAB CONTENTS:
        self.about_label_widget.set_markup(ABOUT_LABEL)
        about_text_buffer = gtk.TextBuffer()
        about_text_buffer.set_text(ABOUT_TEXT_VIEW)
        self.about_text_view_widget.set_buffer(about_text_buffer)

    # -------------------------- menu items -----------------
    
    def on_quit_menu_item_activated(self, menu_item):
        """
        Quits the application.
        """
        print menu_item, "chosen"
        self._confirm_and_quit()
    
    def on_help_menu_item_activated(self, menu_item):
        """
        Opens a web browser to the scenic web site.
        """
        print menu_item, "chosen"
        url = "http://scenic.sat.qc.ca"
        webbrowser.open(url)

    def on_save_menu_item_activated(self, menu_item):
        """
        Saves the addressbook and settings.
        """
        print menu_item, "chosen"
        print("-- Saving addressbook and configuration. -- ")
        self.save_configuration()

    # ---------------------- invitation dialogs -------------------

    def on_invite_contact_clicked(self, *args):
        """
        Sends an INVITE to the remote peer.
        """
        self.app.send_invite()
    
    def on_invite_contact_cancelled(self, *args):
        """
        Sends a CANCEL to the remote peer when invite contact window is closed.
        """
        # unschedule this timeout as we don't care if our peer answered or not
        self._unschedule_offerer_invite_timeout()
        self.app.send_cancel_and_disconnect()
        # don't let the delete-event propagate
        if self.calling_dialog.get_property('visible'):
            self.calling_dialog.hide()
        return True

    def show_error_dialog(self, text, callback=None):
        """
        Shows an error dialog, the old way.
        """
        # TODO: deprecate
        def _response_cb(widget, response_id, callback):
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
        """
        Shows a confirm dialog, the old way.
        """
        # TODO: deprecate
        def _response_cb(widget, response_id, callback):
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
                callback(response_id)
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

    def _unschedule_offerer_invite_timeout(self):
        """ Unschedules our offer invite timeout function """
        if self._offerer_invite_timeout is not None and self._offerer_invite_timeout.active():
            self._offerer_invite_timeout.cancel()
            self._offerer_invite_timeout = None
    
    def _schedule_offerer_invite_timeout(self):
        """ Schedules our offer invite timeout function """
        def _cl_offerer_invite_timed_out(self):
            # XXX
            # in case of invite timeout, act as if we'd cancelled the invite ourselves
            self.on_invite_contact_cancelled()
            if self.calling_dialog.get_property('visible'):
                self.hide_calling_dialog("answTimeout")
            # here we return false so that this callback is unregistered
            return False

        if self._offerer_invite_timeout is None or not self._offerer_invite_timeout.active():
            self._offerer_invite_timeout = reactor.callLater(5, _cl_offerer_invite_timed_out)
        else:
            print("Warning: Already scheduled a timeout as we're already inviting a contact")
