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
		self.strm_details = $('strm_details');
		self.strm_sets = $('strm_sets');
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
		self.callRemote('rc_update_profiles');
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
	 * (called from server)
	 * 
	 * @member Streams
     * @param {array} profiles An array of preset setting object. 
	 */
    function update_profiles(self, profiles) {
		if (profiles.length > 0) {
			self.empty = false;
			self.global_slct.empty();
			self.global_slct.disabled = false;
			// populate with profiles name and ID: 
			if (profiles.length > 0) {
				// add the preset section title
				var optgroup = new Element('optgroup', {
					'label': self.preset_str
				});
				optgroup.inject(self.global_slct);
				profiles.each(function(setting){
					var opt = new Element('option', {
						'html': setting.name,
						'value': setting.id
					});
					opt.inject(optgroup);
				});
			}
		} else {
			// if there's no profile to list, disable the menu
			self.empty = false;
			self.global_slct.disabled = true;
		}
	},
    /**
     * Called when a user changes the value of a select drop-down.
     *
     * Will change the value of a set in a profile to a setting ID.
     *
     * @member Streams
     * @param {int} profile_id
     * @param {string} set_name
     * @param {int} setting_id
     */
    function set_set_value(self, profile_id, set_name, setting_id) {
        dbug.info("set_set_value " + profile_id + " " + set_name + " " + setting_id);
        self.callRemote('rc_set_value_of_set_for_profile', profile_id, set_name, setting_id, self.contact.get('name'));
    },
	/**
	 * Update the details for the currently selected profile.
	 * (called from server)
	 *
	 * sets = [ { set_name:set_name, settings:{setting_id:setting_desc}, chosen:setting_id , desc:desc}
	 * 
	 * @member Streams
     * @param {array} entries_details An array of dict with keys "name" and "value". (each entry value)
     * @param {array} sets An array of dict with keys "name" and "value". (each set with the chosen setting in each)
	 */
    function update_details(self, entries_details, sets_details, profile_id) {
        // Creating the table with all the details for each entry. (expert mode)
        self.strm_details.empty();
        self.strm_sets.empty();
        var is_odd = true;
        var style_name = "";
        //if (entries_details.length > 0) {
        //    var table = new Element('table', {"class": "tight_table"});
        //    table.inject(self.strm_details);
        //    entries_details.each(function(detail) 
        //    {
        //        if (is_odd) {
        //            style_name = "color_zebra";
        //        } else {
        //            style_name = "";
        //        }
        //        var tr = new Element('tr', {"class": style_name});
        //        var td1 = new Element("td").inject(tr);
        //        var td2 = new Element("td").inject(tr);
        //        td1.appendText(detail.name + " :");
        //        td2.appendText(detail.value);
        //        tr.inject(table);
        //        is_odd = ! is_odd;
        //    });
        //}
        if (sets_details.length > 0) {
            var sets_table = new Element("table");
            sets_table.inject(self.strm_sets);
            sets_details.each(function(a_set)
            {
                if (is_odd) {
                    style_name = "color_zebra";
                } else {
                    style_name = "";
                }
                var tr = new Element('tr', {"class": style_name});
                var td1 = new Element("td").inject(tr);
                var td2 = new Element("td").inject(tr);
                td1.appendText(a_set.desc + " :");
                var sel = new Element('select', {"set_name":a_set.set_name, "profile_id":profile_id}).inject(td2);
                a_set.settings.each(function(setting)
                {
                    var opt = new Element('option', {
						'html': setting.setting_desc,
						'value': setting.setting_id,
						'selected': setting.is_selected
                        }).inject(sel);
                });
                sel.addEvent('change', function(event) {
                    dbug.info("on change " + a_set.desc);
                    var profile_id = event.target.getProperty("profile_id");
                    var set_name = event.target.getProperty("set_name");
                    var setting_id = event.target.value;
                    self.set_set_value(profile_id, set_name, setting_id);
                    event.target.blur(); // lose focus on form element
                });
                tr.inject(sets_table);
                is_odd = ! is_odd;
            });
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
	function set_profile(self) {
		var profile_id = self.global_slct.get('inputValue');
		self.callRemote('rc_set_profile', self.contact.get('name'), profile_id);
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
     * (if state == DISCONNECTED or state == CONNECTED)
     *
     * See miville/connectors/states.py
     */
	function upd_start_btn(self, event) {
		// list of events that "list" should react to
		if (event == 'contact_selected') 
        {
			// set the default state
			var button_state = 'disabled';
			var button_name = self.start_str;
			// get the state of other controls necessary to find the state
			var stream_state = self.contact.get('stream_state');
			var connection_state = self.contact.get('state');
			// add event to the button
			self.start_btn.removeEvents('click');
			if (connection_state == 'disconnected' || connection_state == 'connected') {
				if (stream_state == 'stopped') {
                    if (connection_state == 'connected') {
                        button_state = 'enabled';
                    }
					self.start_btn.addEvent('click', function(){
						self.start_streams();
					});
				} else if (stream_state == 'starting') {
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
		if ('contact_selected' == event) {
		    self.global_slct.removeEvents('change');
			// get the setting id of the selected contact
			var setting = self.contact.get('setting')
			if (setting) {
				self.global_slct.set('inputValue', setting);
			} else {
				dbug.info('Error : setting is null');
			}
			self.global_slct.addEvent('change', function(){
				self.set_profile();
			});
		    self.callRemote('rc_profile_details_for_contact', self.contact.get('name'));
			// Enables or disables the settings drop-down according to : 
            //  * the state of the stream (stopped or streaming) Has to be stopped to be enabled.
            //  * if the settings menu is empty
			if (self.empty || self.contact.get('stream_state') != 'stopped') {
				self.global_slct.disabled = true;
			} else {
				self.global_slct.disabled = false;
			}
		} else if (['contact_unselected',
					'add_contact'].contains(event)) {
		    self.global_slct.removeEvents('change');
			self.global_slct.disabled = true;
		}
	}
);
