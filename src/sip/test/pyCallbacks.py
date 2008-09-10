
# Callbacks 

def connection_made_cb():
    """ Callback when an invite session has been successfully established 
    """
    result = 'Python: Connection successful'
    print result
    return result

def connection_end_cb():
    """ Callback when an invite session has been successfully terminated 
    """
    print 'Python: Connection ended'

def connection_incoming_cb():
    """ Callback when an invite session request is incoming 
    """
    print 'Python: Connection incoming'

def connection_message_cb( msg ):
    """ Callback when an instant message has been received
    """
    print 'Python: <IM>' , msg

def connection_failed_cb( reason ):
    """ Callback when an invite session could not have been established
        The reason of the connection failure is received as an argument
    """
    print 'Python: Connection failed:', reason

def connection_media_choice_cb( codec ):
    """ Callback when the sdp negociation has been successfully done
        The compatible codec is between the two peers is passed as an argument 
    """
    print 'Python: SDP negociation done:', codec

# End of callbacks implementation


