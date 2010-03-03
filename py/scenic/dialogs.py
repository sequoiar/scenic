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
GTK Dialogs well integrated with Twisted.
 * Error dialog
 * Yes/No dialog
"""
if __name__ == "__main__":
    from twisted.internet import gtk2reactor
    gtk2reactor.install() # has to be done before importing reactor
from twisted.internet import reactor
from twisted.internet import defer
import gtk
from scenic import glade

class GladeDialogFactory(object):
    """
    Factory for dialogs created with Glade.
    """
    def __init__(self, name, parent=None, modal=True):
        """
        @param name: Name of the dialog in glade.
        """
        self.name = name
        self._dialog = None
        self._parent = parent
        self._is_modal = modal
        self._destroyed = False
        self._terminating = False
        self._widgets_tree = None # different every time we call show()
        
    def show(self):
        """
        Creates a new dialog using the glade XML file.
        Returns a Deferred which is fired when the dialog is closed.
        @rettype: L{Deferred}
        """
        if self.exists():
            msg = "Dialog %s already exists." % (self.name)
            raise RuntimeError(msg)
        else:
            self._widgets_tree = glade.get_widgets_tree()
            self._dialog = self._widgets_tree.get_widget(self.name)
            self._dialog.connect('delete-event', self._on_delete_event)
            self._dialog.connect('destroy-event', self._on_destroy_event)
            self._dialog.set_transient_for(self._parent)
            self._dialog.set_modal(self._is_modal)
            self._destroyed = False
            self._terminating = False
            self._dialog.show()
            self._deferred = defer.Deferred()
            return self._deferred

    def exists(self):
        """
        @rettype: bool
        """
        return self._dialog is not None

    def hide(self):
        """
        Actually destroys the widget.
        """
        self._terminate(False)

    def _terminate(self, answer=None):
        """
        Calls the deferred with answer as result.
        @param answer: None or bool.
        """
        if self.exists():
            self._terminating = True
            self._deferred.callback(answer)
            if not self._destroyed:
                if self._dialog.get_property('visible'):
                    self._dialog.hide()
                self._dialog.destroy()
            self._dialog = None
        else:
            print("Dialog %s is already destroyed."  % (self.name))
        
    def _on_delete_event(self):
        if not self._terminating:
            self._terminate(False)

    def _on_destroy_event(self, widget, event):
        self._destroyed = True
        if not self._terminating:
            self._terminate(answer=False)
        return True # True to stop other handlers from being invoked for the event. False to propagate the event further.

class ConfirmDialog(GladeDialogFactory):
    """
    Could be replaced by the yes/no dialog below.
    """
    def __init__(self, parent=None):
        widget_name = "confirm_dialog"
        GladeDialogFactory.__init__(self, widget_name, parent=parent, modal=True)

    def show(self, text):
        """
        @param text: Text to display in the label.
        """
        deferred = GladeDialogFactory.show(self)
        label = self._widgets_tree.get_widget("confirm_label")
        label.set_label(text)
        self._dialog.connect('response', self._on_response_event, callback)
        return deferred

    def _on_response_event(self, widget, response_id):
        """
        Calls the deferred with True of False as a result.
        """
        result = response_id == gtk.RESPONSE_OK or response_id == gtk.RESPONSE_YES
        print("RESULT %s" % (result))
        self._terminate(result)

class InvitedDialog(GladeDialogFactory):
    def __init__(self, parent=None):
        widget_name = "invited_dialog"
        GladeDialogFactory.__init__(self, widget_name, parent=parent, modal=True)
    
    def show(self, text):
        """
        @param text: Text to display in the label.
        """
        deferred = GladeDialogFactory.show(self)
        label = self._widgets_tree.get_widget("invited_dialog_label")
        label.set_label(text)
        self._dialog.connect('response', self._on_response_event)
        return deferred

    def _on_response_event(self, widget, response_id):
        """
        Calls the deferred with True of False as a result.
        """
        result = response_id == gtk.RESPONSE_OK or response_id == gtk.RESPONSE_YES
        print("RESULT %s" % (result))
        self._terminate(result)

class ErrorDialog(object):
    """
    Error dialog. Fires the deferred given to it once done.
    Use the create static method as a factory.
    """
    def __init__(self, deferred, message, parent=None, details=None, timeout=300): # timeout is 5 minutes
        """
        @param deferred: L{Deferred}
        @param message: str or unicode
        @param details: str or unicode.
        @param timeout: lifetime in seconds. If set to None, disables it.
        """
        #FIXME: error_dialog should be an attribute of this class
        self.deferredResult = deferred
        error_dialog = gtk.MessageDialog(
            parent=parent, 
            flags=0, 
            type=gtk.MESSAGE_ERROR, 
            buttons=gtk.BUTTONS_CLOSE, 
            message_format=message)
        if details is not None:
            error_dialog.vbox.set_spacing(14)
            # TODO: i18n !
            expander = gtk.expander_new_with_mnemonic("Show more details")
            expander.set_spacing(6)
            details_label = gtk.Label()
            details_label.set_text(details)
            expander.add(details_label)
            error_dialog.vbox.pack_start(expander, False, False)
            print("Added details in the error dialog: %s" % (details))
        self._delayed_id = None
        if timeout is not None:
            self._delayed_id = reactor.callLater(timeout, self._timeout, error_dialog)
        error_dialog.set_modal(True)
        error_dialog.connect("close", self.on_close)
        error_dialog.connect("response", self.on_response)
        error_dialog.show_all()

    def _timeout(self, error_dialog):
        print("error dialog timeout. Closing it.")
        self.terminate(error_dialog)

    @staticmethod
    def create(message, parent=None, details=None):
        """
        Returns a Deferred which will be called with a True result.
        @param message: str
        @rettype: L{Deferred}
        """
        d = defer.Deferred()
        dialog = ErrorDialog(d, message, parent, details)
        return d

    def on_close(self, dialog, *params):
        print("on_close %s %s" % (dialog, params))

    def on_response(self, dialog, response_id, *params):
        #print("on_response %s %s %s" % (dialog, response_id, params))
        if response_id == gtk.RESPONSE_DELETE_EVENT:
            print("Deleted")
        elif response_id == gtk.RESPONSE_CANCEL:
            print("Cancelled")
        elif response_id == gtk.RESPONSE_OK:
            print("Accepted")
        self.terminate(dialog)

    def terminate(self, dialog):
        if self._delayed_id is not None:
            if self._delayed_id.active():
                self._delayed_id.cancel()
        dialog.destroy()
        self.deferredResult.callback(True)

class YesNoDialog(object):
    """
    Yes/no confirmation dialog.
    Use the create static method as a factory.
    """
    def __init__(self, deferred, message, parent=None):
        self.deferredResult = deferred
        yes_no_dialog = gtk.MessageDialog(
            parent=parent, 
            flags=0, 
            type=gtk.MESSAGE_QUESTION, 
            buttons=gtk.BUTTONS_YES_NO, 
            message_format=message)
        yes_no_dialog.set_modal(True)
        yes_no_dialog.connect("close", self.on_close)
        yes_no_dialog.connect("response", self.on_response)
        yes_no_dialog.show()

    @staticmethod
    def create(message, parent=None):
        """
        Returns a Deferred which will be called with a boolean result.
        @param message: str
        @rettype: L{Deferred}
        """
        d = defer.Deferred()
        dialog = YesNoDialog(d, message, parent)
        return d

    def on_close(self, dialog, *params):
        print("on_close %s %s" % (dialog, params))

    def on_response(self, dialog, response_id, *params):
        print("on_response %s %s %s" % (dialog, response_id, params))
        if response_id == gtk.RESPONSE_DELETE_EVENT:
            print("Deleted")
            self.terminate(dialog, False)
        elif response_id == gtk.RESPONSE_NO:
            print("Cancelled")
            self.terminate(dialog, False)
        elif response_id == gtk.RESPONSE_YES:
            print("Accepted")
            self.terminate(dialog, True)

    def terminate(self, dialog, answer):
        dialog.destroy()
        self.deferredResult.callback(answer)


if __name__ == '__main__': 
    d = ErrorDialog.create('BOBBBBBBBBBB')
    d.addCallback(lambda result: reactor.stop())
    reactor.run()
