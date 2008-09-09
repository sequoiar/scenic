
# Callbacks 

def connection_made_cb():
    print 'Python: Connection successful'

def connection_end_cb():
    print 'Python: Connection ended'

def connection_incoming_cb():
    print 'Python: Connection incoming'

def connection_message_cb( msg ):
    print 'Python: <IM>' , msg

# End of callbacks implementation


