import libsip_export as libsip

# Callbacks 

def connection_made_cb():
    print 'Python: Connection successful'

def connection_end_cb():
    print 'Python: Connection ended'

def connection_incoming_cb():
    print 'Python: Connection incoming'

# End of callbacks implementation

class Connection:
    """ Class to make transparent the use the C++ pjsip implementation
    """
    
    def __init__(self, port):
        self.userAgent = libsip.SIPSession( port )

    # connection API

    def connect(self):
        res = self.userAgent.connect()
        return res

    def disconnect(self):
        res = self.userAgent.disconnect()
        return res

    def state(self):
        print self.userAgent.getConnectionState()

    def error(self):
        print self.UserAgent.getErrorReason()

