#!/usr/bin/env python

import sip_export

uac = sip_export.SIPSession(50060)
uac.addMedia("a=GSM/PCMU/:12345")
#uac.connect("<sip:bloup@192.168.1.230:5060>")
#uac.disconnect()
