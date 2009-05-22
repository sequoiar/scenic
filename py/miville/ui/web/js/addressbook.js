// import Nevow.Athena

Addressbook = Nevow.Athena.Widget.subclass('Addressbook');

/**
 * This is subclass of Nevow.Athena.Widget.
 * It represent the addressbook interface.
 * 
 * This class is seperated in 5 parts:
 *  - initialisation (__init__)
 *  - utility methods
 *  - methods called from the python server
 *  - methods called from the javascript client interface
 *  - methods responsible of updating interface elements (buttons, etc.) state.
 * 
 * When an event occur notify_controllers is called with an event name as argument.
 * notify_controllers propagate the event to all controllers.
 * Each controller is responsible to update is state in function of the event
 * it receive.
 * 
 * @class Addressbook
 * @base Nevow.Athena.Widget
 */
Addressbook.methods(
    /**
     * Initialisation method.
     *
     * @constructor
     * @member Addressbook
     * @param node The DOM node that hold the widget associated with this class.
     */
    function __init__(self, node) {
        Addressbook.upcall(self, "__init__", node);

		// State variables
		// Get the selected cookie.
		self.selected = Cookie.read('adb_selected');
		self.selected_li = undefined;
		self.edit_type = null;
		self.ask_win = null;
		
		// Get elements.
        self.list = $('adb_list');
		self.status = $('adb_status'); // the adb_status message div // the adb_status message 
		self.add_btn = $('adb_add');
		self.remove_btn = $('adb_remove');
		self.connect_btn = $('adb_join');
		self.name_fld = $('adb_name');
		self.address_fld = $('adb_address');
		self.port_fld = $('adb_port');
		self.contact_flds = [self.name_fld, self.address_fld, self.port_fld]
		self.auto_answer_chk = $('adb_auto_answer');
		self.edit_btn = $('adb_edit');
		self.error_div = $('adb_error_msg');

		// Get string translations. TODO: (maybe we can do this automatically?)
		self.save_str = $('js_adb_save').get('text');
		self.edit_str = $('js_adb_edit').get('text');
		self.keep_str = $('js_adb_keep').get('text');
		self.connect_str = $('js_adb_join').get('text');
		self.disconnect_str = $('js_adb_unjoin').get('text');
		self.accept_str = $('js_adb_accept').get('text');
		self.refuse_str = $('js_adb_refuse').get('text');
		self.optional_str = $('js_adb_optional').get('text');
		
		// Set translations.	TODO: #maybe call notify_controllers('init')
		self.edit_btn.value = self.edit_str;
		self.connect_btn.value = self.connect_str;

		// Create new base elements.
		self.conn_icon = new Element('div', {
			'class': 'basic_icon'
		});
				
		// Create the empty field validator. TODO: (move to utils?)
		self.isEmpty = new InputValidator('required', {
		    errorMsg: 'This field is required.',
		    test: function(field) {
		        return ((field.value == null) || (field.value.length == 0));
		    }
		});
		
		// Add some keyboard events to the info fields. 
		self.contact_flds.each(function(elem) {
			elem.addEvent('enter', function(event){
				self.save_contact();
			});
			
			elem.addEvent('escape', function(event){
				self.cancel_edit_flds();
			});
		});
		
		// Add event to port field.
		self.port_fld.addEvent('focus', function(event) {
			self.unset_optional(self.port_fld);
		});
		
		self.port_fld.addEvent('blur', function(event) {
			self.set_optional(self.port_fld);
		});

		// Register to the widgets communicator.
		register('adb', self);

		// Get the contact list from the server (only when all the page is loaded).
		window.addEvent('domready', function() {
	        self.callRemote("rc_get_list");
		});
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
	 * @member Addressbook
     * @param {string} event The event that fire the notification.
     *
     * Possible event names for this widget are :
     *  - 'add_contact'
     *  - 'contact_selected'
     *  - 'contact_unselected'
     *  - 'edit_contact'
     *  - 'keep_contact'
     *  - 'save_contact'
     *  - 'update_status'
	 */
	function notify_controllers(self, event){
		self.upd_list(event);
		self.upd_status(event);
		self.upd_edit_btn(event);
		self.upd_connect_btn(event);
		self.upd_auto_answer_chk(event);
		self.upd_fields(event);
		self.upd_add_btn(event);
		self.upd_remove_btn(event);
	},

	/**
	 * Returns the attribute specified of the contact li element or null.
	 * 
	 * @member Addressbook
     * @param {string} attr The name of the attribute you want.
     * @return The value of the attribute.
	 */
	function get_selected_attr(self, attr) {
		if (self.selected_li != null) {
			return self.selected_li.get(attr);
		} else {
			return undefined;
		}
	},

	/**
	 * Saves the name of the selected contact.
	 * 
	 * @member Addressbook
     * @param {string} name The name of the newly selected contact.
     */
	function update_selected(self, name) {
		self.selected = name;
		Cookie.write('adb_selected', self.selected, {
			duration: 365
		});
	},

	/**
	 * Removes the 'optional' word from the specified field.
	 * 
     * @member Addressbook
	 * @param {Text input DOM object} field
	 */
	function unset_optional(self, field) {
		if (field.value == self.optional_str) {
			field.value = '';
		}
		field.removeClass('color_optional');
	},
	
	/**
	 * Add the 'optional' word in the specified field.
	 * 
     * @member Addressbook
	 * @param {Text input DOM object} field
	 */
	function set_optional(self, field) {
		if (field.value == '') {
			field.value = self.optional_str;
			if (!field.hasClass('color_optional')) {
				field.addClass('color_optional');
			}
		}
	},		


	/**
	 * -------------------------
	 * Called from python Server
	 * -------------------------
	 */

	/**
	 * Updates the contact list to reflect the state of the server.
	 * (called from ui/web/widgets/addressbook.py)
	 * 
	 * Changed only what was changed.
	 * 
	 * @member Addressbook
     * @param {array} contacts An array of contacts object. 
	 */
    function update_list(self, contacts) {
		// maybe we will have to deal with the scroll position in the future
		// dbug.info(self.list.getScroll().y);
		var counter = 0;
		// previous updated contact (li element)
		var previous = null;
		// current selected li element
		var selected = null;
		var selected_li = null;
		
		// list of contact names coming from the server
		var server_list = contacts.map(function(item, index){
			return item['name'];
		});
		
		// list of contact names on the client side
		var client_list = self.list.getChildren('li').get('name');
		
		// remove on the client side deleted contacts  
		client_list.each(function(item){
			if (item && !server_list.contains(item)) {
				self.list.getElement('li[name=' + item + ']').dispose();
				// update the selected contact to null if we delete the selected one
				if (item == self.selected) {
					//self.update_selected(null);
					self.update_selected(self, null);
				}
			}
		});
		
		// add/update each contact from the server list
		contacts.each(function(item){
			dbug.info(item)

			// counter to know if the row are odd or even
			counter += 1;
			
			// update info
			if (client_list.contains(item['name'])) {
				var li = self.list.getElement('li[name=' + item['name'] + ']');
				li.set('state', item['state']);
				li.set('stream_state', item['stream_state']);
				li.set('setting', item['setting']);
				li.set('auto_created', (item['auto_created'] ? 'true' : ''));
				if (item['auto_created']) {
					if (!li.hasClass('auto_created')) {
						li.addClass('auto_created');
					}
				} else {
					li.removeClass('auto_created');
				}
				
			// add the new contact
			} else {
				var li = new Element('li', {
					'class': (item['auto_created'] ? 'auto_created color_selectable' : 'color_selectable'),
					'html': '',
					'text': item['name'],
					'name': item['name'],
					'state': item['state'],
					'setting': item['setting'],
					'auto_created': (item['auto_created'] ? 'true' : ''),
					'stream_state': item['stream_state'],
					'status': '',
					'error': '',
					'events': {
						'click': function() {
							self.contact_selected(this);
						},
						'dblclick': function() {
							self.edit_contact();
						}
					}
				});
				
				// add chat and state icons
				self.conn_icon.clone().inject(li, 'top');
				self.conn_icon.clone().inject(li, 'top');
				
				// insert it at the write place
				if (previous) {
					li.inject(previous, 'after');
				} else if (counter == 1) {
					li.inject(self.list, 'top');
				} else {
					li.inject(self.list);
				}
			}
			// keep the current li
			previous = li;
			
			// deal with row coloring
			if (counter % 2) {
				li.addClass('color_zebra');
			} else {
				li.removeClass('color_zebra');
			}

			// update the connection state icon
			var conn_state = li.getChildren()[0];
			conn_state.removeClass('spinner_small');
			conn_state.removeClass('conn_connected');
			if (item['state'] > 0 && item['state'] < 3) {
				conn_state.addClass('spinner_small');
			} else if (item['state'] == 3) {
				conn_state.addClass('conn_connected');
			}
			
			// update the stream/test state icon
			var stream_state = li.getChildren()[1];
			stream_state.removeClass('spinner_small');
			stream_state.removeClass('streaming');
			if (item['stream_state'] == 1) {
				stream_state.addClass('spinner_small');
			} else if (item['stream_state'] == 2) {
				stream_state.addClass('streaming');
			}
			
			// keep the current selected li element
			if (item['name'] == self.selected) {
				selected_li = li;
			}
		})
		
		// if first update simulate contact selection or unselection.
		if (self.selected_li == undefined) {
			if (selected_li != null) {
				self.contact_selected(selected_li);
			} else {
				self.contact_unselect();
			}
		
		// after first update do the normal notification
		} else {
			self.selected_li = selected_li;
	
			if (selected_li && (self.edit_type != 'new' || self.edit_type == 'keep')){
				// update the display of selected contact info and button states
				self.notify_controllers('contact_selected');
			} else {
				// or if no selection, clear the info and update button states
				self.notify_controllers('contact_unselected');
			}
			notify('adb', 'selection', self.selected_li);
		}
		
		return false;
	},
	
	/**
	 * Update the connection status of the contacts.
	 * (call from server)
	 * 
	 * @member Addressbook
     * @param {string} contact The name of the contact that have to be updated. 
     * @param {string} msg The new status message for the contact. 
     * @param {string} details The new details (complete error message) for the contact. 
	 */
	function update_status(self, contact, msg, details) {
		// check if contact exist in the list and get it
		var owner = self.list.getElement('li[name=' + contact + ']')
		if (owner) {
			owner.set('status', msg);
			if (details == null) {
				owner.set('error', '');
			} else {
				owner.set('error', details);
				dbug.info('Contact status error msg details: ' + details);
			}
			if (contact == self.selected) {
				self.notify_controllers('update_status');
			}
		} else {
			dbug.info('NO owner - Contact: ' + contact);
		}
	},

	/**
	 * Show selected contact info coming from the server in info fields.
	 * (call from the server)
	 * 
	 * @member Addressbook
	 * @param {contact object} contact
	 */
	function show_contact_info(self, contact) {
		self.name_fld.value = contact.name;
		self.address_fld.value = contact.address;
		self.port_fld.value = contact.port;
		self.auto_answer_chk.checked = contact.auto_answer;
	},

	/**
	 * Show prompt asking to accept an invitation to connect.
	 * (call from the server)
	 * 
	 * @member Addressbook
	 * @param {string} connection The name of the connection who ask.
	 * @param {string} caption The caption of the notification window.
	 * @param {string} body The text body of the notification window.
	 */
	function ask(self, connection, caption, body) {
		dbug.info(caption + body);
		// TODO: add support for many requests at the same time
		if (self.ask_win) {
			self.ask_win.hide();
		}
		self.ask_win = new StickyWin.PointyTip(caption, body, {
			relativeTo: self.list,
			offset: {x: 10, y: -52},
			zIndex: 404004,
	//	    width: 238,
			pointyOptions: {
			    closeButton: false,
				buttons: [{
					text: self.accept_str,
					onClick: function(){
						this.hide();
						self.callRemote('rc_accept', self.ask_win.connection);
					}
				},
				{
					text: self.refuse_str,
					onClick: function(){
						this.hide();
						self.callRemote('rc_refuse', self.ask_win.connection);
					}
				}]
			},
		    point: 10
		});
		self.ask_win.connection = connection;
	},

	/**
	 * Show prompt telling that the invitation is now over.
	 * (call from server)
	 * 
	 * @member Addressbook
	 * @param {string} caption The caption of the notification window.
	 * @param {string} body The text body of the notification window.
	 */
	function notification(self, caption, body) {
		if (self.ask_win) {
			self.ask_win.hide(); // TODO: should dispose also
		}
		self.ask_win = new StickyWin.PointyTip(caption, body, {
			relativeTo: self.list,
			offset: {x: 10, y: -52},
			zIndex: 404004,
//		    width: 238, // TODO: should modify the css and remove the css hack in clientcide StickyWin.UI.Pointy
		    closeButton: false,
		    point: 10
		});
	},

	/**
	 * Called when the contact is successfully saved.
	 * (call from the server)
	 */
	function contact_saved(self, contact) {
		self.edit_type = null;
		
		// update the selected name
		if (self.selected != contact) {
			var li = self.list.getElement('li[name=' + contact + ']');
			if (li) {
				self.contact_selected(li);
			}
			self.notify_controllers('save_contact');
		} else {
			self.notify_controllers('save_contact');
		}
	},

	
	/**
	 * Error window to be redone.
	 * 
	 * @param {string} msg
	 */
	function error(self, msg) {
        self.error_div.empty();
        var span = new Element('span').appendText(msg).inject(self.error_div);
		//StickyWin.alert('Error', msg);
	},


	/**
	 * ----------------
	 * Called from Javascript Client
	 * ----------------
	 */
	
	/**
	 * Get info from others widgets.
	 * (called from the client)
	 * 
	 * (no function for the moment)
	 * 
	 * @member Addressbook
	 * @param {string} caller The short name of the widget.
	 * @param {string} key The name of the receive information.
	 * @param value The receive information.
	 */
	function update(self, caller, key, value) {
		dbug.info(caller);
		dbug.info(key);
		dbug.info(value);
	},

    /**
     * One contact is selected.
	 * (called from the javascript client)
     *
     * @member Addressbook
     * @param {DOM node} contact The <li> node of the selected contact.
     */
	function contact_selected(self, contact) {
		// pass if it's the same contact (but not if in edit mode)
		if (contact != self.selected_li || self.edit_btn.value == self.save_str) {
			self.selected_li = contact;
			// save the selected contact name
			self.update_selected(contact.get('name'));
			
			// notify the controllers of this selection
			self.notify_controllers('contact_selected');
			notify('adb', 'selection', self.selected_li);
		}
	},

    /**
     * No contact is selected.
	 * (called from the javascipt client)
     *
     * @member Addressbook
     */
	function contact_unselect(self) {
		self.notify_controllers('contact_unselected');
		self.selected_li = null;
		self.update_selected(null);
		
		// notify the widgets of this selection
		notify('adb', 'selection', self.selected_li);
	},

    /**
     * Edit contact info.
	 * (called from the javascript client)
     *
     * @member Addressbook
     */
	function edit_contact(self) {
		if (self.get_selected_attr('state') == 0) {
			self.edit_type = 'modify';
			self.notify_controllers('edit_contact');

			// notify the widgets of this
			notify('adb', 'edit', self.selected_li);
		}
	},

    /**
     * Add a new contact.
	 * (called from the javascript client)
     *
     * @member Addressbook
     */
	function add_contact(self) {
		self.edit_type = 'new';
		self.notify_controllers('add_contact');

		// notify the widgets of this
		notify('adb', 'add', null);
	},
	
    /**
     * Save a contact (for a new, modified and keeped contact). 
	 * (called from the javascript client)
     *
     * @member Addressbook
     */
	function save_contact(self) {
		var validate = true;
		if (self.isEmpty.test(self.name_fld)) {
			validate = false;
			self.name_fld.addClass('notify');
			dbug.info(self.isEmpty.getError(self.name_fld));
		}
		if (self.isEmpty.test(self.address_fld)) {
			validate = false;
			self.address_fld.addClass('notify');
			dbug.info(self.isEmpty.getError(self.address_fld));
		}
		
		if (validate) {
			var port = self.port_fld.value;
			if (port == self.optional_str) {
				port = '';
			}
			if (self.edit_type == 'new') {
				self.callRemote('rc_add_contact',
								self.name_fld.value,
								self.address_fld.value,
								port,
								self.auto_answer_chk.checked);
			} else if (self.edit_type == 'modify') {
					self.callRemote('rc_modify_contact',
									self.selected,
									self.name_fld.value,
									self.address_fld.value,
									port,
									self.auto_answer_chk.checked);
			} else if (self.edit_type == 'keep') {
					self.callRemote('rc_keep_contact',
									self.selected,
									self.name_fld.value,
									self.auto_answer_chk.checked);
			}
            self.error('');
		}
	},

    /**
     * Keep (save) an auto-created contact.
	 * (called from the javascript client)
     *
     * @member Addressbook
     */
	function keep_contact(self) {
		self.edit_type = 'keep';
		self.notify_controllers('keep_contact');
	},
	
    /**
     * Remove a contact from the list.
	 * (called from the javascript client)
     *
     * @member Addressbook
     */
	function remove_contact(self) {
		// test if there's a contact selected
		if (self.selected_li != null) {
			// get the previous contact in the list
			var new_selection = self.selected_li.getPrevious('li');
			// if the selected contact is the first one get the next one
			if (!new_selection) {
				new_selection = self.selected_li.getNext('li');
			}
			var removed = self.selected;
			// if it's the last contact, select nothing			
			var selected = null;
			if (new_selection) {
				selected = new_selection.get('name');
			}
			//self.update_selected(selected);
			self.update_selected(self, selected);
			self.callRemote('rc_remove_contact', removed);
		}
	},
	
    /**
     * Cancel contact info editing on cancel key down.
	 * (called from the javascript client)
     *
     * @member Addressbook
     */
	function cancel_edit_flds(self) {
		if (self.selected_li != null) {
			self.notify_controllers('contact_selected');
			
			// notify other widgets
			notify('adb', 'cancel_edit', self.selected_li);
		}
	},

    /**
     * Start a connection with a contact.
	 * (called from the javascript client)
     *
     * @member Addressbook
     */
	function connect(self) {
		self.callRemote('rc_start_connection', self.selected);
		dbug.info(self.selected);
	},
	
    /**
     * Stop a connection with a contact.
	 * (called from the javascript client)
     *
     * @member Addressbook
     */
	function disconnect(self) {
		self.callRemote('rc_stop_connection', self.selected);
		dbug.info(self.selected);
	},
	

	/**
	 * ------------------
	 * Update controllers
	 * ------------------
	 */

    /**
     * Update the contacts list in function of the selected contact.
     *
     * @member Addressbook
     * @param {string} event The event that trigger the update.
     */
	function upd_list(self, event) {
		// list of events that "list" should react to
		if (event == 'contact_selected') {
			// updated the selection color
			self.unselect_contact();
			self.selected_li.addClass('color_selected');
			
			// update the buttons state
			var state = self.selected_li.get('state');
				
		} else if (['add_contact', 'contact_unselected'].contains(event)) {
			self.unselect_contact();
		}
	},

    /**
     * Unselect contact from the list (display update).
     *
     * @member Addressbook
     */
	function unselect_contact(self) {
		var curr_selection = self.list.getElements('li.color_selected')
		if (curr_selection) {
			curr_selection.removeClass('color_selected');
		}
	},

    /**
     * Update the connection status line.
     *
     * @member Addressbook
     * @param {string} event The event that trigger the update.
     */
	function upd_status(self, event) {
		// list of events that "status" should react to
		if (['contact_selected', 'update_status'].contains(event)) {
			self.status.set('text', self.selected_li.get('status'));
			self.status.set('title', self.selected_li.get('error'));
		} else if (['add_contact',
					'contact_unselected',
					'save_contact'].contains(event)) {
			self.status.set('text', '');
			self.status.set('title', '');
		}
	},

    /**
     * Update the contact fields.
     *
     * @member Addressbook
     * @param {string} event The event that trigger the update.
     */
	function upd_fields(self, event) {
		// list of events that "fields" should react to
		if (event == 'contact_selected') {
			self.name_fld.removeClass('notify');
			self.address_fld.removeClass('notify');
			self.set_fields_state('disabled');
			self.unset_optional(self.port_fld);
			self.callRemote('rc_get_contact', self.selected);
		} else if (event == 'edit_contact' || event == 'add_contact') {
			if (event == 'add_contact') {
				// clear the contact info fields
				self.contact_flds.each(function(elem) {
					elem.value = '';
				});
			}
			
			self.set_optional(self.port_fld);
			self.set_fields_state('enable');
			self.name_fld.select();
		} else if (event == 'contact_unselected') {
			// clear the contact info fields
			if (self.edit_type == null) {
				self.contact_flds.each(function(elem) {
					elem.value = '';
				});
			}
			self.set_fields_state('disabled');
			self.unset_optional(self.port_fld);
		} else if (event == 'keep_contact') {
			self.name_fld.disabled = false;
			self.name_fld.select();
			self.address_fld.disabled = true;
			self.port_fld.disabled = true;
		}
	},

    /**
     * Enable/disabled the contact info fields.
     *
     * @member Addressbook
     * @param {string} state The desired state (enable or disable).
     */
	function set_fields_state(self, state) {
		var value = true;
		if (state == 'enable') value = false;
		self.contact_flds.each(function(elem) {
			elem.disabled = value;
		});
	},

    /**
     * Update auto_answer checkbox state.
     *
     * @member Addressbook
     * @param {string} event The event that trigger the update.
     */
	function upd_auto_answer_chk(self, event) {
		// list of events that "fields" should react to
		if (event == 'edit_contact' || event == 'add_contact' || event == 'keep_contact') {
			if (event == 'add_contact' || event == 'keep_contact') {
				// clear the checkbox
				self.auto_answer_chk.checked = false;
			}
			self.auto_answer_chk.disabled = false;
		} else if (event == 'contact_unselected') {
			// clear the checkbox
			self.auto_answer_chk.checked = false;
			self.auto_answer_chk.disabled = true;
		} else if (event == 'contact_selected') {
			// clear the checkbox
			self.auto_answer_chk.disabled = true;
		}
	},

    /**
     * Update add button state.
     *
     * @member Addressbook
     * @param {string} event The event that trigger the update.
     */
	function upd_add_btn(self, event) {
		// list of events that "add" should react to
		if (['contact_selected',
			 'edit_contact',
			 'keep_contact',
			 'contact_unselected'].contains(event)) {
			// set the default state
			var button_state = 'enabled';
			
			// get the state of other controls necessary to find the state
			if (self.edit_btn.value == self.save_str) {
				button_state = 'disabled';	
			}
			
			// set the state of the button
			self.add_btn.removeEvents('click');
			if (button_state == 'enabled') {
				self.add_btn.removeClass('button_disabled');
				self.add_btn.addEvent('click', function(){
					self.add_contact();
				});
			} else if (!self.add_btn.hasClass('button_disabled')) {
				self.add_btn.addClass('button_disabled');
			}
		} else if (event == 'add_contact') {
			self.add_btn.removeEvents('click');
			if (!self.add_btn.hasClass('button_disabled')) {
				self.add_btn.addClass('button_disabled');
			}		
		}
	},

    /**
     * Update remove button state.
     *
     * @member Addressbook
     * @param {string} event The event that trigger the update.
     */
	function upd_remove_btn(self, event) {
		// list of events that "remove" should react to
		if (['contact_selected',
			 'edit_contact',
			 'add_contact',
			 'keep_contact',
			 'contact_unselected'].contains(event)) {
			// set the default state
			var button_state = 'disabled';

			// get the state of other controls necessary to find the state
			var contact_state = self.get_selected_attr('state');	// state of selected contact
			
			if (contact_state == 0 && self.edit_btn.value == self.edit_str) {
				button_state = 'enabled';	
			}
			
			// set the state of the button
			self.remove_btn.removeEvents('click');
			if (button_state == 'enabled') {
				self.remove_btn.removeClass('button_disabled');
				self.remove_btn.addEvent('click', function(){
					self.remove_contact();
				});
			} else if (!self.remove_btn.hasClass('button_disabled')) {
				self.remove_btn.addClass('button_disabled');
			}
		}
	},

    /**
     * Update edit/save button state.
     *
     * @member Addressbook
     * @param {string} event The event that trigger the update.
     */
	function upd_edit_btn(self, event) {
		if (['contact_selected', 'contact_unselected'].contains(event)) {
			// set the default states
			var button_state = 'enabled';
			var auto_created = false;
			
			// get the state of other controls necessary to find the state
			var contact_state = self.get_selected_attr('state');	// state of selected contact
			var contact_auto_created = self.get_selected_attr('auto_created');	// auto_created of selected contact
			if (contact_auto_created && event == 'contact_selected') {
			    auto_created = true;
			}

			if (contact_state != 0 && !auto_created) {
				button_state = 'disabled';	
			}
			
			// set the state of the button
			self.edit_btn.removeEvents('click');
			if (button_state == 'enabled') {
				if (auto_created) {
					self.edit_btn.value = self.keep_str;
					self.edit_btn.addEvent('click', function(){
						self.keep_contact();
					});
				} else {
					self.edit_btn.value = self.edit_str;
					self.edit_btn.addEvent('click', function(){
						self.edit_contact();
					});
				}
				self.edit_btn.disabled = false;
			} else {
				self.edit_btn.value = self.edit_str;
				self.edit_btn.disabled = true;
			}

		} else if (['edit_contact', 'add_contact', 'keep_contact'].contains(event)) {
			self.edit_btn.removeEvents('click');
			self.edit_btn.disabled = false;
			self.edit_btn.value = self.save_str;
			self.edit_btn.addEvent('click', function(){
				self.save_contact();
			});
		}			
	},

    /**
     * Update connect/disconnect button state.
     *
     * @member Addressbook
     * @param {string} event The event that trigger the update.
     */
	function upd_connect_btn(self, event) {
		// list of events that "connect" should react to
		if (['contact_selected',
			 'edit_contact',
			 'add_contact',
			 'keep_contact',
			 'save_contact',
			 'contact_unselected'].contains(event)) {

			// set the default state
			var button_state = 'disabled';
			var button_name = self.connect_str;

			// get the state of other controls necessary to find the state
			var contact_state = self.get_selected_attr('state');	// state of selected contact
			if (contact_state >= 0 && (self.edit_btn.value == self.edit_str || self.edit_btn.value == self.keep_str)) {
				button_state = 'enabled';
			}
			
			if (contact_state > 0) {
				button_name = self.disconnect_str;
			}

			// set the state of the button
			self.connect_btn.removeEvents('click');
			if (button_state == 'enabled') {
				self.connect_btn.disabled = false;
				
				if (contact_state > 0) {
					button_name = self.disconnect_str;
					self.connect_btn.addEvent('click', function(){
						self.disconnect();
					});
				} else {
					self.connect_btn.addEvent('click', function(){
						self.connect();
					});
				}
			} else {
				self.connect_btn.disabled = true;
			}
			
			// set the name of the button
			self.connect_btn.value = button_name;							
		}
	}

);


