#!/usr/bin/env python


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

_TEST = 0
__version__ = 1.0
_DEBUG = 0


### MODULES IMPORTS  ###

import sys
import os, signal
import time
import socket
import smtplib
import re
import subprocess
import time
from optparse import OptionParser

try:
    import pygtk
    pygtk.require("2.0")
except:
    pass
try:
    import gtk
    import gtk.glade
    import gobject
except:
    sys.exit(1)



### MULTILINGUAL SUPPORT ###
APP = "maugis"
DIR = "locale"

import gettext

_ = gettext.gettext

gettext.bindtextdomain(APP, DIR)
gettext.textdomain(APP)
gtk.glade.bindtextdomain(APP, DIR)
gtk.glade.textdomain(APP)




### MAIN MEDIATOR(CONTROLLER)/COLLEAGUE CLASSES ###

class Mediator(object):
    def __init__(self):
        self.config = Config()

        self.ad_book = AddressBook()
        
        self.gstsend_proc = Processes(self)

        # command line parsing
        parser = OptionParser(usage="%prog", version=str(__version__))
        parser.add_option("-k", "--kiosk", action="store_true", dest="kiosk", \
                help="Run maugis in kiosk mode")
        (options, args) = parser.parse_args()
        
        self.gui = GuiClass(self, options.kiosk)

        self.server = Server(self)
        self.server.start_listening()
        
        gtk.main()

    def colleague_changed(self, colleague, event, event_args):
        if hasattr(self, event):
            if event_args:
                eval("self." + event)(colleague, event_args)
            else:
                eval("self." + event)(colleague)


    ### General Methods ###

    def set_contact_dialog(self, text, gui=None):
        if not gui:
            gui = self.gui
        gui.contact_problem_label.set_label(text)
        answ = gui.contact_dialog.run()
        gui.contact_dialog.hide()
        if answ == gtk.RESPONSE_OK:
            return 1
        else:
            return 0

    def set_confirm_dialog(self, text, gui=None):
        if not gui:
            gui = self.gui
        gui.confirm_label.set_label(text)
        answ = gui.dialog.run()
        gui.dialog.hide()
        if answ == gtk.RESPONSE_OK:
            return 1
        else:
            return 0

    def hide_contacting_window(self, msg="", err=""):
        self.gui.contacting_window.hide()
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


    ### Callbacks ###

    def check_ext_program(self, gui):
        global __version__, _TEST
        if not _TEST:
            # verify propulseart version
            try:
                w, r, err = os.popen3(self.config.gstsend + ' -v')
                err_str = err.read()
                if err_str:
                    text = _("<b><big>Could not start gstsend?</big></b>\n\nError: %s. Quitting.") % err_str
                    if self.set_contact_dialog(text, gui):
                        pass
                    sys.exit()
                else:
                    match = re.search(r'version ([^ \n]+)', r.readline(), re.I)
                    if match:
                        self.gstsend_version = match.group(1)
                    else:
                        self.gstsend_version = "?"
                w.close()
                r.close()
                err.close()
            except:
                text = _("<b><big>Could not start gstsend?</big></b>\n\nCould not start gstsend. Quitting.")
                if self.set_contact_dialog(text, gui):
                    pass
                sys.exit()

            try:
                w, r, err = os.popen3(self.config.gstrecv + ' -v')
                err_str = err.read()

                ### a remettre quand gstrecv ne retournera plus d'erreur par defaut ###
                #~ if err_str:
                    #~ text = (_("<b><big>Could not start gstrecv?</big></b>\n\nError: %(err_str). Quitting."), err_str)
                    #~ if self.set_contact_dialog(text, gui):
                        #~ pass
                    #~ sys.exit()
                #~ else:
                match = re.search(r'version ([^ \n]+)', r.readline(), re.I)
                if match:
                    self.gstrecv_version = match.group(1)
                else:
                    self.gstrecv_version = "?"

                w.close()
                r.close()
                err.close()
            except:
                text = _("<b><big>Could not start gstrecv?</big></b>\n\nCould not start gstrecv. Quitting.")
                if self.set_contact_dialog(text, gui):
                    pass
                sys.exit()

            else:
                text = _("<b><big>maugis</big></b>\nVersion: ")
                text += str(__version__)
                text += _("\ngstsend: ") + self.gstsend_version
                text += _("\ngstrecv: ") + self.gstrecv_version
                text += _("\nCopyright: SAT")
                text += _("\nAuthors: Etienne Desautels")

        if _TEST:
            self.gstsend_version = '1.23'
            self.gstrecv_version = '1.21'

            text = _("<b><big>maugis</big></b>\nVersion: ")
            text += str(__version__)
            text += _("\ngstsend: ") + self.gstsend_version
            text += _("\ngstrecv: ") + self.gstrecv_version
            text += _("\nCopyright: SAT")
            text += _("\nAuthors: Etienne Desautels")
                        
        gui.info_label.set_label(text)


    def on_main_window_destroy(self, colleague, (widget)):
        self.server.close()
        self.ad_book.write()
        self.gstsend_proc.stop()
        gtk.main_quit()


    def on_main_tabs_switch_page(self, gui, (widget, page, page_num)):
        tab = widget.get_nth_page(page_num).name
        if tab == "localPan":
            gui.widgets.get_widget("netConfSetBut").grab_default()
        if tab == "contactPan":
            gui.widgets.get_widget("contactJoinBut").grab_default()

    def init_bandwidth(self, gui):
        base = 30
        num = 2 # number of choice
        selection = int(round((self.config.bandwidth - 1) * num / base))
        if selection < 0:
            selection = 0
        elif selection > base:
            selection = base
        gui.net_conf_bw_combo.set_active(selection)

    def on_net_conf_bw_combo_changed(self, gui, (widget)):
        base = 30
        num = 2 # number of choice
        step = base / num
        selection = gui.net_conf_bw_combo.get_active()
        self.config.bandwidth = (selection + 1) * step
        self.config._write()

    def init_negotiation_port(self, gui):
        gui.negotiation_port_entry.set_text(str(self.config.negotiationport))

    def on_net_conf_port_entry_changed2(self, gui, (widget)):
        port = gui.negotiation_port_entry.get_text()
        if not port.isdigit():
            gui.widgets.get_widget("mainTabs").set_current_page(1)
            self.init_negotiation_port(gui)
            text = _("<b><big>The port number is not valid</big></b>\n\nEnter a valid port number in the range of 1-999999")
            if self.set_contact_dialog(text):
                gui.negotiation_port_entry.grab_focus()
                return False
        else:
            self.config.negotiationport = int(port)
            self.config._write()
            self.server.close()
            self.server = Server(self)
            self.server.start_listening()
        

    def init_ad_book_list(self, gui):
        ad_book = self.ad_book
        ad_book.contact = None
        ad_book.new_contact = 0
        
        if len(ad_book.list) > 0:
            for contact in ad_book.list:
                gui.contact_tree.append(    ["<b>" + contact["name"]
                                            + "</b>\n  IP: "
                                            + contact["address"] + "\n  Port: "
                                            + str(contact["port"])] )
            gui.selection.select_path(ad_book.selected)
        else:
            gui.contact_edit_but.set_sensitive(False)
            gui.remove_contact.set_sensitive(False)
            gui.contact_join_but.set_sensitive(False)

    def on_contact_list_changed(self, gui, (widget, data)):
        list, gui.row = widget.get_selected()
        if gui.row:
            gui.contact_edit_but.set_sensitive(True)
            gui.remove_contact.set_sensitive(True)
            gui.contact_join_but.set_sensitive(True)

            self.num = list.get_path(gui.row)[0]
            self.ad_book.contact = self.ad_book.list[self.num]
            self.ad_book.selected = self.num
        else:
            gui.contact_edit_but.set_sensitive(False)
            gui.remove_contact.set_sensitive(False)
            gui.contact_join_but.set_sensitive(False)

            self.ad_book.contact = None

    def on_add_contact_clicked(self, gui, (widget)):
        self.ad_book.new_contact = 1
        gui.contact_name_entry.set_text("")
        gui.contact_ip_entry.set_text("")
        gui.contact_port_entry.set_text("")
        gui.edit_contact_window.show()

    def on_remove_contact_clicked(self, gui, (widget)):
        text = _("<b><big>Delete this contact from the list?</big></b>\n\nAre you sure you want to delete this contact from the list?")
        if self.set_confirm_dialog(text):
            del self.ad_book.list[self.num]
            gui.contact_tree.remove(gui.row)
            self.ad_book.write()
            num = self.num - 1
            if num < 0:
                num = 0
            gui.selection.select_path(num)

    def on_contact_edit_but_clicked(self, gui, (widget)):
        gui.contact_name_entry.set_text(self.ad_book.contact["name"])
        gui.contact_ip_entry.set_text(self.ad_book.contact["address"])
        gui.contact_port_entry.set_text(str(self.ad_book.contact["port"]))
        gui.edit_contact_window.show()

    def on_edit_contact_window_delete_event(self, gui, (widget, event)):
        widget.hide()

    def on_edit_contact_cancel_but_clicked(self, gui, (widget)):
        gui.edit_contact_window.hide()

    def on_edit_contact_save_but_clicked(self, gui, (widget)):
        ad_book = self.ad_book
        #Validate the port number
        port = gui.contact_port_entry.get_text()
        if port == "":
            port = str(self.config.gstsendport) #set port to default
        elif (not port.isdigit()) or (int(port) not in range(1,10000)):
            text = _("<b><big>The port number is not valid</big></b>\n\nEnter a valid port number in the range of 1000-99999")
            if self.set_contact_dialog(text):
                return

        #Validate the address
        addr = gui.contact_ip_entry.get_text()
        if len(addr) < 7:
            text = _("<b><big>The address is not valid</big></b>\n\nEnter a valid address\nExample: 168.123.45.32 or tot.sat.qc.ca")
            if self.set_contact_dialog(text):
                return

        if ad_book.new_contact:
            gui.contact_tree.append(    ["<b>" + gui.contact_name_entry.get_text()
                                        + "</b>\n  IP: " + addr
                                        + "\n  Port: " + port]  )
            ad_book.list.append({})
            gui.selection.select_path(len(ad_book.list) - 1)
            ad_book.contact = ad_book.list[len(ad_book.list) - 1]
            ad_book.new_contact = 0
        else:
            gui.contact_tree.set_value(gui.row, 0, "<b>" + gui.contact_name_entry.get_text() + "</b>\n  IP: " + addr + "\n  Port: " + port)
        ad_book.contact["name"] = gui.contact_name_entry.get_text()
        ad_book.contact["address"] = addr
        ad_book.contact["port"] = int(port)
        ad_book.write()
        gui.edit_contact_window.hide()


    def on_net_conf_set_but_clicked(self, gui, (widget)):
        os.system('gksudo "network-admin"')

    def on_sys_shutdown_but_clicked(self, gui, (widget)):
        text = _("<b><big>Shutdown the computer?</big></b>\n\nAre you sure you want to shutdown the computer now?")
        if self.set_confirm_dialog(text):
            os.system('gksudo "shutdown -h now"')

    def on_sys_reboot_but_clicked(self, gui, (widget)):
        text = _("<b><big>Reboot the computer?</big></b>\n\nAre you sure you want to reboot the computer now?")
        if self.set_confirm_dialog(text):
            os.system('gksudo "shutdown -r now"')

    def on_maint_upd_but_clicked(self, gui, (widget)):
        os.system('gksudo "update-manager"')

    def on_maint_send_but_clicked(self, gui, (widget)):
        text = _("<b><big>Send the settings?</big></b>\n\nAre you sure you want to send your computer settings to the administrator of maugis?")
        if self.set_confirm_dialog(text):
            msg = "--- gstsend ---\n" + self.gstsend_version + "\n\n"
            msg += "--- gstrecv ---\n" + self.gstrecv_version + "\n\n"
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

    def on_client_join_but_clicked(self, gui, (widget)):
        gui.contacting_window.show()
        msg = repr({"command":"ask", "port":self.ad_book.contact["port"], "bandwidth":self.config.bandwidth})
        client = Client(self)   ###############
        gobject.idle_add(client.connect, self.ad_book.contact["address"], msg)
    

    def on_server_rcv_command(self, server, (msg, addr, conn)):
        if msg and "command" in msg:
            cmd = msg["command"]
            if cmd == "ask":
                self.rcv_watch = gobject.timeout_add(5000, self.server_answer_timeout, addr[0])
                self.gui.contact_request_label.set_label("<b><big>" + addr[0] + " is contacting you.</big></b>\n\nDo you accept the connection?")
                answ = self.gui.contact_request_dialog.run()
                gobject.source_remove(self.rcv_watch)
                self.gui.contact_request_dialog.hide()
                if answ == gtk.RESPONSE_OK:
                    if "bandwidth" in msg and self.config.bandwidth > msg["bandwidth"]:
                        bandwidth = msg["bandwidth"]
                    else:
                        bandwidth = self.config.bandwidth
                    conn.sendall(repr({"answer":"accept", "bandwidth": bandwidth}))
                    conn.close()
                    self.gstsend_proc.start(addr[0], bandwidth)
                else:
                    conn.sendall(repr({"answer":"refuse"}))
                    conn.close()
            elif cmd == "stop":
                self.gstsend_proc.stop()
                conn.sendall(repr({"answer":"stopped"}))
                conn.close()
            else:
                conn.close()
        else:
            conn.close()

    def server_answer_timeout(self, addr):
        self.gui.contact_request_dialog.response(gtk.RESPONSE_NONE)
        self.gui.contact_request_dialog.hide()
        text = _("<b><big>%s was contacting you.</big></b>\n\nBut you did not answer in reasonable delay.") % addr
        if self.set_contact_dialog(text):
            pass
        return False


    def on_client_socket_timeout(self, client):
        #~ if self.widgets: ############
        self.hide_contacting_window("timeout")
    
    def on_client_socket_error(self, client, (err, msg)):
        #~ if self.widgets: ############
        self.hide_contacting_window(msg)
        print err

    def on_client_add_timeout(self, client):
        #~ if self.gui.contacting_window.get_property('visible'):
        self.answ_watch = gobject.timeout_add(5000, self.client_answer_timeout, client)
        #~ return False

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
                self.gstsend_proc.start(client.host, bandwidth)
            elif answ == "refuse":
                self.hide_contacting_window("refuse")
            elif answ == "stopped":
                self.gstsend_proc.stop()
            else:
                self.hide_contacting_window("badAnsw")

    def client_answer_timeout(self, client):
        if (client.io_watch):
            gobject.source_remove(client.io_watch)
        if self.gui.contacting_window.get_property('visible'):
            self.hide_contacting_window("answTimeout")
        return False
    
    
    def on_start_gstsend(self, colleague):
        self.gui.contact_join_but.set_sensitive(False)

    def on_stop_gstsend(self, colleague):
        self.gui.contact_join_but.set_sensitive(True)

    def watch_gstrecv(self, colleague, (child, condition)):
        msg = repr({"command":"stop"})
        client = Client(self)
        client.connect(self.ad_book.contact["address"], msg)
        #~ client.close()
        


class Colleague:
    def __init__(self, med):
        self.med = med

    def _changed(self, colleague, event_args=None, event=None):
        if not event:
            event = sys._getframe(1).f_code.co_name
        self.med.colleague_changed(colleague, event, event_args)



### READING AND WRITING CONFIGURATION FILE ###

class Config:
    # Default values
    gstsendport = 8000
    negotiationport = 17446
    gstsend = "milhouse"
    gstrecv = gstsend
    smtpserver = "smtp.sat.qc.ca"
    emailinfo = "maugis@sat.qc.ca"
    audiolib = "oss"
    audio_input = "jackaudiosrc"
    audio_output = "jackaudiosink"
    audio_codec = "raw"
    audio_channels = 8
    video_input = "v4l2src"
    video_device = "/dev/video0"
    video_output = "xvimagesink"
    video_codec = "mpeg4"
    video_bitrate = "3000000"
    video_port = gstsendport
    audio_port = video_port + 10

    bandwidth = 30


    def __init__(self):
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
        config_str = _("# Configuration written by %(app)s %(version)s\n") % {'app': sys.argv[0], 'version': __version__}
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


### READING & WRITING ADDRESS BOOK FILE ###

class AddressBook:
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
                        self.list.append(eval(line))
                    except:
                        pass
            ad_book_file.close()

    def write(self):
        if ((os.path.isfile(self.ad_book_name)) or (len(self.list) > 0)):
            try:
                ad_book_file = file(self.ad_book_name, "w")
                for contact in self.list:
                    ad_book_file.write(repr(contact) + "\n")
                if self.selected:
                    ad_book_file.write("sel:" + str(self.selected) + "\n")
                ad_book_file.close()
            except:
                print _("Cannot write Address Book file")





### GTK GUI ###

class GuiClass(Colleague):
    def __init__(self, med, kiosk):
        global _TEST, __version__
        
        Colleague.__init__(self, med)

        # Set the Glade file
        glade_file = 'maugis.glade'
        if os.path.isfile(glade_file):
            glade_path = glade_file
        elif os.path.isfile('/usr/share/maugis/' + glade_file):
            glade_path = '/usr/share/maugis/' + glade_file
        else:
            text = _("<b><big>Could not find the Glade file?</big></b>\n\nBe sure the file %s is in /usr/share/maugis/. Quitting.") % glade_file
            print text
            sys.exit()


        self.widgets = gtk.glade.XML(glade_path, domain=APP)
        
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
#       self.client_preview = self.widgets.get_widget("clientPreviewWindow")
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


        # verify gstrecv and gstsend
        #self._changed(self, event="check_ext_program")
        
        # adjust the bandwidth combobox iniline with the config
        self._changed(self, event="init_bandwidth")
        
        # switch to Kiosk mode if asked
        if kiosk:
            self.main_window.set_decorated(False)
            self.widgets.get_widget("sysBox").show()

        # Build the contact list view
        self.selection = self.contact_list.get_selection()
        self.selection.connect("changed", self.on_contact_list_changed, None)

        self.contact_tree = gtk.ListStore(str)
        self.contact_list.set_model(self.contact_tree)
        column = gtk.TreeViewColumn(_("Contacts"), gtk.CellRendererText(), markup=0)
        self.contact_list.append_column(column)
        
        self._changed(self, event="init_ad_book_list")
        
        self._changed(self, event="init_negotiation_port")
        
        self.main_window.show()
        
        
    ### GUI Callbacks ###
    
    def on_main_window_destroy(self, *args):
        self._changed(self, args)


    def on_main_tabs_switch_page(self, *args):
        self._changed(self, args)


    def on_contact_list_changed(self, *args):
        self._changed(self, args)

    def on_contact_list_row_activated(self, *args):
        self._changed(self, args, event="on_contact_edit_but_clicked")

    def on_add_contact_clicked(self, *args):
        self._changed(self, args)

    def on_remove_contact_clicked(self, *args):
        self._changed(self, args)

    def on_contact_edit_but_clicked(self, *args):
        self._changed(self, args)

    def on_edit_contact_window_delete_event(self, *args):
        self._changed(self, args)
        return True

    def on_edit_contact_cancel_but_clicked(self, *args):
        self._changed(self, args)

    def on_edit_contact_save_but_clicked(self, *args):
        self._changed(self, args)


    def on_net_conf_set_but_clicked(self, *args):
        self._changed(self, args)

    def on_sys_shutdown_but_clicked(self, *args):
        self._changed(self, args)

    def on_sys_reboot_but_clicked(self, *args):
        self._changed(self, args)

    def on_maint_upd_but_clicked(self, *args):
        self._changed(self, args)

    def on_maint_send_but_clicked(self, *args):
        self._changed(self, args)


    def on_client_join_but_clicked(self, *args):
        self._changed(self, args)


    def on_net_conf_bw_combo_changed(self, *args):
        self._changed(self, args)

    def on_net_conf_port_entry_changed(self, *args):
        gobject.timeout_add(0, self.on_net_conf_port_entry_changed2, args)
        return False

    def on_net_conf_port_entry_changed2(self, *args):
        self._changed(self, args)

### DV... PROCESS ###

class Processes(Colleague):
    global _DEBUG
    def __init__(self, med):
        Colleague.__init__(self, med)
        self.config = med.config
        self.video_port = self.config.gstsendport
        self.audio_port = self.video_port + 10
        
    def start(self, host, bandwidth):
    
        self._changed(self, event="on_start_gstsend")
        
        base = 30
        divider = base / bandwidth
        

        # First start the gst_recv process, gstsend needs a remote running propulseart --receive to work
        self.gstrecv_cmd = [self.config.gstrecv,
                                        '--receiver', 
                                        '--address', host,
                                        '--videosink', self.config.video_output,
                                        '--audiosink', self.config.audio_output,
                                        '--videocodec', self.config.video_codec,
                                        '--audiocodec', self.config.audio_codec,
                                        '--videoport', str(self.video_port),
                                        '--audioport', str(self.audio_port) ]
        print "gstrecv_cmd: ", self.gstrecv_cmd
        def env_sequence():
            return [key + '=' + value for key, value in os.environ.items()]
        self.gstrecv_pid, self.gstrecv_input, self.gstrecv_output, self.gstrecv_error = gobject.spawn_async(self.gstrecv_cmd,
                                            envp = env_sequence(),
                                            working_directory = os.environ['PWD'],
                                            flags = gobject.SPAWN_SEARCH_PATH,
                                            standard_input = False,
                                            standard_output = True,
                                            standard_error = True)

        self.watched_gstrecv = gobject.io_add_watch(  self.gstrecv_output,
                                            gobject.IO_HUP,
                                            self.watch_gstrecv  )
        # Now we launch the sender
        # Do we need a little sleep?
        #self.gstrecv_child = subprocess.Popen( '/usr/local/bin/propulseart --receiver --address %s --videocodec h264 --audiocodec vorbis --videoport 8000 --audioport 8010' %host )

        time.sleep(1)

        if _DEBUG:
            self.gstsend_cmd = [self.config.gstsend, '--sender', 
                                            '--address', host,
                                            '--videosource', self.config.video_input,
                                            '--videocodec', self.config.video_codec,
                                            '--videobitrate', self.config.video_bitrate,
                                            '--audiosource', self.config.audio_input,
                                            '--audiocodec', self.config.audio_codec,
                                            '--videoport', str(self.video_port),
                                            '--audioport', str(self.audio_port)]
            self.gstsend_cmd = ['xlogo']
        else:
            self.gstsend_cmd = [self.config.gstsend, '--sender', 
                                            '--address', host,
                                            '--videosource', self.config.video_input,
                                            '--videocodec', self.config.video_codec,
                                            '--videobitrate', self.config.video_bitrate,
                                            '--audiosource', self.config.audio_input,
                                            '--audiocodec', self.config.audio_codec,
                                            '--videoport', str(self.video_port),
                                            '--audioport', str(self.audio_port)]

        print "gstsend_cmd: ", self.gstsend_cmd
        self.gstsend_subproc = subprocess.Popen(self.gstsend_cmd, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
        print "gstsend_cmd launched "
        self.gstsend_pid = self.gstsend_subproc.pid


    def watch_gstrecv(self, *args):
        print "watch_gstrecv"
        self._changed(self, args)
        self.gstrecv_timeout = gobject.timeout_add(5000, self.stop)
        return False

    def watch_gstsend(self, *args):
        print "watch_gstsend"
        self._changed(self, args)
        self.gstsend_timeout = gobject.timeout_add(5000, self.stop)
        return False

    def stop(self):
        print "stop: ", 
        if hasattr(self, "watched_gstrecv"):
            print "watch"
            gobject.source_remove(self.watched_gstrecv)
        if hasattr(self, "timeout"):
            print "timeout"
            gobject.source_remove(self.timeout)
            
        try:
            print "killing gstrecv: ", self.gstrecv_pid
            os.kill(self.gstrecv_pid, signal.SIGTERM)
            print "recv: before os.wait()"
            print "recv: after os.wait()"
        except:
            pass
        try:
            print "killing gstsend_pid: ", self.gstsend_pid
            os.kill(self.gstsend_pid, signal.SIGTERM)
            print "send: before os.wait()"
            os.wait()
            print "send: after os.wait()"
        except:
            pass
            
        self._changed(self, event="on_stop_gstsend")





### NETWORK ###

class Network(Colleague):
    def __init__(self, med):
        Colleague.__init__(self, med)
        self.config = med.config
        self.buf_size = 1024
        self.port = self.config.negotiationport


    def validate(self, msg):
        tmp_msg = msg.strip()
        msg = None
        if tmp_msg[0] == "{" and tmp_msg[-1] == "}" and tmp_msg.find("{",1,-2) == -1 and tmp_msg.find("}",1,-2) == -1:
            try:
                tmp_msg = eval(tmp_msg)
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
    def __init__(self, med):
        global _TEST
        Network.__init__(self, med)
        self.host = ''

        if _TEST:
            self.port = int(sys.argv[1])
            self.host = socket.gethostname()

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

        self._changed(self, (msg, addr, conn), event="on_server_rcv_command")
        return True
        


class Client(Network):
    def __init__(self, med):
        global _TEST
        Network.__init__(self, med)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(10)

        if _TEST:
            self.port = int(sys.argv[2])

    def connect(self, host, msg):
        self.host = host
        try:
            self.sock.connect((host, self.port))
        except socket.timeout, err:
            self._changed(self, event="on_client_socket_timeout")
        except socket.error, err:
            self._changed(self, (err, "err"), event="on_client_socket_error")
        else:
            if self.send(msg):
                #~ if self.widgets: ###############
                self.io_watch = gobject.io_add_watch(self.sock, gobject.IO_IN, self.handle_data)
        return False

    def send(self, msg):
        if not len(msg)%self.buf_size:
            msg += " "
        try:
            self.sock.sendall(msg)
            self._changed(self, event="on_client_add_timeout")
            return True
        except socket.error, err:
            self._changed(self, err, event="on_client_add_timeout")
            self._changed(self, "send", err, event="on_client_socket_error")
            return False

    def handle_data(self, source, condition):
        buffer = self.recv(source)
        source.close()
        msg = self.validate(buffer)
        self._changed(self, msg, event="on_client_rcv_command")
        return False



if __name__ == '__main__':
    main = Mediator()
