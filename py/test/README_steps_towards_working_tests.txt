What to do to make miville stream using the web/cli ui
------------------------------------------------------

 * 1 - miville non-blocking output for logs - at least.
 * 2 - create a Notification wrapper class
 * 2.1 - use the Notification class in Api, StreamsManager, MilhouseService, MilhouseStream, MilhouseProcessManager and ManagedProcessProtocol.
 * 3 - write down test requisites
 * 3.1 - list tests that need to be done.
 * 3.2 - write selenium tests for web ui

Communication
-------------
 * use network-distributed deferreds. (ComChanDeferred with expected answer)
 * fix the ComChanDialog if not working.
 * use the ComChanDialog class in streams/communication

Stream State
------------
 * remove stream_state from addressbook.Contact. (and .py and .js files)
   * create StreamsManager.contact_is_streaming()
 * parse milhouse output : WARNING/ERROR/CRITICAL (priorty to highest)

