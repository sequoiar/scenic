#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Copyright (c) 2009, Atzm WATANABE, Simon Piette
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

# $Id: tvctl.py,v 1.11 2008/09/12 14:05:22 atzm Exp $
# Based on tvctl.py from atzm
# http://diary.atzm.org/20080912.html


import os.path
import glob
import commands
import gobject
import gtk

from ConfigParser import SafeConfigParser, NoSectionError, NoOptionError
from cStringIO import StringIO

CMD_CH_FMT = '/usr/bin/ivtv-tune -d %s -t %s -c %d > /dev/null 2>&1'
CMD_BL_FMT = '/usr/bin/v4l2-ctl -d %s -t %s > /dev/null 2>&1'
CMD_IN_FMT = '/usr/bin/v4l2-ctl -d %s --set-input %d > /dev/null 2>&1'
CMD_CTRL_SET_FMT = '/usr/bin/v4l2-ctl -d %s -c %s=%d'
CMD_CTRL_GET_FMT = "/usr/bin/v4l2-ctl -d %s -C %s | awk -F': ' '{print $2}'"
CMD_STND_SET_FMT = '/usr/bin/v4l2-ctl -d %s -s %d'
CMD_STND_GET_FMT = "/usr/bin/v4l2-ctl -d %s -S | head -1 | awk -F= '{print $2}'"
CMD_STNDLST_GET_FMT = '/usr/bin/v4l2-ctl --list-standards'
CMD_INPUTLST_GET_FMT= '/usr/bin/v4l2-ctl --list-inputs'
CMD_INPUT_GET_FMT= "/usr/bin/v4l2-ctl -d %s --get-input | awk '{print $4}'"
CMD_INPUT_SET_FMT= '/usr/bin/v4l2-ctl -d %s --set-input=%s'

config = SafeConfigParser()
config.readfp(
    StringIO("""
[gui]
channel_max:   12
channel_width: 3
""")
)
config.read( os.path.expanduser('~/.tvctl') )

def v4l2_ctl_get(*args):
    return commands.getoutput( CMD_CTRL_GET_FMT % args )

def v4l2_ctl_set(*args):
    return commands.getoutput( CMD_CTRL_SET_FMT % args )

def get_standards():
    val   = commands.getoutput(CMD_STNDLST_GET_FMT)
    _list = [ v.split('\n') for v in val.split('\n\n') ]

    for item in _list:
        _index = _id = _name = None

        for i in item:
            key, value = [ v.strip() for v in i.split(':') ]

            # index is zero origin :)
            if key == 'index':
                _index = int(value)
            elif key == 'ID':
                _id    = int( value, 16 )
            elif key == 'Name':
                _name  = value

        _list[_index] = ( _id, _name )

    return _list

def get_inputs():
    val   = commands.getoutput(CMD_INPUTLST_GET_FMT)
    _list = [ v.split('\n') for v in val.split('\n\n') ]

    for item in _list:
        _index = _name = _status = None

        for i in item:
            key, value = [ v.strip() for v in i.split(':') ]

            # index is zero origin :)
            if key == 'Input':
                _index = int(value)
            elif key == 'Name':
                _name  = str(value)

        _list[_index] = ( _index, _name, )

    return _list

class ChannelTable(gtk.Frame):
    def __init__( self, device='/dev/video0',
                  max_ch=config.getint('gui', 'channel_max'),
                  width=config.getint('gui', 'channel_width') ):

        gtk.Frame.__init__( self, label='Channels' )
        self.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
        self.set_border_width(3)

        self._tooltips = gtk.Tooltips()
        self._table    = gtk.Table( width, max_ch / width )
        self._device   = device
        self._buttons  = {}

        self.add(self._table)

        _prev_radio_button = None

        for _ch in range(max_ch):
            ch  = _ch + 1
            col = _ch % width
            row = _ch / width
            ch  = str(ch)

            b = gtk.RadioButton( group=_prev_radio_button, label=ch )
            b.set_mode(False)
            b.connect( 'toggled', self._button_toggled, ch )

            try:
                tip = config.get( 'channel_alias', ch )
                self._tooltips.set_tip( b, tip )
            except ( NoSectionError, NoOptionError ):
                pass

            self._buttons[ch] = _prev_radio_button = b
            self._table.attach( b,
                                col, col + 1,
                                row, row + 1,
                                gtk.FILL, gtk.FILL,
                                2, 2 )

    def _button_toggled( self, button, ch ):
        if not button.get_active():
            return True

        freq = 'japan-bcast'
        if config.has_option( 'channel_assign', ch ):
            freq, ch = config.get( 'channel_assign', ch ).split()

        os.system( CMD_CH_FMT % ( self._device, freq, int(ch) ) )

class BilingualTable(gtk.Frame):
    def __init__( self, device='/dev/video0' ):
        gtk.Frame.__init__( self, label='Bilingual' )
        self.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
        self.set_border_width(3)

        self._vbox    = gtk.VBox( True, 2 )
        self._device  = device
        self._buttons = {}
        self._table   = {
            '主音声': 'mono',
            '主+副':  'stereo',
            '副音声': 'lang2'
        }
        self.add(self._vbox)

        _prev_radio_button = None

        for l, cmd_arg in self._table.iteritems():
            b = gtk.RadioButton( group=_prev_radio_button, label=l )
            b.set_mode(False)
            b.connect( 'toggled', self._button_toggled, cmd_arg )
            self._buttons[l] = _prev_radio_button = b
            self._vbox.pack_start(b)

    def _button_toggled( self, button, cmd_arg ):
        if not button.get_active():
            return True
        os.system( CMD_BL_FMT % ( self._device, cmd_arg ) )


class ControlsTable(gtk.Frame):
    def __init__( self, device='/dev/video0' ):
        gtk.Frame.__init__( self, label='Controls' )
        self.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
        self.set_border_width(3)

        self._vbox    = gtk.VBox( True, 3 )
        self._device  = device
        self._buttons = {}
        self._table   = {
            'mute': 'Mute',
            'chroma_agc': 'Chroma AGC',
            'combfilter': 'Comb Filter',
            'automute': 'Automute',
            'luma_decimation_filter': 'Luma Decimation Filter',
            'agc_crush': 'AGC Crush',
            'vcr_hack': 'VCR hack',
            'full_luma_range': 'Full Luma Range',
        }
        self.add(self._vbox)
    
        for control, label in sorted(self._table.iteritems()):
            toggle = gtk.CheckButton(label)
            if (v4l2_ctl_get(self._device, control) == "1"):
                toggle.set_active(True)
            else:
                toggle.set_active(False)
            toggle.connect( "toggled", self._button_toggled, control)
            self._vbox.pack_start(toggle)

    def _button_toggled( self, widget, control=None ):
        if widget.get_active():
            v4l2_ctl_set( self._device, control, 1)
        else:
            v4l2_ctl_set( self._device, control, 0)


class InputTable(gtk.Frame):
    def __init__( self, device='/dev/video0' ):
        gtk.Frame.__init__( self, label='Input' )
        self.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
        self.set_border_width(3)

        self._vbox    = gtk.VBox( True, 3 )
        self._device  = device
        self._buttons = {}
        self._table   = {
            'Composite 0':  0,
            'Composite 1':  1,
            'Composite 2':  2,
            'S-Video': 3,
        }

        self.add(self._vbox)

        _prev_radio_button = None

        #for l, cmd_arg in sorted(self._table.iteritems()):
        for (index, name ) in get_inputs():
            b = gtk.RadioButton( group=_prev_radio_button, label=name )
            b.set_mode(False)
            b.connect( 'toggled', self._button_toggled, index )
            self._buttons[name] = _prev_radio_button = b
            self._vbox.pack_start(b)

    def _button_toggled( self, button, cmd_arg ):
        if not button.get_active():
            return True
        os.system( CMD_IN_FMT % ( self._device, cmd_arg ) )

class StandardsComboBox(gtk.ComboBox):
    def __init__( self, device='/dev/video0' ):
        self._device     = device
        self._standards  = get_standards()
        self._list_store = gtk.ListStore( gobject.TYPE_STRING )
        self._cell       = gtk.CellRendererText()

        gtk.ComboBox.__init__( self, self._list_store )
        self.pack_start( self._cell, True )
        self.add_attribute( self._cell, 'text', 0 )

        for _id, _name in self._standards:
            self.append_text(_name)

        self._set_active_by_id( int( commands.getoutput( CMD_STND_GET_FMT % self._device ), 16 ) )
        self.connect( 'changed', self._changed )

    def _set_active_by_id( self, _id ):
        for i in range( len( self._standards ) ):
            if self._standards[i][0] == _id:
                self.set_active(i)
                break

    def _changed( self, combo_box ):
        os.system( CMD_STND_SET_FMT % ( self._device, combo_box.get_active() ) )

class InputComboBox(gtk.ComboBox):
    def __init__( self, device='/dev/video0' ):
        self._device     = device
        self._inputs  = get_inputs()
        self._list_store = gtk.ListStore( gobject.TYPE_STRING )
        self._cell       = gtk.CellRendererText()

        gtk.ComboBox.__init__( self, self._list_store )
        self.pack_start( self._cell, True )
        self.add_attribute( self._cell, 'text', 0 )

        for _id, _name in self._inputs:
            self.append_text(_name)

        self._set_active_by_id( int( commands.getoutput( CMD_INPUT_GET_FMT % self._device ), 16 ) )
        self.connect( 'changed', self._changed )

    def _set_active_by_id( self, _id ):
        for i in range( len( self._inputs ) ):
            if self._inputs[i][0] == _id:
                self.set_active(i)
                break

    def _changed( self, combo_box ):
        os.system( CMD_INPUT_SET_FMT % ( self._device, combo_box.get_active() ) )

class V4L2ControlScale(gtk.Frame):
    def __init__( self, label, ctrl, max_val, device='/dev/video0' ):
        self._device  = device
        self._label   = label
        self._ctrl    = ctrl
        self._max_val = max_val

        gtk.Frame.__init__( self, label=self._label )
        self.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
        self.set_border_width(3)

        current = v4l2_ctl_get( self._device, self._ctrl )
        default = int( float(current) / self._max_val * 100 )

        self._adj = gtk.Adjustment( default, 0, 100, 1, 10, 0 )
        self._adj.connect( 'value-changed', self._adj_value_changed )

        self._hscale = gtk.HScale(self._adj)
        self._hscale.set_value_pos(gtk.POS_RIGHT)
        self._hscale.set_digits(0)

        self.hbox = gtk.VBox( False, 0 )
        self.hbox.pack_start(self._hscale)

        self.add(self.hbox)

    def _adj_value_changed( self, adj ):
        val = self._max_val * ( adj.get_value() / 100 )
        return v4l2_ctl_set( self._device, self._ctrl, val )

class VolumeScale(V4L2ControlScale):
    def __init__( self, device='/dev/video0' ):
        V4L2ControlScale.__init__( self, device=device, label='Volume',
                                   ctrl='volume', max_val=65535 )

        mute = bool( int( v4l2_ctl_get( self._device, 'mute' ) ) )

        self._mute_button = gtk.CheckButton( label='Mute' )
        self._mute_button.set_active(mute)
        self._mute_button.connect( 'toggled', self._button_toggled )
        self.get_child().pack_start(self._mute_button)

    def _button_toggled( self, button ):
        mute = int( button.get_active() )
        v4l2_ctl_set( self._device, 'mute', mute )

class DeviceNotebook(gtk.Notebook):
    def __init__(self):
        gtk.Notebook.__init__(self)
        self._devices = glob.glob('/dev/video?')
        self._devices.sort()

        for d in self._devices:
            hbox = gtk.HBox( False, 4 )
            hbox.pack_start( InputComboBox( device=d ) )

            standards  = StandardsComboBox( device=d )
            volume     = VolumeScale( device=d )
            controls     = ControlsTable( device=d )
            balance    = V4L2ControlScale( device=d, label='Balance',
                                           ctrl='balance', max_val=65535 )
            brightness = V4L2ControlScale( device=d, label='Brightness',
                                           ctrl='brightness', max_val=65535 )
            contrast   = V4L2ControlScale( device=d, label='Contrast',
                                           ctrl='contrast', max_val=65535 )
            saturation   = V4L2ControlScale( device=d, label='Saturation',
                                           ctrl='saturation', max_val=65535 )
            hue   = V4L2ControlScale( device=d, label='Hue',
                                           ctrl='hue', max_val=65535 )

            whitecrush_upper = V4L2ControlScale( device=d,
                                    label='Whitecrush Upper',
                                    ctrl='whitecrush_upper', max_val=255 )
            whitecrush_lower = V4L2ControlScale( device=d,
                                    label='Whitecrush Lower',
                                    ctrl='whitecrush_lower', max_val=255 )
            coring   = V4L2ControlScale( device=d, label='Coring',
                                           ctrl='coring', max_val=3 )

            uv_ratio   = V4L2ControlScale( device=d, label='UV Ratio',
                                           ctrl='uv_ratio', max_val=100 )

            vbox = gtk.VBox( False, 0 )
            vbox.pack_start(hbox)
            vbox.pack_start(standards)
            vbox.pack_start(controls)
            #vbox.pack_start(volume)
            #vbox.pack_start(balance)
            vbox.pack_start(brightness)
            vbox.pack_start(contrast)
            vbox.pack_start(saturation)
            vbox.pack_start(hue)
            vbox.pack_start(whitecrush_upper)
            vbox.pack_start(whitecrush_lower)
            vbox.pack_start(coring)
            vbox.pack_start(uv_ratio)


            self.append_page( vbox, gtk.Label(os.path.basename(d)) )

def main():
    notebook = DeviceNotebook()

    window = gtk.Window()
    window.set_title('Video4Linux Controls')
    window.connect( 'destroy', lambda w: gtk.main_quit() )

    window.add(notebook)
    window.show_all()
    gtk.main()

if __name__ == '__main__':
    main()
