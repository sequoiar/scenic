#TODO: remove this from __init__
#TODO: rename this file as common.
# states common to all streams
STATE_IDLE = "idle"
STATE_STARTING = "starting"
STATE_STREAMING = "streaming"
STATE_STOPPED = "stopped"
STATE_FAILED = "failed"

class StreamError(Exception):
    """
    Any error thrown by a stream of service.
    """
    pass

