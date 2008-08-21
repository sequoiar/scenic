#!/usr/bin/env python

import sip_export

uas = sip_export.SIPSession(5060)
uas.addMedia("a=GSM/PCMU/:12345")
#uas.startMainloop()
#uas.disconnect()
