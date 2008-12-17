from subprocess import Popen, call
import sys
import os
import glob 

def shell_command(*popenargs, **kwargs):
    """Run command with arguments.  Wait for command to complete, then
    return the stdout.

    The arguments are the same as for the Popen constructor.  Example:

    ret = shell_command(["ls", "-l"])
    
    subprocess.call is similar, but that one doesn't return stdout, 
    only the succes or not. (0, or not)
    """
    # .communicate() returns (stdout, stderr)
    # .wait()        returns return code
    try:
        ret = Popen(*popenargs, **kwargs).communicate()[0]
    except OSError,e:
        print "OS error :",e
    return ret
    
def list_v4l_devices():
    """
    Returns list of /dev/video*
    """
    return glob.glob('/dev/video*')

class video4linux_device:
    """
    Controls one v4l device (such as /dev/video0)
    """
    def __init__(self,dev='/dev/video0'):
        self.set_device(dev)
    
    def set_device(self,dev='/dev/video0'):
        """
        Sets the v4l device file name.
        """ 
        self.dev = dev
        
    def print_attr(self):
        """
        Prints attributes that can be changed for a v4l device
        
        Calls v4lctl list
        """
        ret = shell_command(['v4lctl', '-c', self.dev, 'list'])
        print "INFO for %s:\n" % (self.dev)
        print ret
    
    def set_attr(self,attr,val):
        """
        Of course attr must be of the right type.
        """
        call(['v4lctl', '-c', self.dev, 'setattr',attr,val])
    
# attribute  | type   | current | default | possibilities | comment

if __name__ == '__main__':
    devices = list_v4l_devices()
    print "v4l devices: ", devices
    if len(devices) == 0:
        print "NO V4L device !"
    else:
        print "using ", devices[0]
        dev = video4linux_device(devices[0])
        
        print "ATTRIBUTES FOR %s :", devices[0]
        dev.print_attr()
        print "SETTING NORM TO NTSC"
        dev.set_attr('norm','NTSC')
        print "ATTRIBUTES FOR %s :", devices[0]
        dev.print_attr()
        print "SETTING NORM TO PAL"
        dev.set_attr('norm','PAL')
        print "ATTRIBUTES FOR %s :", devices[0]
        dev.print_attr()

        dev.set_attr('input','Composite0') # Composite0 Composite1 Composite2 S-Video
    

# v4l-info /dev/video0          --> show a lot of infos
# v4l2-dbg -d /dev/video0 -D    --> show driver info

"""
aalex@brrr:~$ v4l-conf -h
usage: v4l-conf  [ options ] 

options:
    -q        quiet
    -d <dpy>  X11 Display     [localhost:10.0]
    -c <dev>  video device    [/dev/video0]
    -b <n>    displays color depth is <n> bpp
    -s <n>    shift display by <n> bytes
    -f        query frame buffer device for info
    -a <addr> set framebuffer address to <addr>
              (in hex, root only, successful autodetect
               will overwrite this address)
    -1        force v4l API
    -2        force v4l2 API
"""

"""
aalex@brrr:~$ v4lctl list
attribute  | type   | current | default | comment
-----------+--------+---------+---------+-------------------------------------
norm       | choice | NTSC    | PAL     | PAL NTSC SECAM PAL-Nc PAL-M PAL-N NTSC-JP PAL-60
input      | choice | Composi | Composi | Composite0 Composite1 Composite2 S-Video
bright     | int    |   32768 |   32768 | range is 0 => 65535
contrast   | int    |   32768 |   32768 | range is 0 => 65535
color      | int    |   32768 |   32768 | range is 0 => 65535
hue        | int    |   32768 |   32768 | range is 0 => 65535
volume     | int    | -1208313824 |   65535 | range is 0 => 65535
Balance    | int    |   32768 |   32768 | range is 0 => 65535
Bass       | int    |   32768 |   32768 | range is 0 => 65535
Treble     | int    |   32768 |   32768 | range is 0 => 65535
mute       | bool   | on      | off     |
chroma agc | bool   | off     | off     |
combfilter | bool   | off     | off     |
automute   | bool   | on      | off     |
luma decim | bool   | off     | off     |
agc crush  | bool   | on      | off     |
vcr hack   | bool   | off     | off     |
whitecrush | int    |     207 |     207 | range is 0 => 255
whitecrush | int    |     127 |     127 | range is 0 => 255
uv ratio   | int    |      50 |      50 | range is 0 => 100
full luma  | bool   | off     | off     |
coring     | int    |       0 |       0 | range is 0 => 3
"""
