#!/usr/bin/env python 
# -*- coding: utf-8 -*-
"""
Session description classes.
Those are typically turned into dict and sent to the remote agent for 
negociation.

"""
import time
import warnings
try:
    import json # python 2.6
except ImportError:
    import simplejson as json # python 2.4 to 2.5
try:
    _tmp = json.loads
except AttributeError:
    warnings.warn("Use simplejson, not the old json module.")
    sys.modules.pop('json') # get rid of the bad json module
    import simplejson as json
# ------------------ Session infos. --------
# agent roles
ROLE_OFFERER = "offerer" # alice
ROLE_ANSWERER = "answerer" # bob
# directions
DIRECTION_TO_OFFERER = "TO_OFFERER"
DIRECTION_TO_ANSWERER = "TO_ANSWERER"
# streamers
STREAMER_SENDER = "send"
STREAMER_RECEIVER = "recv"
# states for session, streams, streamers.
STATE_IDLE = 0
STATE_STARTING = 1
STATE_STREAMING = 2
STATE_STOPPING = 3
STATE_STOPPED = 4
STATE_ERROR = 5

# --------------------- SSIP protocol details ---------
# details
PROTOCOL_VERSION = "0.1"
PROTOCOL_NAME = "SSIP" # simple session initialization protocol
# parsing stuff
ASSIGNATION_OPERATOR = ":"
# requests / methods of the SSIP protocol
REQUEST_INVITE = "INVITE"
REQUEST_OK = "OK"
REQUEST_ACK = "ACK"
REQUEST_BYE = "BYE"
REQUEST_REFUSE = "REFUSE"
REQUEST_ERROR = "ERROR"
_allowed_methods = [REQUEST_ERROR, REQUEST_REFUSE, REQUEST_BYE, REQUEST_ACK, REQUEST_OK, REQUEST_INVITE]

# ------------------- Requests attributes ----------
ATTR_SESSION_ID = "Session-ID"
ATTR_ERROR_CODE = "Error-Code"
ATTR_ALLOW = "Allow"
ATTR_METHOD_ANSWERED = "Method-Answered" 
ATTR_CONFIG_ENTRIES = "Configuration-Entries"
ATTR_COMMENT = "Comment"
ATTR_PROTOCOL_VERSION = "Protocol-Version"
_values_types = {
    ATTR_SESSION_ID:int,
    ATTR_PROTOCOL_VERSION:str,
    ATTR_ERROR_CODE:int,
    ATTR_ALLOW:list,
    ATTR_METHOD_ANSWERED:str,
    ATTR_COMMENT:str,
    ATTR_CONFIG_ENTRIES:dict,
    }

#--------------------- Requests Codes --------------
# 2xx Success
CODE_SUCCESS_OK = 200
# refuse
CODE_REFUSE_BUSY_HERE = 486
# error
CODE_REFUSE_DECLINE = 603
# 5xx Server Errors
CODE_ERROR_INTERNAL_SERVER = 500
CODE_ERROR_NOT_IMPLEMENTED = 501
CODE_ERROR_SERVICE_NOT_AVAILABLE = 503
CODE_ERROR_SSIP_VERSION_UNSUPPORTED = 505
# 4xx Client Errors
CODE_ERROR_BAD_REQUEST = 400
CODE_ERROR_UNSUPPORTED_MEDIA_TYPE = 415

def serialize(data):
    """
    Python dict/list/int/str/float to text.
    """
    try:
        txt = json.dumps(data)
    except TypeError, e:
        raise ParsingError("Error trying to encode to a JSON serialized object.")
    return txt

def unserialize(txt):
    """
    Text to Python dict/list/int/str/float.
    """
    try:
        data = json.loads(txt)
    except ValueError, e:
        raise ParsingError("Error trying to decode JSON serialized object.")
    return data

class SessionError(Exception):
    """
    Any error the SimpleSessionInitiationProtocol can raise.
    """
    pass

class ParsingError(Exception):
    """
    Exception raised when errors occur attempting to parse a message.
    """
    pass

class SimpleRequest(object):
    def __init__(self, method=None):
        self.method = method
        self.values = {}


def text_to_request(text=""):
    """
    Parses SSIP text and returns a Request object
    """
    global _values_types
    req = SimpleRequest()
    first_line = True
    for line in text.splitlines():
        line = line.strip()
        if first_line:
            req.method = line # str
            first_line = False
        else:
            pos = line.find(ASSIGNATION_OPERATOR)
            if pos == -1:
                raise ParsingError("Assignation operator not found in '%s'." % (line))
            key = line[:pos].strip()
            value = line[pos+1:].strip()
            if value == "":
                raise ParsingError("Value is empty for key '%s'" % (key))
            cast = None
            try:
                cast = _values_types[key]
            except KeyError, e:
                raise ParsingError("This key named '%s' is not known." % (key))
            if cast in [int, str, float]:
                res = cast(value)
            else: # list or dict
                res = unserialize(value)
            req.values[key] = res
    return req

def request_to_text(request):
    """
    Turns a SimpleRequest instance into a string SSIP message.
    """
    global _values_types
    txt = ""
    if request.method is None:
        raise ParsingError("Every request must have a method.")
    if request.method not in _allowed_methods:
        raise ParsingError("The method '%s' is not supported." % (request.method))
    
    txt += "%s\n" % (request.method)
    for key, value in request.values.iteritems():
        try:
            cast = _values_types[key]
        except KeyError, e:
            raise ParsingError("This key name is not known.")
        if cast in [int, str, float]:
            res = cast(value) # just checking the type
        else: # list or dict
            res = serialize(value)
        txt += "%s : %s\n" % (key, res)
    return txt

def get_current_utc_time():
    """
    Returns a timestamp in UTC time.
    
    Important : this relies on the computer internal clock and time zone.
    """
    return time.time() - time.timezone

def utc_time_to_local(stamp):
    """
    Converts UTC timestamp to local time zone timestamp.
    You can then compare it the time.time()

    Important : relies on the computer time zone.
    """
    return stamp + time.timezone

class SessionDescription(object):
    """
    We need to wrap the informations about the current session in an object.
    This will be much easier to manager.
    Contains stuff that is negociated between the peers.
    """
    # Not used yet.
    def __init__(self, session_id=None, time_started=None, contact_infos=None, offerer_entries=None, answerer_entries=None, role=ROLE_OFFERER):
        """
        :param contact_infos: ContactInfos object
        :alice_entries: Dict of configuration entries for initiator peer.
        :bob_entries: Dict of configuration entries for the othe peer.
        :param role: str Either "offerer" or "answerer"
        """
        #We could classify them as local and remote instead of offerer and answerer
        self.contact_infos = contact_infos
        self.stream_to_offerer = StreamDescription(direction=DIRECTION_TO_OFFERER, entries=offerer_entries)
        self.stream_to_answerer = StreamDescription(direction=DIRECTION_TO_ANSWERER, entries=answerer_entries)
        self.role = role # the role if this agent. Is this miville offerer or answerer ?
        self.time_started = None # UTC timestamp
        self.stream_state = STATE_IDLE
        self.comment = ""
        self.session_id = None
        # init stuff
        if time_started is None:
            self.time_started = get_current_utc_time()
        else:
            self.time_started = time_started
        if session_id is not None:
            self.session_id = session_id
        else:
            self.session_id = int(self.time_started)

class StreamDescription(object):
    """
    Description of a stream as passed using the Streaming Session Protocol.
    """
    def __init__(self, name="", direction=DIRECTION_TO_ANSWERER, entries=None):
        """
        :param entries: dict
        """
        self.entries = entries # dict or None
        self.name = name # a profile name ?
        self.state = STATE_IDLE
        self.direction = direction
        # every stream has a sender and a receiver !
        self.sender_streamer = StreamerDescription(STREAMER_SENDER)
        self.receiver_streamer = StreamerDescription(STREAMER_RECEIVER)

class StreamerDescription(object):
    """
    A Streamer is a sender or receiver, as described in the session
    For instance, a Milhouse stream has a sender and a receiver, 
    typically ran by two different agents (miville) on two separate hosts.
    
    This describes the state of each stream of each agent.
    """
    def __init__(self, role=STREAMER_SENDER):
        self.role = role
        self.state = self.STATE_STOPPED
        self.error = None
        self.details = "" # detailed error message

