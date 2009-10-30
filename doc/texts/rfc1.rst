RFC 1 - The Miville pseudo-SIP Protocol
----------------------------------------
An offer/answer model with session description protocol using python basic types. 

Introduction
------------
In this case, we only think about unicast sessions. 


Definitions
-----------

 * agent : answere or offerer. (single Miville software running)
 * answerer : (Bob)
 * offerer : (Alice)
 * media stream : a single media stream, which has a sender and a receiver.
 * offer, answer : the messages themselves


Protocol Operation
------------------
The offerer (Alice) must create a SessionDescription object. This SessionDescription can be serialized to a Python dict using the PseudoSessionInitialisationProtocol. This dict is then sent to the remote host using the com_chan, an implementation of the Twisted Perspective Broker. 

 1. The offerer sends a offer. (INVITE) 
 2. The answerer can answer (OK) or reject (REJECT). 
 3. The offerer send an acknowledgement (ACK) and the streams start.
 4. When any of the agents want to halt the session, it tells it (BYE).
 5. The other agent(s) confirms the halting. (OK)

At anytime, any agent MAY generate a new offer, but MUST NOT do it if he hasn't answered an already existing offer. 


Requirements and Recommendations
--------------------------------
The session description MUST include : 

 * Session name ("default")
 * A session ID. (used to share a handle to many infos)
 * Session purpose. An optional comment. ("")
 * The UTC time it started
 * A list of available ports... and the the answerer chooses.
 * The list of media descriptions in the session
   For each media stream, a dict of configuration entries in the OSC-path style 
   provide informations regarding : 

  * The stream service name (milhouse) Type of the media ("audio", "video", "midi", etc.)
  * IPv4 addresses
  * Port numbers (MUST be between 10000 and 65536, with 10 minimum interval)
  * Formats ("mpeg4", etc.)
  * Bitrate (not mandatory)

 * Contact infos for each agent

  * IPv4 addresses
  * Communication channel port number
  * Contact name as seen by each. (How Bob names Alice, how Alice names Bob)

 * The name of the profile 

  * And maybe the list of settings names ?

 * MUST include the state code (integer + message) of every stream for Alice and Bob.  (each stream has a source and sink, each located on each agent.)

Ports Negociation
-----------------
If the answerer (Bob) doesn't want to start these streams, he can answer with REJECT.

If the answerer wants 

Lots of codes
-------------------
SIP/2.0:
Code    Meaning
1xx Informational -- request received, continuing to process the request
2xx Success -- the action was successfully received, understood, and accepted
3xx Redirection -- further action needs to be taken in order to complete the request
4xx Client Error -- the request contains bad syntax or cannot be fulfilled at this server
5xx Server Error -- the server failed to fulfill an apparently valid request
6xx Global Failure -- the request cannot be fulfilled at any server.
Code    Meaning
1xx Informational
100 Trying
180 Ringing
181 Call Is Being Forwarded
182 Queued
2xx Success
200 OK
3xx Redirection
300 Multiple Choices
301 Moved Permanently
302 Moved Temporarily
303 See Other
305 Use Proxy
380 Alternative Service
4xx Client Error
400 Bad Request
401 Unauthorized
402 Payment Required
403 Forbidden
404 Not Found
405 Method Not Allowed
406 Not Acceptable
407 Proxy Authentication Required
408 Request Timeout
409 Conflict
410 Gone
411 Length Required
413 Request Entity Too Large
414 Request-URI Too Large
415 Unsupported Media Type
420 Bad Extension
480 Temporarily not available
481 Call Leg/Transaction Does Not Exist
482 Loop Detected
483 Too Many Hops
484 Address Incomplete
485 Ambiguous
486 Busy Here
5xx Server Error
500 Internal Server Error
501 Not Implemented
502 Bad Gateway
503 Service Unavailable
504 Gateway Time-out
505 SIP Version not supported
6xx General Error
600 Busy Everywhere
603 Decline (9)
604 Does not exist anywhere
606 Does not exist anywhere




