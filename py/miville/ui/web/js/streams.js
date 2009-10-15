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
		self.empty = true;
		
		// Get DOM elements.
		self.start_btn = $('strm_start');
		self.global_slct = $('strm_global_setts');
		
		// Get string translations.
		self.start_str = $('js_strm_start').get('text');
		self.stop_str = $('js_strm_stop').get('text');
		self.user_str = $('js_strm_user').get('text');
		self.preset_str = $('js_strm_preset').get('text');
		
		// Set translations.
		self.start_btn.value = self.start_str;
		
		// Register to the widgets communicator.
		register('strm', self);
		
		// Request the settings list
		self.callRemote('rc_update_settings');
	},

	/**
	 * -----------------
	 * Utility methods
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
     *
     * Possible event names :
     *  - 'contact_selected'
     *  - 'contact_unselected'
	 */
	function notify_controllers(self, event){
		self.upd_start_btn(event);
		self.upd_global_slct(event);
	},

	/**
	 * -----------------------------
	 * Called from the python Server
	 * -----------------------------
	 */

	/**
	 * Update the settings menu to reflect the state of the server.
	 * (call from server)
	 * 
	 * @member Streams
     * @param {array} presets An array of preset setting object. 
     * @param {array} users An array of user setting object. 
	 */
    function update_settings(self, presets, users) {
		if (users.length > 0 || presets.length > 0) {
			self.empty = false;
			self.global_slct.empty();
			self.global_slct.disabled = false;
			
			// // populate the user settings
			// if (users.length > 0) {
			// 	// add the user section title
			// 	var optgroup = new Element('optgroup', {
			// 		'label': self.user_str
			// 	});
			// 	optgroup.inject(self.global_slct);
			// 	
			// 	users.each(function(setting){
			// 		var opt = new Element('option', {
			// 			'html': setting.name,
			// 			'value': setting.id
			// 		});
			// 		opt.inject(optgroup);
			// 	});
			// }
			
			// populate the user settings
			if (presets.length > 0) {
				// add the preset section title
				var optgroup = new Element('optgroup', {
					'label': self.preset_str
				});
				optgroup.inject(self.global_slct);
				
				presets.each(function(setting){
					var opt = new Element('option', {
						'html': setting.name,
						'value': setting.id
					});
					opt.inject(optgroup);
				});
			}
			
		} else {
			// if there's no setting, disabled the menu
			self.empty = false;
			self.global_slct.disabled = true;
		}
		
	},

	/**
	 * -----------------------------
	 * Called from javascript Client
	 * -----------------------------
	 */
	
	/**
	 * Info received from others widgets.
	 * (called from the javascript client)
	 * 
	 * Called when a contact is selected.
	 * 
	 * @member Streams
	 * @param {string} caller The short name of the widget.
	 * @param {string} key The name of the receive information.
	 * @param value The receive information.
     *
     * Possible update keys : 
     *  - 'selection'
     *  - 'cancel_edit'
	 */
	function update(self, caller, key, value) {
		if (caller == 'adb') {
			// keep reference of the selected contact
			self.contact = value;
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
	function start_streams(self) {
		self.callRemote('rc_start_streams', self.contact.get('name'));
	},

	/**
	 * Stop the streams of this contact.
	 * (call from the client)
	 * 
	 * @member Streams
	 */
	function stop_streams(self) {
		self.callRemote('rc_stop_streams', self.contact.get('name'));
	},

	/**
	 * Save the newly selected global settings in the selected contact.
	 * (call from the client)
	 * 
	 * @member Streams
	 */
	function set_setting(self) {
		var setting = self.global_slct.get('inputValue');
		self.callRemote('rc_set_setting', self.contact.get('name'), setting);
	},

	/**
	 * ------------------
	 * Update controllers
	 * ------------------
	 */

    /**
     * Updates the start button in function of the selected contact.
     *
     * @member Streams
     * @param {string} event The event that trigger the update.
     *
     * Possible keys: 
     *  - 'contact_selected'
     *  - 'contact_unselected'
     *  - 'add_contact'
     *
     * If key is adb.'contact_selected', checks the connection state. 
     * If we are streaming, sets the button accordingly.
     * (if state = 0 or 3)
     *
     * See miville/connectors/states.py
     */
	function upd_start_btn(self, event) {
		// list of events that "list" should react to
		if ('contact_selected' == event) {
			
			// set the default state
			var button_state = 'disabled';
			var button_name = self.start_str;
			
			// get the state of other controls necessary to find the state
			var stream_state = self.contact.get('stream_state');
			var connection_state = self.contact.get('state').toInt();

			// add event to the button
			self.start_btn.removeEvents('click');
			if ([0, 3].contains(connection_state)) {
                // "start" if:
                // 0: DISCONNECTED
                // 3: CONNECTED
				if (stream_state == 0) {
					button_state = 'enabled';
					self.start_btn.addEvent('click', function(){
						self.start_streams();
					});
				} else if (stream_state == 1) {
					button_name = self.stop_str;
				} else {
					button_state = 'enabled';
					button_name = self.stop_str;
					self.start_btn.addEvent('click', function(){
						self.stop_streams();
					});
				}
			}
			
			// set the name
			self.start_btn.value = button_name;
			
			// set the state
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

	},
	
    /**
     * Updates the global settings menu in function of the selected contact.
     *
     * @member Streams
     * @param {string} event The event that trigger the update.
     *
     * Possible keys: 
     *  - 'contact_selected'
     *  - 'contact_unselected'
     *  - 'add_contact'
     */
	function upd_global_slct(self, event) {
		self.global_slct.removeEvents('change');
		if ('contact_selected' == event) {
			// get the setting id of the selected contact
			var setting = self.contact.get('setting')
			if (setting) {
				self.global_slct.set('inputValue', setting);
			} else {
				dbug.info('We have a problem Roger: setting = null');
			}
			self.global_slct.addEvent('change', function(){
				self.set_setting();
			});
			
			// enable or disable the menu in function the state of the stream,
			// the contact type (auto_created) or if the menu is empty
			if (self.empty || self.contact.get('stream_state') != 0 || self.contact.get('auto_created') == 'true') {
				self.global_slct.disabled = true;
			} else {
				self.global_slct.disabled = false;
			}

		} else if (['contact_unselected',
					'add_contact'].contains(event)) {
			self.global_slct.disabled = true;
		}
	}
	
);

