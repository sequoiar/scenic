#!/usr/bin/env python
import os
import pprint
from twisted.internet import utils
from twisted.internet import defer
from twisted.python import procutils
from twisted.internet import reactor

TESTDATA = """Ver:0.3.6
INFO:Built on Feb 10 2010 at 10:17:35

DC1394 Camera 0: Unibrain Fire-i 1.2
GUID = 814436102630ee6
Supported modes :
    640x480_MONO8 (vmode 69)
    Framerates: 3.75,7.5,15,30
    640x480_RGB8 (vmode 68)
    Framerates: 3.75,7.5,15
    640x480_YUV422 (vmode 67)
    Framerates: 3.75,7.5,15
    640x480_YUV411 (vmode 66)
    Framerates: 3.75,7.5,15,30
    320x240_YUV422 (vmode 65)
    Framerates: 3.75,7.5,15,30
    160x120_YUV444 (vmode 64)
    Framerates: 7.5,15,30

Video4Linux Camera /dev/video1:
    Driver name   : 
    Card type     : 
    Bus info      : 
    Driver version: 0
    Video input   : 
    Standard      : 
    Width/Height  : 0x0
    Pixel Format  : 
    Capture Type  : 1
    Field         : Any
    Bytes per Line: 0
    Size Image    : 0
    Colorspace    : Unknown (00000000)
    Format 924x576 not supported
    Format 768x480 not supported
    Format 720x480 not supported
    Format 704x480 not supported
    Format 704x240 not supported
    Format 640x480 not supported
    Format 352x240 not supported
    Format 320x240 not supported
    Format 176x120 not supported
WARNING:Format 0x-1222839056not reverted correctly

Video4Linux Camera /dev/video0:
    Driver name   : bttv
    Card type     : BT878 video (Osprey 210/220/230
    Bus info      : PCI:0000:05:00.0
    Driver version: 2321
    Video input   : 0 (Composite0)
    Standard      : PAL
    Width/Height  : 640x480
    Pixel Format  : BGR3
    Capture Type  : 1
    Field         : Interlaced
    Bytes per Line: 1920
    Size Image    : 921600
    Colorspace    : Unknown (00000000)
    Format 924x576 supported
    Format 768x480 supported
    Format 720x480 supported
    Format 704x480 supported
    Format 704x240 supported
    Format 640x480 supported
    Format 352x240 supported
    Format 320x240 supported
    Format 176x120 supported
Exitting Milhouse
"""


def _parse_milhouse_list_cameras(text):
    v4l2_devices = []
    currently_parsed_is_v4l2 = False
    for line in text.splitlines():
        line = line.strip()
        print("line:" + line)
        if line.startswith('Video4Linux Camera'):
            name = line.split()[2].split(":")[0]
            #print "  name", name
            v4l2_devices.append({
                "name": name,
                "size": None,
                "standard": None,
                "is_interlaced": False,
                "input": None,
                "supported_sizes": []
                })
            currently_parsed_is_v4l2 = True
        elif line.startswith("DC1394 Camera"):
            currently_parsed_is_v4l2 = False
        # TODO: know if currently parsed is a V4L 1
        elif currently_parsed_is_v4l2:
            if line.startswith("Standard"):
                try:
                    standard = line.split(":")[1].strip()
                except IndexError:
                    standard = None
                else:
                    if standard == '':
                        standard = None
                    v4l2_devices[-1]["standard"] = standard
                    print "  standard:", standard
            elif line.startswith("Width/Height"):
                size = line.split(":")[1].strip()
                v4l2_devices[-1]["size"] = size
                print "  size:", size
            elif line.startswith("Format"):
                if line.find("not") == -1:
                    pass
                else:
                    size = line.split(" ")[1].strip()
                    v4l2_devices[-1]["supported_sizes"].append(size)
                    print "  adding supported_size:", size
            elif line.startswith("Field"):
                is_interlaced = line.split(":")[1].strip() == "Interlaced"
                v4l2_devices[-1]["is_interlaced"] = is_interlaced
                print "  interlaced:", is_interlaced
            elif line.startswith("Video input"):
                try:
                    input = line.split(":")[1].split(" ")[0]
                except IndexError:
                    input = None
                else:
                    try:
                        input = int(input)
                    except ValueError:
                        input = None
                    else:
                        print "  input", input
                        v4l2_devices[-1]["input"] = input
    #print v4l2_devices
    return v4l2_devices

def list_cameras():
    """
    Calls the Deferred with the list of device names as argument. 
    
    @rettype: Deferred
    """
    def _cb(text, deferred):
        print text
        ret = _parse_milhouse_list_cameras(text)
        deferred.callback(ret)
        
    def _eb(reason, deferred):
        deferred.errback(reason)
        print(reason)
    
    command_name = "milhouse"
    args = ['--list-cameras']
    try:
        executable = procutils.which(command_name)[0] # gets the executable
    except IndexError:
        return defer.fail(RuntimeError("Could not find command %s" % (command_name)))
    deferred = defer.Deferred()
    d = utils.getProcessOutput(executable, args=args, env=os.environ)
    d.addCallback(_cb, deferred)
    d.addErrback(_eb, deferred)
    return deferred


#if __name__ == "__main__":
#    def _go():
#        def _cb(result):
#            reactor.stop()
#        def _eb(reason):
#            print(reason)
#            return None
#        d = list_cameras()
#        d.addCallback(_cb)
#
#    reactor.callLater(0, _go)
#    reactor.run()


if __name__ == "__main__":
    pprint.pprint(_parse_milhouse_list_cameras(TESTDATA))

