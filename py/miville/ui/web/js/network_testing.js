// import Nevow.Athena

NetworkTesting = Nevow.Athena.Widget.subclass('NetworkTesting');

/**
 * This is subclass of Nevow.Athena.Widget.
 * It represent the Network testing interface.
 * 
 * This class is seperated in 5 parts:
 *  - initialisation (__init__)
 *  - utility methods
 *  - methods call from the server
 *  - methods call from the client interface
 *  - methods responsible of updating interface elements (buttons, etc.) state.
 * 
 * When an event occur notify_controllers is call with an event name as argument.
 * notify_controllers propagate the event to all controllers.
 * Each controller is responsible to update is state in function of the event
 * it receive.
 * 
 * @class NetworkTesting
 * @base Nevow.Athena.Widget
 */
NetworkTesting.methods(
    /**
     * Initialisation method.
     *
     * @constructor
     * @member Streams
     * @param node The DOM node that hold the widget associated with this class.
     */
    function __init__(self, node) {
        NetworkTesting.upcall(self, "__init__", node);
		
		// State variables
		self.contact = null;
		
		// Get elements.
		self.start_btn = $('nettest_start');
		
		// Get string translations.
		self.start_str = $('js_nettest_start').get('text'); // start string
		self.stop_str = $('js_nettest_stop').get('text'); // stop string
		
		// Set translations.
		self.start_btn.value = self.start_str;
		
		// Register to the widgets communicator.
		register('nettest', self);
	},
	
	/**
	 * -----------------
	 * Utility functions
	 * -----------------
	 */
	
	/**
	 * Notify all the controllers (buttons/fields/etc) state when an event call
	 * this method.
	 * 
	 * For every controller you want to be notify you have to call the method
	 * that update the controller from here.
	 * 
	 * **!Achtung!** the order theses controller methods are call can be very
	 * important if the state of a controller is dependant of the state
	 * of another controller.
	 * 
	 * So put the least dependants at the top and more dependants at the bottom.
	 * 
	 * @member Streams
     * @param {string} event The event that fire the notification.
	 */
	function notify_controllers(self, event){
		self.upd_start_btn(event);
	},



	/**
	 * ----------------
	 * Call from Client
	 * ----------------
	 */
	
	/**
	 * Get info from others widgets.
	 * (call from the client)
	 * 
	 * Call when a contact is selected.
	 * 
	 * @member Streams
	 * @param {string} caller The short name of the widget.
	 * @param {string} key The name of the receive information.
	 * @param value The receive information.
	 */
	function update(self, caller, key, value) {
		dbug.info('NETTEST');
		dbug.info(caller);
		dbug.info(key);
		dbug.info(value);
		if (caller == 'adb') {
			self.contact = value;
			// cancel_edit & selection keys : 
            if (['selection', 'cancel_edit'].contains(key)) {
				if (value == null) {
					self.notify_controllers('contact_unselected');
				} else {
					self.notify_controllers('contact_selected');
				}
			} else if (['edit', 'add'].contains(key)) {
				self.notify_controllers('contact_unselected');
			}
		}
	},

	/**
	 * Start the streams of this contact.
	 * (call from the client)
	 * 
	 * @member Streams
	 */
	function start_test(self) {
		self.callRemote('rc_start_test', self.contact.get('name'));
	},

	/**
	 * Stop the streams of this contact.
	 * (call from the client)
	 * 
	 * @member Streams
	 */
	function stop_test(self) {
		self.callRemote('rc_stop_test', self.contact.get('name'));
	},

	/**
	 * ------------------
	 * Update controllers
	 * ------------------
	 */

    /**
     * Update the start button in function of the selected contact.
     *
     * @member Streams
     * @param {string} event The event that trigger the update.
     */
	function upd_start_btn(self, event) {
		dbug.info(event);
		// list of events that "list" should react to
		if ('contact_selected' == event) {
			// set the default state
			var button_state = 'disabled';
			var button_name = self.start_str; // "start" string that is i18nized
			
			// get the state of other controls necessary to find the state
			var stream_state = self.contact.get('stream_state');
			var connection_state = self.contact.get('state').toInt();

			self.start_btn.removeEvents('click');
			if ([0, 3].contains(connection_state)) {
				if (stream_state == 0) {
					button_state = 'enabled';
					self.start_btn.addEvent('click', function(){
						self.start_test();
					});
				} else if (stream_state == 1) {
					button_state = 'disabled';
					button_name = self.start_str;
				} else {
					button_state = 'enabled';
					button_name = self.stop_str;
					self.start_btn.addEvent('click', function(){
						self.stop_test();
					});
				}
			}
			
			self.start_btn.value = button_name;
			
			if (button_state == 'enabled') {
				self.start_btn.disabled = false;
			} else {
				self.start_btn.disabled = true;
			}
		} else if (['contact_unselected',
					'add_contact'].contains(event)) {
			self.start_btn.disabled = true;
			self.start_btn.value = self.start_str;
		}
	}

	
);
