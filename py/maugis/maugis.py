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

### GENERAL NOTES  ###
"""
- voir si il faut gerer une demande de connexion alors que c'est deja connecte
- voir si le bouton "cancel" est necesaire dans la fenetre "contacting" :
    - si oui il faudra trouver un moyen de faire la connection sans bloquer l'interface
    (thread, idle gtk ou io_watch?)
- en prod regler test a 0
- bug pour setter le bouton par defaut quand on change de tab. Il faut que le tab est le focus pour que ca marche. Pourtant le "print" apparait ???
"""
### CONSTANTS ###
__version__ = 1.0
APP_NAME = "maugis" # changed in __main__

### MODULES IMPORTS  ###

import sys
import os, signal
import time
import socket
import smtplib
import re
import subprocess
from optparse import OptionParser
import propulseart
PACKAGE_DATA = os.path.dirname(propulseart.__file__)
try:
    import pygtk
    pygtk.require("2.0")
except:
    pass
try:
    import gtk
    import gtk.glade
    import gobject
except ImportError, e:
    print "Could not load GTK or glade. Install python-gtk2 and python-glade2.", str(e)
    sys.exit(1)
# JSON import:
try:
    import json # python 2.6
except ImportError:
    import simplejson as json # python 2.4 to 2.5
try:
    _tmp = json.loads
except AttributeError:
    import warnings
    warnings.warn("Use simplejson, not the old json module.")
    sys.modules.pop('json') # get rid of the bad json module
    import simplejson as json

### MULTILINGUAL SUPPORT ###
DIR = os.path.join(PACKAGE_DATA, "locale")
import gettext
_ = gettext.gettext
gettext.bindtextdomain(APP_NAME, DIR)
gettext.textdomain(APP_NAME)
gtk.glade.bindtextdomain(APP_NAME, DIR)
gtk.glade.textdomain(APP_NAME)

class Config(object):
    def __init__(self):
        # Default values
        self.negotiation_port = 17446
        self.streamer_command = "milhouse"
        self.smtpserver = "smtp.sat.qc.ca"
        self.emailinfo = "maugis@sat.qc.ca"
        self.audio_input = "jackaudiosrc"
        self.audio_output = "jackaudiosink"
        self.audio_codec = "raw"
        self.audio_channels = 8
        self.video_input = "v4l2src"
        self.video_device = "/dev/video0"
        self.video_output = "xvimagesink"
        self.video_codec = "mpeg4"
        self.video_bitrate = "3000000"
        self.video_port = 8000
        self.audio_port = self.video_port + 10
        self.bandwidth = 30

        config_file = 'maugis.cfg'
        if os.path.isfile('/etc/' + config_file):
            config_path = '/etc/'
        else:
            config_path = os.environ['HOME'] + '/.maugis/'
        self.config_path = config_path + config_file
        if os.path.isfile(self.config_path):
            self._read()
        else:
            if not os.path.isdir(config_path):
                os.mkdir(config_path)
            self._write()

    def _write(self):
        global __version__
        config_str = _("# Configuration written by %(app)s %(version)s\n") % {'app': APP_NAME, 'version': __version__}
        for c in dir(self.__class__):
            if c[0] != '_' and hasattr(self, c):
                inst_attr = getattr(self, c)
                if inst_attr == getattr(Config, c):
                    comment = "# "
                else:
                    comment = ""
                config_str += "\n" + comment + c + "=" + str(inst_attr)
        config_file = file(self.config_path, "w")
        config_file.write(config_str)
        config_file.close()

    def _read(self):
        config_file  = file(self.config_path, "r")
        for line in config_file:
            line = line.strip()
            if line and line[0] != "#" and len(line) > 2:
                try:
                    item = line.split("=")
                    item[0] = item[0].strip()
                    item[1] = item[1].strip()
                    if item[1].isdigit():
                        item[1] = int(item[1])
                    setattr(self, item[0], item[1])
                except:
                    pass
        config_file.close()

class AddressBook(object):
    """
    READING & WRITING ADDRESS BOOK FILE 
    """
    def __init__(self):
        self.list = []
        self.selected = 0
        self.ad_book_name = os.environ['HOME'] + '/.maugis/maugis.adb'
        self.read()

    def read(self):
        if os.path.isfile(self.ad_book_name):
            self.list = []
            ad_book_file = file(self.ad_book_name, "r")
            for line in ad_book_file:
                if line[:4] == "sel:":
                    self.selected = int(line[4:].strip())
                else:
                    try:
                        self.list.append(json.loads(line))
                    except:
                        pass
            ad_book_file.close()

    def write(self):
        if ((os.path.isfile(self.ad_book_name)) or (len(self.list) > 0)):
            try:
                ad_book_file = file(self.ad_book_name, "w")
                for contact in self.list:
                    ad_book_file.write(json.dumps(contact) + "\n")
                if self.selected:
                    ad_book_file.write("sel:" + str(self.selected) + "\n")
                ad_book_file.close()
            except:
                print _("Cannot write Address Book file")

class Application(object):
    """
    Main application (arguably God) class
    """
    def __init__(self, kiosk):
        self.config = Config()
        self.ad_book = AddressBook()
        self.process_manager = ProcessManager(self.config)
        self.server = Server(self)

        # Set the Glade file
        glade_file = os.path.join(PACKAGE_DATA, 'maugis.glade')
        if os.path.isfile(glade_file):
            glade_path = glade_file
        else:
            text = _("<b><big>Could not find the Glade file?</big></b>\n\n" \
                    "Be sure the file %s is in /usr/share/maugis/. Quitting.") % glade_file
            print text
            sys.exit()
        self.widgets = gtk.glade.XML(glade_path, domain=APP_NAME)
        
        # connects callbacks to widgets automatically
        cb = {}
        for n in dir(self.__class__):
            if n[0] != '_' and hasattr(self, n):
                cb[n] = getattr(self, n)
        self.widgets.signal_autoconnect(cb)

        # get all the widgets that we use
        self.main_window = self.widgets.get_widget("mainWindow")
        self.dialog = self.widgets.get_widget("normalDialog")
        self.dialog.set_transient_for(self.main_window)
        self.confirm_label = self.widgets.get_widget("confirmLabel")
        self.contacting_window = self.widgets.get_widget("contactingWindow")
        self.contact_dialog = self.widgets.get_widget("contactDialog")
        self.contact_dialog.set_transient_for(self.main_window)
        self.contact_problem_label = self.widgets.get_widget("contactProblemLabel")
        self.contact_request_dialog = self.widgets.get_widget("contactRequestDialog")
        self.contact_request_dialog.set_transient_for(self.main_window)
        self.contact_request_label = self.widgets.get_widget("contactRequestLabel")
        self.edit_contact_window = self.widgets.get_widget("editContactWindow")
        self.edit_contact_window.set_transient_for(self.main_window)
        self.contact_name_entry = self.widgets.get_widget("contactNameEntry")
        self.contact_ip_entry = self.widgets.get_widget("contactIPEntry")
        self.contact_port_entry = self.widgets.get_widget("contactPortEntry")
        self.contact_edit_but = self.widgets.get_widget("contactEditBut")
        self.remove_contact = self.widgets.get_widget("removeContact")
        self.contact_join_but = self.widgets.get_widget("contactJoinBut")
        self.info_label = self.widgets.get_widget("infoLabel")
        self.contact_list = self.widgets.get_widget("contactList")
        self.negotiation_port_entry = self.widgets.get_widget("netConfPortEntry")
        self.net_conf_bw_combo = self.widgets.get_widget("netConfBWCombo")
        
        self.server.start_listening()

        # switch to Kiosk mode if asked
        if kiosk:
            self.main_window.set_decorated(False)
            self.widgets.get_widget("sysBox").show()
        
        # Build the contact list view
        self.selection = self.contact_list.get_selection()
        self.contact_tree = gtk.ListStore(str)
        self.contact_list.set_model(self.contact_tree)
        column = gtk.TreeViewColumn(_("Contacts"), gtk.CellRendererText(), markup=0)
        self.contact_list.append_column(column)
        self.main_window.show()
        
    def on_main_window_destroy(self, *args):
        self.server.close()
        self.ad_book.write()
        self.process_manager.stop()
        gtk.main_quit()

    def on_main_tabs_switch_page(self, widget, notebook_page, page_number):
        tab = widget.get_nth_page(page_number)
        if tab == "localPan":
            self.widgets.get_widget("netConfSetBut").grab_default()
        elif tab == "contactPan":
            self.widgets.get_widget("contactJoinBut").grab_default()

    def on_contact_list_changed(self, widget):
        list, self.row = widget.get_selected()
        if self.row:
            self.contact_edit_but.set_sensitive(True)
            self.remove_contact.set_sensitive(True)
            self.contact_join_but.set_sensitive(True)
            self.num = list.get_path(self.row)[0]
            self.ad_book.contact = self.ad_book.list[self.num]
            self.ad_book.selected = self.num
        else:
            self.contact_edit_but.set_sensitive(False)
            self.remove_contact.set_sensitive(False)
            self.contact_join_but.set_sensitive(False)
            self.ad_book.contact = None

    def on_contact_list_row_activated(self, *args):
        self.on_contact_edit_but_clicked(args)

    def on_add_contact_clicked(self, *args):
        self.ad_book.new_contact = 1
        self.contact_name_entry.set_text("")
        self.contact_ip_entry.set_text("")
        self.contact_port_entry.set_text("")
        self.edit_contact_window.show()

    def on_remove_contact_clicked(self, *args):
        text = _("<b><big>Delete this contact from the list?</big></b>\n\nAre you sure you want "
                "to delete this contact from the list?")
        if self.set_confirm_dialog(text):
            del self.ad_book.list[self.num]
            self.contact_tree.remove(self.row)
            self.ad_book.write()
            num = self.num - 1
            if num < 0:
                num = 0
            self.selection.select_path(num)

    def on_contact_edit_but_clicked(self, *args):
        self.contact_name_entry.set_text(self.ad_book.contact["name"])
        self.contact_ip_entry.set_text(self.ad_book.contact["address"])
        self.contact_port_entry.set_text(str(self.ad_book.contact["port"]))
        self.edit_contact_window.show()

    def on_edit_contact_window_delete_event(self, widget):
        widget.hide()
        return True

    def on_edit_contact_cancel_but_clicked(self, *args):
        self.edit_contact_window.hide()

    def on_edit_contact_save_but_clicked(self, *args):
        ad_book = self.ad_book
        #Validate the port number
        port = self.contact_port_entry.get_text()
        if port == "":
            port = str(self.config.video_port) #set port to default
        elif (not port.isdigit()) or (int(port) not in range(1,10000)):
            text = _("<b><big>The port number is not valid</big></b>\n\nEnter a valid port number in the range of 1000-99999")
            if self.set_contact_dialog(text):
                return

        #Validate the address
        addr = self.contact_ip_entry.get_text()
        if len(addr) < 7:
            text = _("<b><big>The address is not valid</big></b>\n\nEnter a valid address\nExample: 168.123.45.32 or tot.sat.qc.ca")
            if self.set_contact_dialog(text):
                return

        if ad_book.new_contact:
            self.contact_tree.append(    ["<b>" + self.contact_name_entry.get_text()
                                        + "</b>\n  IP: " + addr
                                        + "\n  Port: " + port]  )
            ad_book.list.append({})
            self.selection.select_path(len(ad_book.list) - 1)
            ad_book.contact = ad_book.list[len(ad_book.list) - 1]
            ad_book.new_contact = 0
        else:
            self.contact_tree.set_value(self.row, 0, "<b>" + self.contact_name_entry.get_text() + "</b>\n  IP: " + addr + "\n  Port: " + port)
        ad_book.contact["name"] = self.contact_name_entry.get_text()
        ad_book.contact["address"] = addr
        ad_book.contact["port"] = int(port)
        ad_book.write()
        self.edit_contact_window.hide()

    def on_net_conf_set_but_clicked(self, *args):
        os.system('gksudo "network-admin"')

    def on_sys_shutdown_but_clicked(self, *args):
        text = _("<b><big>Shutdown the computer?</big></b>\n\nAre you sure you want to shutdown the computer now?")
        if self.set_confirm_dialog(text):
            os.system('gksudo "shutdown -h now"')

    def on_sys_reboot_but_clicked(self, *args):
        text = _("<b><big>Reboot the computer?</big></b>\n\nAre you sure you want to reboot the computer now?")
        if self.set_confirm_dialog(text):
            os.system('gksudo "shutdown -r now"')

    def on_maint_upd_but_clicked(self, *args):
        os.system('gksudo "update-manager"')

    def on_maint_send_but_clicked(self, *args):
        text = _("<b><big>Send the settings?</big></b>\n\nAre you sure you want to send your computer settings to the administrator of maugis?")
        if self.set_confirm_dialog(text):
            msg = "--- milhouse_send ---\n" + self.milhouse_send_version + "\n\n"
            msg += "--- milhouse_recv ---\n" + self.milhouse_recv_version + "\n\n"
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
                msg += "Error executing 'uname -a'\n\n"
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

            fromaddr = "maugis@sat.qc.ca"
            toaddrs  = self.config.emailinfo
            toaddrs = toaddrs.split(', ')
            server = smtplib.SMTP(self.config.smtpserver)
            server.set_debuglevel(0)
            try:
                server.sendmail(fromaddr, toaddrs, msg)
            except:
                text = _("<b><big>Could not send info.</big></b>\n\nCheck your internet connection.")
                if self.set_contact_dialog(text):
                    pass
            server.quit()

    def on_client_join_but_clicked(self, *args):
        self.contacting_window.show()
        msg = json.dumps({"command":"ask", "port":self.ad_book.contact["port"], "bandwidth":self.config.bandwidth})
        client = Client(self)
        gobject.idle_add(client.connect, self.ad_book.contact["address"], msg)

    def on_net_conf_bw_combo_changed(self, *args):
        base = 30
        num = 2 # number of choice
        step = base / num
        selection = self.net_conf_bw_combo.get_active()
        self.config.bandwidth = (selection + 1) * step
        self.config._write()

    def on_net_conf_port_entry_changed(self, *args):
        gobject.timeout_add(0, self.on_net_conf_port_entry_changed2, args)
        return False

    def on_net_conf_port_entry_changed2(self, *args):
        port = self.negotiation_port_entry.get_text()
        if not port.isdigit():
            self.widgets.get_widget("mainTabs").set_current_page(1)
            self.init_negotiation_port()
            text = _("<b><big>The port number is not valid</big></b>\n\nEnter a valid port number in the range of 1-999999")
            if self.set_contact_dialog(text):
                self.negotiation_port_entry.grab_focus()
                return False
        else:
            self.config.negotiation_port = int(port)
            self.config._write()
            self.server.close()
            self.server = Server(self)
            self.server.start_listening()

    def set_contact_dialog(self, text):
        self.contact_problem_label.set_label(text)
        answer = self.contact_dialog.run()
        self.contact_dialog.hide()
        return answer == gtk.RESPONSE_OK

    def set_confirm_dialog(self, text):
        self.confirm_label.set_label(text)
        answer = self.dialog.run()
        self.dialog.hide()
        return answer == gtk.RESPONSE_OK

    def hide_contacting_window(self, msg="", err=""):
        self.contacting_window.hide()
        text = None
        if msg == "err":
            text = _("<b><big>Contact unreacheable.</big></b>\n\nCould not connect to the IP address of this contact.")
        elif msg == "timeout":
            text = _("<b><big>Connection timeout.</big></b>\n\nCould not connect to the port of this contact.")
        elif msg == "answTimeout":
            text = _("<b><big>Contact answer timeout.</big></b>\n\nThe contact did not answer in an reasonable delay.")
        elif msg == "send":
            text = _("<b><big>Problem sending command.</big></b>\n\nError: %s") % err
        elif msg == "refuse":
            text = _("<b><big>Connection refuse.</big></b>\n\nThe contact refuse the connection.")
        elif msg == "badAnsw":
            text = _("<b><big>Invalid answer.</big></b>\n\nThe answer was not valid.")
        if text:
            if self.set_contact_dialog(text):
                pass

    def init_bandwidth(self):
        base = 30
        num = 2 # number of choice
        selection = int(round((self.config.bandwidth - 1) * num / base))
        if selection < 0:
            selection = 0
        elif selection > base:
            selection = base
        self.net_conf_bw_combo.set_active(selection)

    def init_negotiation_port(self):
        self.negotiation_port_entry.set_text(str(self.config.negotiation_port))

    def init_ad_book_list(self):
        ad_book = self.ad_book
        ad_book.contact = None
        ad_book.new_contact = 0
        if len(ad_book.list) > 0:
            for contact in ad_book.list:
                self.contact_tree.append(    ["<b>" + contact["name"]
                                            + "</b>\n  IP: "
                                            + contact["address"] + "\n  Port: "
                                            + str(contact["port"])] )
            self.selection.select_path(ad_book.selected)
        else:
            self.contact_edit_but.set_sensitive(False)
            self.remove_contact.set_sensitive(False)
            self.contact_join_but.set_sensitive(False)

    def on_server_rcv_command(self, server, (msg, addr, conn)):
        if msg and "command" in msg:
            cmd = msg["command"]
            if cmd == "ask":
                self.rcv_watch = gobject.timeout_add(5000, self.server_answer_timeout, addr[0])
                self.contact_request_label.set_label("<b><big>" + addr[0] + " is contacting you.</big></b>\n\nDo you accept the connection?")
                answ = self.contact_request_dialog.run()
                gobject.source_remove(self.rcv_watch)
                self.contact_request_dialog.hide()
                if answ == gtk.RESPONSE_OK:
                    if "bandwidth" in msg and self.config.bandwidth > msg["bandwidth"]:
                        bandwidth = msg["bandwidth"]
                    else:
                        bandwidth = self.config.bandwidth
                    conn.sendall(json.dumps({"answer":"accept", "bandwidth": bandwidth}))
                    conn.close()
                    self.process_manager.start(addr[0], bandwidth)
                else:
                    conn.sendall(json.dumps({"answer":"refuse"}))
                    conn.close()
            elif cmd == "stop":
                self.process_manager.stop()
                conn.sendall(json.dumps({"answer":"stopped"}))
                conn.close()
            else:
                conn.close()
        else:
            conn.close()

    def server_answer_timeout(self, addr):
        self.contact_request_dialog.response(gtk.RESPONSE_NONE)
        self.contact_request_dialog.hide()
        text = _("<b><big>%s was contacting you.</big></b>\n\nBut you did not answer in reasonable delay.") % addr
        if self.set_contact_dialog(text):
            pass
        return False

    def on_client_socket_timeout(self, client):
        self.hide_contacting_window("timeout")
    
    def on_client_socket_error(self, client, (err, msg)):
        self.hide_contacting_window(msg)
        print err

    def on_client_add_timeout(self, client):
        self.answ_watch = gobject.timeout_add(5000, self.client_answer_timeout, client)

    def on_client_rcv_command(self, client, (msg)):
        if(self.answ_watch):
            gobject.source_remove(self.answ_watch)
        if msg and "answer" in msg:
            answ = msg["answer"]
            if answ == "accept":
                self.hide_contacting_window("accept")
                if "bandwidth" in msg:
                    bandwidth = msg["bandwidth"]
                else:
                    bandwidth = self.config.bandwidth
                self.process_manager.start(client.host, bandwidth)
            elif answ == "refuse":
                self.hide_contacting_window("refuse")
            elif answ == "stopped":
                self.process_manager.stop()
            else:
                self.hide_contacting_window("badAnsw")

    def client_answer_timeout(self, client):
        if (client.io_watch):
            gobject.source_remove(client.io_watch)
        if self.contacting_window.get_property('visible'):
            self.hide_contacting_window("answTimeout")
        return False
    
    def on_start_milhouse_send(self):
        self.contact_join_but.set_sensitive(False)

    def on_stop_milhouse_send(self):
        self.contact_join_but.set_sensitive(True)

    def watch_milhouse_recv(self, (child, condition)):
        msg = json.dumps({"command":"stop"})
        client = Client(self)
        client.connect(self.ad_book.contact["address"], msg)


class ProcessManager(object):
    """
    PROCESS manager.
    """
    def __init__(self, config):
        self.config = config
        self.video_port = self.config.video_port
        self.audio_port = self.config.audio_port
        # receiver
        self.milhouse_recv_cmd = None
        self.milhouse_recv_pid = None
        # files
        self.milhouse_recv_input = None
        self.milhouse_recv_output = None
        self.milhouse_recv_error = None
        # File description watcher
        self.watched_milhouse_recv_id = None
        self.milhouse_recv_timeout = None # call_later
        # sender
        self.milhouse_send_cmd = None
        self.milhouse_send_subproc = None
        self.milhouse_send_pid = None
        self.milhouse_send_timeout = None
        
    def start(self, host, bandwidth):
        base = 30
        divider = base / bandwidth
        # First, start the milhouse_recv process, milhouse_send needs a remote running propulseart --receive to work
        self.milhouse_recv_cmd = [
            self.config.streamer_command,
            '--receiver', 
            '--address', host,
            '--videosink', self.config.video_output,
            '--audiosink', self.config.audio_output,
            '--videocodec', self.config.video_codec,
            '--audiocodec', self.config.audio_codec,
            '--videoport', str(self.video_port),
            '--audioport', str(self.audio_port) 
            ]
        print "milhouse_recv_cmd: ", self.milhouse_recv_cmd
        
        # local function declaration:
        def _env_sequence():
            return [key + '=' + value for key, value in os.environ.items()]
        
        self.milhouse_recv_pid, self.milhouse_recv_input, self.milhouse_recv_output, self.milhouse_recv_error = gobject.spawn_async(
            self.milhouse_recv_cmd,
            envp = _env_sequence(),
            working_directory = os.environ['PWD'],
            flags = gobject.SPAWN_SEARCH_PATH,
            standard_input = False,
            standard_output = True,
            standard_error = True)
        self.watched_milhouse_recv_id = gobject.io_add_watch(
            self.milhouse_recv_output,
            gobject.IO_HUP,
            self.watch_milhouse_recv)
        self.milhouse_send_cmd = [self.config.streamer_command, '--sender', 
            '--address', host,
            '--videosource', self.config.video_input,
            '--videocodec', self.config.video_codec,
            '--videobitrate', self.config.video_bitrate,
            '--audiosource', self.config.audio_input,
            '--audiocodec', self.config.audio_codec,
            '--videoport', str(self.video_port),
            '--audioport', str(self.audio_port)]
        print "milhouse_send_cmd: ", self.milhouse_send_cmd
        self.milhouse_send_subproc = subprocess.Popen(self.milhouse_send_cmd, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
        print "milhouse_send_cmd launched "
        self.milhouse_send_pid = self.milhouse_send_subproc.pid

    def watch_milhouse_recv(self, *args):
        print "watch_milhouse_recv"
        self.milhouse_recv_timeout = gobject.timeout_add(5000, self.stop)
        return False

    def watch_milhouse_send(self, *args):
        print "watch_milhouse_send"
        self.milhouse_send_timeout = gobject.timeout_add(5000, self.stop)
        return False

    def stop(self):
        print "stop: ", 
        if hasattr(self, "watched_milhouse_recv_id"):
            print "watch"
            try:
                gobject.source_remove(self.watched_milhouse_recv_id)
            except TypeError:
                pass
        if hasattr(self, "timeout"):
            print "timeout"
            gobject.source_remove(self.timeout)
        try:
            print "killing milhouse_recv: ", self.milhouse_recv_pid
            os.kill(self.milhouse_recv_pid, signal.SIGTERM)
            # not waiting for child process !!
        except:
            pass
        try:
            print "killing milhouse_send_pid: ", self.milhouse_send_pid
            os.kill(self.milhouse_send_pid, signal.SIGTERM)
            print "send: before os.wait()"
            os.wait()
            print "send: after os.wait()"
        except:
            pass
        self.on_stop_milhouse_send()

### NETWORK ###

class Network(object):
    def __init__(self, negotiation_port):
        self.buf_size = 1024
        self.port = negotiation_port
    
    def validate(self, msg):
        tmp_msg = msg.strip()
        msg = None
        if tmp_msg[0] == "{" and tmp_msg[-1] == "}" and tmp_msg.find("{", 1, -2) == -1 and tmp_msg.find("}", 1, -2) == -1:
            try:
                tmp_msg = json.loads(tmp_msg)
                if type(tmp_msg) is dict:
                    msg = tmp_msg
            except:
                pass
        return msg

    def recv(self, conn):
        data = conn.recv(self.buf_size)
        buffer = data
        while len(data) == self.buf_size: # maybe we should recall handle_data on io_watch
            data = conn.recv(self.buf_size)
            buffer += data
        return buffer

    def close(self):
        self.sock.close()


class Server(Network):
    def __init__(self, app):
        Network.__init__(self, app.config.negotiation_port)
        self.app = app
        self.host = ''

    def start_listening(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((self.host, self.port))
        sock.listen(1)
        gobject.io_add_watch(sock, gobject.IO_IN, self.handle_data)
        self.sock = sock

    def handle_data(self, source, condition):
        conn, addr = source.accept()
        buffer = self.recv(conn)
        msg = self.validate(buffer)
        self.app.on_server_rcv_command(self, (msg, addr, conn))
        return True


class Client(Network):
    def __init__(self, app):
        Network.__init__(self, app.config.negotiation_port)
        self.app = app
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(10)

    def connect(self, host, msg):
        self.host = host
        try:
            self.sock.connect((host, self.port))
        except socket.timeout, err:
            on_client_socket_timeout()
        except socket.error, err:
            on_client_socket_error()
        else:
            if self.send(msg):
                self.io_watch = gobject.io_add_watch(self.sock, gobject.IO_IN, self.handle_data)
        return False

    def send(self, msg):
        if not len(msg)%self.buf_size:
            msg += " "
        try:
            self.sock.sendall(msg)
            self.app.on_client_add_timeout()
            return True
        except socket.error, err:
            self.app.on_client_add_timeout()
            self.app.on_client_socket_error()
            return False

    def handle_data(self, source, condition):
        buffer = self.recv(source)
        source.close()
        msg = self.validate(buffer)
        self.app.on_client_rcv_command()
        return False

if __name__ == '__main__':
    # command line parsing
    APP_NAME = sys.argv[0]
    parser = OptionParser(usage="%prog", version=str(__version__))
    parser.add_option("-k", "--kiosk", action="store_true", dest="kiosk", \
            help="Run maugis in kiosk mode")
    (options, args) = parser.parse_args()
    main = Application(kiosk=options.kiosk)
    gtk.main()
    
