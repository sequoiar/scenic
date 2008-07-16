from ctypes import *

# Adapt the path
libsip = cdll.LoadLibrary("/home/emmanuel/dev/propulseART/miville/trunk/inhouse/prototypes/ctypes-python/libsip.so")
libsip.test()
