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

Former Notes
------------
 * bug pour setter le bouton par defaut quand on change de tab. Il faut que le tab est le focus pour que ca marche. Pourtant le "print" apparait ???
"""
### CONSTANTS ###
from scenic import configure
__version__ = configure.VERSION
APP_NAME = configure.APPNAME
PACKAGE_DATA = configure.PKGDATADIR

### MODULES IMPORTS  ###

import sys
import os
import smtplib
import gtk.glade
import webbrowser
import gettext
from twisted.internet import reactor
from twisted.python.reflect import prefixedMethods
from scenic import process # just for constants
from scenic import dialogs
from scenic.devices import cameras

### MULTILINGUAL SUPPORT ###
_ = gettext.gettext
gettext.bindtextdomain(APP_NAME, os.path.join(PACKAGE_DATA, "locale"))
gettext.textdomain(APP_NAME)
gtk.glade.bindtextdomain(APP_NAME, os.path.join(PACKAGE_DATA, "locale"))
gtk.glade.textdomain(APP_NAME)

def _get_key_for_value(dictionnary, value):
    """
    Returns the key for a value in a dict.
    @param dictionnary: dict
    @param value: The value.
    """
    return dictionnary.keys()[dictionnary.values().index(value)]

def _get_combobox_value(widget):
    """
    Returns the current value of a GTK ComboBox widget.
    """
    index = widget.get_active()
    tree_model = widget.get_model()
    try:
        tree_model_row = tree_model[index]
    except IndexError, e:
        raise RuntimeError("ComboBox widget %s doesn't have value with index %s." % (widget, index))
    return tree_model_row[0] 

def _set_combobox_choices(widget, choices=[]):
    """
    Sets the choices in a GTK combobox.
    """
    #TODO When we change a widget value, its changed callback is called...
    previous_value = _get_combobox_value(widget)
    tree_model = gtk.ListStore(str)
    for choice in choices:
        tree_model.append([choice])
    widget.set_model(tree_model)
    _set_combobox_value(widget, previous_value)

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


#videotestsrc legible name:
VIDEO_TEST_INPUT = "Color bars"

# GUI legible value to milhouse value mapping:
VIDEO_CODECS = {
    "h.264": "h264",
    "h.263": "h263",
    "Theora": "theora",
    "MPEG4": "mpeg4"
    }
AUDIO_CODECS = {
    "Raw": "raw",
    "MP3": "mp3",
    "Vorbis": "vorbis",
    }
AUDIO_SOURCES = {
    "JACK": "jackaudiosrc",
    "Test sound": "audiotestsrc"
    }
# min/max:
VIDEO_BITRATE_MIN_MAX = {
    "h.264": [2.0, 16.0],
    "MPEG4": [0.5, 4.0],
    "h.263": [0.5, 4.0],
    }
# standards:
VIDEO_STANDARDS = ["NTSC", "PAL"]

def format_contact_markup(contact):
    """
    Formats a contact for the Adressbook GTK widget.
    @param contact: A dict with keys "name", "address" and "port"
    @rettype: str
    @return: Pango markup for the TreeView widget.
    """
    return "<b>%s</b>\n  IP: %s\n  Port: %s" % (contact["name"], contact["address"], contact["port"])

class Gui(object):
    """
    Graphical User Interface
     * Contains the main GTK window.
     * And some dialogs.
    """
    def __init__(self, app, kiosk_mode=False, fullscreen=False):
        self.app = app
        self.load_gtk_theme(self.app.config.theme)
        self.kiosk_mode_on = kiosk_mode
        self._offerer_invite_timeout = None
        # Set the Glade file
        glade_file = os.path.join(PACKAGE_DATA, 'scenic.glade')
        if os.path.isfile(glade_file):
            glade_path = glade_file
        else:
            text = _("Error : Could not find the Glade file %s. Exitting.") % (glade_file)
            print(text)
            sys.exit()
        self.widgets = gtk.glade.XML(glade_path, domain=APP_NAME)
        
        # connects callbacks to widgets automatically
        glade_signal_slots = {}
        for method in prefixedMethods(self, "on_"):
            glade_signal_slots[method.__name__] = method
        self.widgets.signal_autoconnect(glade_signal_slots)
        
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
        # video
        self.video_capture_size_widget = self.widgets.get_widget("video_capture_size")
        self.video_display_widget = self.widgets.get_widget("video_display")
        self.video_bitrate_widget = self.widgets.get_widget("video_bitrate")
        self.video_source_widget = self.widgets.get_widget("video_source")
        self.video_codec_widget = self.widgets.get_widget("video_codec")
        self.video_fullscreen_widget = self.widgets.get_widget("video_fullscreen")
        self.video_view_preview_widget = self.widgets.get_widget("video_view_preview")
        self.video_deinterlace_widget = self.widgets.get_widget("video_deinterlace")
        self.aspect_ratio_widget = self.widgets.get_widget("aspect_ratio")
        self.v4l2_input_widget = self.widgets.get_widget("v4l2_input")
        self.v4l2_standard_widget = self.widgets.get_widget("v4l2_standard")
        self.video_jitterbuffer_widget = self.widgets.get_widget("video_jitterbuffer")
        self.video_bitrate_widget = self.widgets.get_widget("video_bitrate")
        
        # audio
        self.audio_source_widget = self.widgets.get_widget("audio_source")
        self.audio_codec_widget = self.widgets.get_widget("audio_codec")
        self.audio_jack_icon_widget = self.widgets.get_widget("audio_jack_icon")
        self.audio_jack_state_widget = self.widgets.get_widget("audio_jack_state")
        self.audio_numchannels_widget = self.widgets.get_widget("audio_numchannels")

        self.jack_latency_widget = self.widgets.get_widget("jack_latency")
        self.jack_sampling_rate_widget = self.widgets.get_widget("jack_sampling_rate")
        # system tab contents:
        self.network_admin_widget = self.widgets.get_widget("network_admin")
            
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
        column = gtk.TreeViewColumn(_("Contacts"), gtk.CellRendererText(), markup=False)
        self.contact_list_widget.append_column(column)
        self._v4l2_input_changed_by_user = True # if False, the software is changing those drop-down values itself.
        self._v4l2_standard_changed_by_user = True
        self._video_source_changed_by_user = True
        self.main_window.show()
        # The main app must call init_widgets_value
   
    #TODO: for the preview in the drawing area   
    #def on_expose_event(self, widget, event):
    #    self.preview_xid = widget.window.xid
    #    return False

    # ------------------ window events and actions --------------------

    def load_gtk_theme(self, name="Darklooks"):
        file_name = os.path.join(PACKAGE_DATA, "themes/%s/gtkrc" % (name))
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
        if self.app.config.confirm_quit and not configure.IN_DEVELOPMENT_MODE:
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
            self.app.save_configuration() #gathers and saves
            command = "milhouse --videosource %s --localvideo --window-title preview" % (self.app.config.video_source)
            if self.app.config.video_source != "videotestsrc":
                command += " --videodevice %s" % (self.app.config.video_device)
            print "spawning $%s" % (command)
            process.run_once(*command.split())
            dialogs.ErrorDialog.create("You must manually close the preview window.", parent=self.main_window)
        else:
            print "should be stopping preview"

    def on_main_tabs_switch_page(self, widget, notebook_page, page_number):
        """
        Called when the user switches to a different page.
        Pages names are : 
         * contacts_tab_contents
         * video_tab_contents
         * audio_tab_contents
         * system_tab_contents
         * about_tab_contents
        """
        tab_widget = widget.get_nth_page(page_number)
        tab_name = tab_widget.get_name()
        if tab_name == "contacts_tab_contents":
            self.invite_contact_widget.grab_default()
        elif tab_name == "video_tab_contents":
            self.app.poll_x11_devices()
            self.app.poll_camera_devices()
        elif tab_name == "audio_tab_contents":
            pass
        elif tab_name == "system_tab_contents":
            self.network_admin_widget.grab_default()

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
        def _on_confirm_result(result):
            if result:
                del self.app.address_book.contact_list[self.selected_contact_index]
                self.contact_tree.remove(self.selected_contact_row)
                num = self.selected_contact_index - 1
                if num < 0:
                    num = 0
                self.selection.select_path(num)
        text = _("<b><big>Delete this contact from the list?</big></b>\n\nAre you sure you want "
            "to delete this contact from the list?")
        self.show_confirm_dialog(text, _on_confirm_result)

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
        def _when_valid_save():
            """ Saves contact info after it's been validated and then closes the window"""
            contact = {
                "name": self.contact_name_widget.get_text(),
                "address": addr, 
                "port": int(port)
                }
            contact_markup = format_contact_markup(contact)
            if self.app.address_book.current_contact_is_new:
                self.contact_tree.append([contact_markup]) # add it to the tree list
                self.app.address_book.contact_list.append(contact) # and the internal address book
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
            dialogs.ErrorDialog.create("The port number must be an integer", parent=self.main_window)
            return
        elif int(port) not in range(10000, 65535):
            dialogs.ErrorDialog.create("The port number must be in the range of 10000-65535", parent=self.main_window)
            return
        # Validate the address
        addr = self.contact_addr_widget.get_text()
        if len(addr) < 7:
            dialogs.ErrorDialog.create("The address is not valid\n\nEnter a valid address\n" +
                    "Example: 168.123.45.32 or example.org", parent=self.main_window)
            return
        # save it.
        _when_valid_save()

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
        def _on_confirm_result(result):
            if result:
                process.run_once("gksudo", "shutdown -h now")

        text = _("<b><big>Shutdown the computer?</big></b>\n\nAre you sure you want to shutdown the computer now?")
        self.show_confirm_dialog(text, _on_confirm_result)

    def on_system_reboot_clicked(self, *args):
        """
        Reboots the computer.
        """
        def _on_confirm_result(result):
            if result:
                process.run_once("gksudo", "shutdown -r now")

        text = _("<b><big>Reboot the computer?</big></b>\n\nAre you sure you want to reboot the computer now?")
        self.show_confirm_dialog(text, _on_confirm_result)

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
        def _on_confirm_result(result):
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
                    dialogs.ErrorDialog.create("Could not send info.\n\nCheck your internet connection.", parent=self.main_window)
                server.quit()
        
        text = _("<b><big>Send the settings?</big></b>\n\nAre you sure you want to send your computer settings to the administrator of scenic?")
        self.show_confirm_dialog(text, _on_confirm_result)

    # --------------------- configuration and widgets value ------------

    def _gather_configuration(self):
        """
        Updates the configuration with the value of each widget.
        """
        print("gathering configuration")
        # VIDEO SIZE:
        video_capture_size = _get_combobox_value(self.video_capture_size_widget)
        self.app.config.video_capture_size = video_capture_size
        print ' * video_capture_size:', self.app.config.video_capture_size
        # DISPLAY:
        video_display = _get_combobox_value(self.video_display_widget)
        print ' * video_display:', video_display
        self.app.config.video_display = self.app.config.video_display
        # VIDEO SOURCE AND DEVICE:
        video_source = _get_combobox_value(self.video_source_widget)
        if video_source == "Color bars":
            self.app.config.video_source = "videotestsrc"
        elif video_source.startswith("/dev/video"): # TODO: firewire!
            self.app.config.video_device = video_source
            self.app.config.video_source = "v4l2src"
        print ' * videosource:', self.app.config.video_source
        # VIDEO CODEC:
        video_codec = _get_combobox_value(self.video_codec_widget)
        self.app.config.video_codec = VIDEO_CODECS[video_codec]
        print ' * video_codec:', self.app.config.video_codec
        # VIDEO ASPECT RATIO:
        video_aspect_ratio = _get_combobox_value(self.aspect_ratio_widget)
        self.app.config.video_aspect_ratio = video_aspect_ratio
        print ' * video_aspect_ratio:', self.app.config.video_aspect_ratio
        #VIDEO FULLSCREEN
        video_fullscreen = self.video_fullscreen_widget.get_active()
        self.app.config.video_fullscreen = video_fullscreen
        print ' * video_fullscreen:', self.app.config.video_fullscreen
        #VIDEO DEINTERLACE
        video_deinterlace = self.video_deinterlace_widget.get_active()
        self.app.config.video_deinterlace = video_deinterlace
        print ' * video_deinterlace:', self.app.config.video_deinterlace
        # VIDEO JITTERBUFFER
        video_jitterbuffer = self.video_jitterbuffer_widget.get_value_as_int() # spinbutton
        self.app.config.video_jitterbuffer = video_jitterbuffer
        print ' * video_jitterbuffer:', self.app.config.video_jitterbuffer
        # VIDEO BITRATE
        video_bitrate = self.video_bitrate_widget.get_value() # spinbutton (float)
        self.app.config.video_bitrate = video_bitrate
        print ' * video_bitrate:', self.app.config.video_bitrate
        
        # AUDIO:
        audio_source_readable = _get_combobox_value(self.audio_source_widget)
        audio_codec_readable = _get_combobox_value(self.audio_codec_widget)
        audio_numchannels = self.audio_numchannels_widget.get_value_as_int() # spinbutton
        print " * audio_source:", audio_source_readable
        print " * audio_codec:", audio_codec_readable
        print " * audio_numchannels:", audio_numchannels
        self.app.config.audio_source = AUDIO_SOURCES[audio_source_readable]
        self.app.config.audio_codec = AUDIO_CODECS[audio_codec_readable]
        # FIXME: the interface should already prevent this case from happening
        if audio_numchannels > 2 and self.app.config.audio_codec == "mp3":
            dialogs.ErrorDialog.create("Will receive 2 channels, since the MP3 codec allows a maximum of 2 channels.")
            audio_numchannels = 2
        self.app.config.audio_channels = audio_numchannels

    def update_widgets_with_saved_config(self):
        """
        Called once at startup.
         * Once the config file is read, and the devices have been polled
         * Sets the value of each widget according to the data stored in the configuration file.
        It could be called again, once another config file has been read.
        """
        print("Changing widgets value according to configuration.")
        # VIDEO CAPTURE SIZE:
        video_capture_size = self.app.config.video_capture_size
        _set_combobox_value(self.video_capture_size_widget, video_capture_size)
        print ' * video_capture_size:', video_capture_size
        # DISPLAY:
        video_display = self.app.config.video_display
        _set_combobox_value(self.video_display_widget, video_display)
        print ' * video_display:', video_display
        # VIDEO SOURCE AND DEVICE:
        if self.app.config.video_source == "videotestsrc":
            video_source = "Color bars"
        elif self.app.config.video_source == "v4l2src":
            video_source = self.app.config.video_device
        _set_combobox_value(self.video_source_widget, video_source)
        print ' * videosource:', video_source
        # VIDEO CODEC:
        video_codec = _get_key_for_value(VIDEO_CODECS, self.app.config.video_codec)
        _set_combobox_value(self.video_codec_widget, video_codec)
        print ' * video_codec:', video_codec
        # VIDEO ASPECT RATIO:
        video_aspect_ratio = self.app.config.video_aspect_ratio
        _set_combobox_value(self.aspect_ratio_widget, video_aspect_ratio)
        print ' * video_aspect_ratio:', video_aspect_ratio
        # VIDEO FULLSCREEN:
        video_fullscreen = self.app.config.video_fullscreen
        self.video_fullscreen_widget.set_active(video_fullscreen)
        print ' * video_fullscreen:', video_fullscreen
        # VIDEO DEINTERLACE:
        video_deinterlace = self.app.config.video_deinterlace
        self.video_deinterlace_widget.set_active(video_deinterlace)
        print ' * video_deinterlace:', video_deinterlace
        # VIDEO JITTERBUFFER
        video_jitterbuffer = self.app.config.video_jitterbuffer
        self.video_jitterbuffer_widget.set_value(video_jitterbuffer) # spinbutton
        print ' * video_jitterbuffer:', video_jitterbuffer
        # VIDEO BITRATE
        video_bitrate = self.app.config.video_bitrate
        self.video_bitrate_widget.set_value(video_bitrate) # spinbutton
        print ' * video_bitrate:', video_bitrate
        
        # ADDRESSBOOK:
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
        # AUDIO:
        audio_source_readable = _get_key_for_value(AUDIO_SOURCES, self.app.config.audio_source)
        audio_codec = _get_key_for_value(AUDIO_CODECS, self.app.config.audio_codec)
        audio_numchannels = self.app.config.audio_channels
        print " * audio_source:", audio_source_readable
        print " * audio_codec:", audio_codec
        print " * audio_numchannels:", audio_numchannels
        self.audio_numchannels_widget.set_value(audio_numchannels) # spinbutton
        _set_combobox_value(self.audio_source_widget, audio_source_readable)
        _set_combobox_value(self.audio_codec_widget, audio_codec)

    def on_audio_codec_changed(self, widget):
        """
        Called when the user selects a different audio codec, updates
        the range of the numchannels box.
        """
        old_numchannels = self.audio_numchannels_widget.get_value()
        max_channels = None
        if _get_combobox_value(self.audio_codec_widget) == "MP3":
            max_channels = 2
        elif _get_combobox_value(self.audio_codec_widget) == "Raw":
            max_channels = 8
        elif _get_combobox_value(self.audio_codec_widget) == "Vorbis":
            max_channels = 24 
        # update range and clamp numchannels to new range 
        self.audio_numchannels_widget.set_range(1, max_channels)
        self.audio_numchannels_widget.set_value(min(old_numchannels, max_channels)) 

    def on_video_codec_changed(self, widget):
        old_bitrate = self.video_bitrate_widget.get_value()
        codec = _get_combobox_value(self.video_codec_widget)
        if codec in VIDEO_BITRATE_MIN_MAX.keys():
            self.video_bitrate_widget.set_sensitive(True)
            mini = VIDEO_BITRATE_MIN_MAX[codec][0]
            maxi = VIDEO_BITRATE_MIN_MAX[codec][1]
            self.video_bitrate_widget.set_range(mini, maxi)
            self.video_bitrate_widget.set_value(min(maxi, max(old_bitrate, mini)))
        else:
            self.video_bitrate_widget.set_sensitive(False)
        
    def update_x11_devices(self):
        """
        Called once Application.poll_x11_devices has been run
        """
        x11_displays = [display["name"] for display in self.app.devices["x11_displays"]]
        print("Updating X11 displays with values %s" % (x11_displays))
        _set_combobox_choices(self.video_display_widget, x11_displays)

    def update_camera_devices(self):
        """
        Called once Application.poll_camera_devices has been run
        """
        self._video_source_changed_by_user = False
        cameras = self.app.devices["cameras"].keys()
        cameras.insert(0, VIDEO_TEST_INPUT)
        print("Updating video sources with values %s" % (cameras))
        _set_combobox_choices(self.video_source_widget, cameras)
        self.update_v4l2_inputs_size_and_norm()
        self._video_source_changed_by_user = True

    def update_v4l2_inputs_size_and_norm(self):
        """
        Called when : 
         * user chooses a different video source.
        If the selected is not a V4L2, disables the input and norm widgets.
        """
        value = _get_combobox_value(self.video_source_widget)
        self._v4l2_input_changed_by_user = False
        self._v4l2_standard_changed_by_user = False
        # change choices and value:
        if value == VIDEO_TEST_INPUT:
            # INPUTS:
            self.v4l2_input_widget.set_sensitive(False)
            self.v4l2_input_widget.set_active(-1)
            # STANDARD:
            self.v4l2_standard_widget.set_sensitive(False)
            self.v4l2_standard_widget.set_active(-1)
            # SIZE:
            _set_combobox_choices(self.video_capture_size_widget, ["320x240", "640x480", "720x480"]) # TODO: more test sizes
        else:
            # INPUTS:
            current_camera_name = _get_combobox_value(self.video_source_widget)
            cam = self.app.devices["cameras"][current_camera_name]
            current_input = cam["input"]
            if current_input is not None: # check if device has many inputs
                self.v4l2_input_widget.set_sensitive(True)
                _set_combobox_choices(self.v4l2_input_widget, cam["inputs"])
                _set_combobox_value(self.v4l2_input_widget, cam["inputs"][current_input]) # which in turn calls on_v4l2_input_changed
            else:
                self.v4l2_input_widget.set_sensitive(False)
                self.v4l2_input_widget.set_active(-1)
                
            # STANDARD: 
            current_standard = cam["standard"]
            if current_standard is not None: # check if device supports different standards
                self.v4l2_standard_widget.set_sensitive(True)
                _set_combobox_choices(self.v4l2_standard_widget, VIDEO_STANDARDS)
                _set_combobox_value(self.v4l2_standard_widget, cam["standard"]) # which in turn calls on_v4l2_standard_changed
            else:
                self.v4l2_standard_widget.set_sensitive(False)
                self.v4l2_standard_widget.set_active(-1)
            #self.v4l2_standard_widget.set_sensitive(True)
            # SIZE:
            print "supported sizes: ", cam["supported_sizes"]
            _set_combobox_choices(self.video_capture_size_widget, cam["supported_sizes"]) # TODO: more test sizes
        # once done:
        self._v4l2_input_changed_by_user = True
        self._v4l2_standard_changed_by_user = True
            
    def on_video_source_changed(self, widget):
        """
        Called when the user changes the video source.
         * updates the input
        """
        if self._video_source_changed_by_user:
            current_camera_name = _get_combobox_value(self.video_source_widget)
            if current_camera_name != VIDEO_TEST_INPUT:
                self.app.poll_camera_devices()
            self.update_v4l2_inputs_size_and_norm()

    def on_v4l2_standard_changed(self, widget):
        """
        When the user changes the V4L2 standard, we actually change this standard using milhouse.
        Calls `milhouse --videodevice /dev/videoX --v4l2-standard XXX
        Values are either NTSC or PAL.
        """
        if self._v4l2_standard_changed_by_user:
            # change standard for device
            current_camera_name = _get_combobox_value(self.video_source_widget)
            if current_camera_name != VIDEO_TEST_INPUT:
                standard_name = _get_combobox_value(widget)
                cam = self.app.devices["cameras"][current_camera_name]
                d = cameras.set_v4l2_video_standard(device_name=current_camera_name, standard=standard_name)
                def _cb2(cameras):
                    try:
                        cam = cameras[current_camera_name]
                    except KeyError, e:
                        print("Camera %s disappeared !" % (current_camera_name))
                    else:
                        actual_standard = cam["standard"]
                        if actual_standard != standard_name:
                            msg = _("Could not change V4L2 standard from %s to %s for device %s.") % (actual_standard, standard_name, current_camera_name)
                            print(msg)
                            # Maybe we should show an error dialog in that case, or set the value to what it really is.
                        else:
                            print("Successfully changed standard to %s for device %s." % (actual_standard, current_camera_name))
                def _cb(result):
                    d2 = cameras.list_cameras()
                    d2.addCallback(_cb2)
                d.addCallback(_cb)
        
    def on_v4l2_input_changed(self, widget):
        """
        When the user changes the V4L2 input, we actually change this input using milhouse.
        Calls `milhouse --videodevice /dev/videoX --v4l2-input N
        """
        if self._v4l2_input_changed_by_user:
            # change input for device
            current_camera_name = _get_combobox_value(self.video_source_widget)
            if current_camera_name != VIDEO_TEST_INPUT:
                input_name = _get_combobox_value(widget)
                cam = self.app.devices["cameras"][current_camera_name]
                input_number = cam["inputs"].index(input_name)
                d = cameras.set_v4l2_input_number(device_name=current_camera_name, input_number=input_number)
                def _cb2(cameras):
                    try:
                        cam = cameras[current_camera_name]
                    except KeyError, e:
                        print("Camera %s disappeared !" % (current_camera_name))
                    else:
                        actual_input = cam["input"]
                        if actual_input != input_number:
                            msg = _("Could not change V4L2 input from %s to %s for device %s.") % (actual_input, input_number, current_camera_name)
                            print(msg)
                            # Maybe we should show an error dialog in that case, or set the value to what it really is.
                        else:
                            print("Successfully changed input to %s for device %s." % (actual_input, current_camera_name))
                def _cb(result):
                    d2 = cameras.list_cameras()
                    d2.addCallback(_cb2)
                d.addCallback(_cb)

    # -------------------------- menu items -----------------
    
    def on_about_menu_item_activate(self, menu_item):
        About.create() # TODO: set parent window ?
    
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
        url = configure.ONLINE_HELP_URL 
        webbrowser.open(url)

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
            dialogs.ErrorDialog.create(text, parent=self.main_window)

    def _unschedule_offerer_invite_timeout(self):
        """ Unschedules our offer invite timeout function """
        if self._offerer_invite_timeout is not None and self._offerer_invite_timeout.active():
            self._offerer_invite_timeout.cancel()
            self._offerer_invite_timeout = None
    
    def _schedule_offerer_invite_timeout(self):
        """ Schedules our offer invite timeout function """
        def _cl_offerer_invite_timed_out():
            # XXX
            # in case of invite timeout, act as if we'd cancelled the invite ourselves
            self.on_invite_contact_cancelled()
            self.hide_calling_dialog("answTimeout")
            # here we return false so that this callback is unregistered
            return False

        if self._offerer_invite_timeout is None or not self._offerer_invite_timeout.active():
            self._offerer_invite_timeout = reactor.callLater(5, _cl_offerer_invite_timed_out)
        else:
            print("Warning: Already scheduled a timeout as we're already inviting a contact")

    def update_jackd_status(self):
        is_zombie = self.app.devices["jackd_is_zombie"]
        is_running = self.app.devices["jackd_is_running"]
        fill_stats = False
        if is_zombie:
                self.audio_jack_state_widget.set_markup("<b>Zombie</b>")
                self.audio_jack_icon_widget.set_from_stock(gtk.STOCK_DIALOG_WARNING, 4)
        else:
            if is_running:
                self.audio_jack_state_widget.set_markup("<b>Running</b>")
                self.audio_jack_icon_widget.set_from_stock(gtk.STOCK_YES, 4)
                fill_stats = True
            else:
                self.audio_jack_state_widget.set_markup("<b>Stopped</b>")
                self.audio_jack_icon_widget.set_from_stock(gtk.STOCK_NO, 4)
        if fill_stats:
            j = self.app.devices["jack_servers"][0] 
            latency = (j["period"] * j["nperiods"] / float(j["rate"])) * 1000 # ms
            self.jack_latency_widget.set_text("%4.2f ms" % (latency))
            self.jack_sampling_rate_widget.set_text("%d Hz" % (j["rate"]))
        else:
            self.jack_latency_widget.set_text("")
            self.jack_sampling_rate_widget.set_text("")
            
class About(object):
    """
    About dialog
    """
    def __init__(self):
        # TODO: set parent window ?
        self.icon_file = configure.get_icon_path()
        self.about_dialog = gtk.AboutDialog()

    def show_about_dialog(self):
        self.about_dialog.set_name(configure.APPNAME)
        self.about_dialog.set_role('about')
        self.about_dialog.set_version(__version__)
        commentlabel = configure.ONE_LINE_DESCRIPTION 
        self.about_dialog.set_comments(commentlabel)
        self.about_dialog.set_copyright(configure.COPYRIGHT_SHORT) 
        self.about_dialog.set_license(configure.LICENSE_TEXT)
        self.about_dialog.set_authors(configure.AUTHORS_LIST)
        #self.about_dialog.set_artists(['Public domain'])
        gtk.about_dialog_set_url_hook(self.show_website)
        self.about_dialog.set_website("http://svn.sat.qc.ca/trac/scenic")
        if not os.path.exists(self.icon_file):
            print("Could not find icon file %s." % (self.icon_file))
        else:
            large_icon = gtk.gdk.pixbuf_new_from_file(self.icon_file)
            self.about_dialog.set_logo(large_icon)
        # Connect to callbacks
        self.about_dialog.connect('response', self.destroy_about)
        self.about_dialog.connect('delete_event', self.destroy_about)
        self.about_dialog.connect("delete-event", self.destroy_about)
        self.about_dialog.show_all()

    @staticmethod
    def create():
        """
        @rettype: None
        """
        dialog = About()
        return dialog.show_about_dialog()
     
    def show_website(self, widget, data):
        webbrowser.open(data)

    def destroy_about(self, *args):
        self.about_dialog.destroy()
