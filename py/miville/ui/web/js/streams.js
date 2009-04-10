// import Nevow.Athena

Streams = Nevow.Athena.Widget.subclass('Streams');

/**
 * This is subclass of Nevow.Athena.Widget.
 * It represent the Streams interface.
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
 * @class Streams
 * @base Nevow.Athena.Widget
 */
Streams.methods(
    /**
     * Initialisation method.
     *
     * @constructor
     * @member Streams
     * @param node The DOM node that hold the widget associated with this class.
     */
    function __init__(self, node) {
        Streams.upcall(self, "__init__", node);
		
		// State variables
		self.contact = null;
		
		// Get elements.
		self.start_btn = $('strm_start');
		
		// Get string translations.
		self.start_str = $('js_strm_start').get('text');
		self.stop_str = $('js_strm_stop').get('text');
		
		// Set translations.
		self.start_btn.value = self.start_str;
		
		// Register to the widgets communicator.
		register('strm', self);
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
	 * @member Help
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
	 * @member Help
	 * @param {string} caller The short name of the widget.
	 * @param {string} key The name of the receive information.
	 * @param value The receive information.
	 */
	function update(self, caller, key, value) {
		dbug.info('STREAM');
		dbug.info(caller);
		dbug.info(key);
		dbug.info(value);
		if (key == 'selection') {
			self.contact = value;
			if (value == null) {
				self.notify_controllers('contact_unselected');
			} else {
				self.notify_controllers('contact_selected');
			}
		}
	},

	/**
	 * ------------------
	 * Update controllers
	 * ------------------
	 */

    /**
     * Update the start button in function of the selected contact.
     *
     * @member Help
     * @param {string} event The event that trigger the update.
     */
	function upd_start_btn(self, event) {
		// list of events that "list" should react to
		if (event == 'contact_selected') {
			
			// set the default state
			var button_state = 'disabled';
			var button_name = self.start_str;
			
			// get the state of other controls necessary to find the state
			

				
		//} else if (event == 'contact_unselected') {
		//	self.unselect_contact();
		}
	}

	
);
