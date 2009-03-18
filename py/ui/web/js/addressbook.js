// import Nevow.Athena

Addressbook = Nevow.Athena.Widget.subclass('Addressbook');

Addressbook.methods(
    function __init__(self, node) {
        Addressbook.upcall(self, "__init__", node);

		// State variables
		// Get the selected cookie.
		self.selected = Cookie.read('adb_selected');
		
		// Get elements.
        self.list = $('adb_list');
		self.status = $('adb_status');
		self.add_btn = $('adb_add');
		self.remove_btn = $('adb_remove');
		self.join_btn = $('adb_join');
		self.name_fld = $('adb_name');
		self.address_fld = $('adb_address');
		self.port_fld = $('adb_port');
		self.contact_flds = [self.name_fld, self.address_fld, self.port_fld]
		self.edit_btn = $('adb_edit');

		// Get string translations. (maybe we can do this automatically?)
		self.save_str = $('js_adb_save').get('text');
		self.edit_str = $('js_adb_edit').get('text');
		self.join_str = $('js_adb_join').get('text');
		self.unjoin_str = $('js_adb_unjoin').get('text');
		
		// Set translations.	#maybe call notify_controllers('init')
		self.edit_btn.value = self.edit_str;
		self.join_btn.value = self.join_str;

		// Create new base elements.
		self.conn_icon = new Element('div', {
			'class': 'conn_icon'
		});
				
		// Create the empty field validator. (move to utils?)
		self.isEmpty = new InputValidator('required', {
		    errorMsg: 'This field is required.',
		    test: function(field) {
		        return ((field.value == null) || (field.value.length == 0));
		    }
		});
		
		// Add some keyboard events to the info fields. 
		self.contact_flds.each(function(elem) {
			elem.addEvent('enter', function(event){
				self.upd_edit_btn();
			});
			
			elem.addEvent('escape', function(event){
				self.cancel_edit_flds();
			});
		});
		
		// Get the contact list from the server.
        self.callRemote("rc_get_list");
		
		// Register to the widgets communicator.
		register('adb', self);
    },  


	///////////////////////
	/* Utility functions */
	///////////////////////
	
	// Notify all the controllers (buttons/fields/etc) state when an event call this method.
	// For every controller you want to be notify you have to call to the method
	// that update the controller from here.
	// !Atchung! the order theses controller methods are call can be very important
	// if the state of a controller is dependant of the state of another controller.
	// So put the least dependants at the top and more dependants at the bottom.
	function notify_controllers(self, event){
			self.upd_remove_btn(event);
	},

	// Return the selected contact li element or null. 
	function get_selected(self) {
		if (self.selected) {
			return self.list.getElement('li[name=' + self.selected +']');
		} else {
			return null;
		}
	},

	// Return the state contact li element or null. 
	function get_selected_state(self) {
		var contact = self.get_selected();
		if (contact) {
			return contact.get('state');
		} else {
			return null;
		}
	},

	//////////////////////
	/* Call from Server */
	//////////////////////

	// Update the contact list to reflect the state of the server.
	// Changed only what was changed.
    function update_list(self, contacts) {
		// maybe we will have to deal with the scroll position in the future
		// dbug.info(self.list.getScroll().y);
		var counter = 0;
		// previous updated contact (li element)
		var previous = null;
		// current selected li element
		var selected = null;
		
		// list of contact names coming from the server
		var server_list = contacts.map(function(item, index){
			return item[0];
		});
		
		// list of contact names on the client side
		var client_list = self.list.getChildren('li').get('name');
		
		// remove deleted contacts on the client side 
		client_list.each(function(item){
			if (item && !server_list.contains(item)) {
				self.list.getElement('li[name=' + item + ']').dispose();
			}
		});
		
		// add/update each contact from the server list
		contacts.each(function(item){
			dbug.info(item)

			// counter to know if the row are odd or even
			counter += 1;
			
			// update info
			if (client_list.contains(item[0])) {
				var li = self.list.getElement('li[name=' + item[0] + ']');
				li.set('state', item[1]);
				
			// add the new contact
			} else {
				var li = new Element('li', {
					'class': 'color_selectable',
					'html': '',
					'text': item[0],
					'name': item[0],
					'state': item[1],
					'status': '',
					'error': '',
					'events': {
						'click': function() {
							self.contact_selected(this);
						},
						'dblclick': function() {
							self.upd_edit_btn();
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
			// keep the current position
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
			if (item[1] > 0 && item[1] < 3) {
				conn_state.addClass('spinner_small');
			} else if (item[1] == 3) {
				conn_state.addClass('conn_connected');
			}
			
			// keep the current selected li element
			if (item[0] == self.selected) {
				selected = li;
			}
		})

		if (selected){
			// update the display of selected contact info and button states
			selected.fireEvent('click');
		} else {
			// or if no selection, clear the info and update button states 
			self.contact_flds.each(function(elem) {
				elem.value = '';
			});
			self.fieldState(true);
		}
		return false;
	},
	
	function showContact(self, contact) {
		self.name_fld.value = contact.name;
		self.address_fld.value = contact.address;
		self.port_fld.value = contact.port;
	},

	function modifyContact(self) {
		self.callRemote('rc_modify_contact', self.selected, self.name_fld.value, self.address_fld.value, self.port_fld.value);
		self.selected = self.name_fld.value;
	},

	function addContact(self) {
		self.callRemote('rc_add_contact', self.name_fld.value, self.address_fld.value, self.port_fld.value);
		self.selected = self.name_fld.value;
	},

	function error(self, msg) {
		StickyWin.alert('Error', msg);
	},

	function status(self, contact, msg, details) {
		// check if owner is null
		var owner = self.list.getElement('li[name=' + contact + ']')
		if (owner) {
			owner.set('status', msg);
		} else {
			dbug.info('Owner: ' + owner + ' Contact: ' + contact);
		}
		if (details == null) {
			owner.set('error', '');
		} else {
			owner.set('error', details);
			dbug.info(details);
		}
		if (contact == self.selected) {
			self.status.set('text', msg);
			if (details == null) {
				self.status.set('title', '');
			} else {
				self.status.set('title', details);
			}
		}
	},


	//////////////////////
	/* Call from Client */
	//////////////////////
	
	// get info from others widgets
	function update(self, caller, key, value) {
		dbug.info(caller);
		dbug.info(key);
		dbug.info(value);
	},

	// update the interface in function of the selected contact
	function contact_selected(self, contact) {

		// updated the selection color
		var curr_selection = self.list.getElements('li.color_selected')
		if (curr_selection) {
			curr_selection.removeClass('color_selected');
		}
		contact.addClass('color_selected');
		
		// get the contact infos
		var name = contact.get('name');
		self.callRemote('rc_get_contact', name);
		
		// update the buttons state
		var state = contact.get('state');

		self.add_state('enable');
		
		self.edit_btn.disabled = state;
		self.joinState(state);
		self.status.set('text', contact.get('status'));
		self.status.set('title', contact.get('error'));
		self.cleanFields();
		
		// save the selected contact name
		self.selected = name;
		Cookie.write('adb_selected', name, {duration: 365});
		
		// notify the others widgets of this selection
		self.notify_controllers('contact_selected');
		notify('adb', 'selection', self.selected);
	},

	// manage add button state
	function add_state(self, state) {
		(state == 'enable') ? state=false : state=true
		self.add_btn.disabled = state;
	},

	// Update remove button state.
	function upd_remove_btn(self, event) {
		// list of events that "remove" should react to
		var events = ['contact_selected'];
		if (events.contains(event)) {
			// set the default state
			var button_state = 'disabled';

			// get the state of other controls necessary to find the state
			var contact_state = self.get_selected_state();	// state of selected contact
			var edit_state = self.edit_btn.disabled;	// state of edit button
			if (contact_state == 0 && edit_state) {
				button_state = 'enabled';	
			}
			
			// set the state of the button
			self.remove_btn.removeEvents('click');
			if (button_state == 'enabled') {
				self.remove_btn.removeClass('button_disabled');
				self.remove_btn.addEvent('click', function(){
					self.remove();
				});
			} else if (!self.remove_btn.hasClass('button_disabled')) {
				self.remove_btn.addClass('button_disabled');
			}
		}
	},

	// manage edit/save button state
	function edit_save_state(self) {
		if (self.edit_btn.value == self.save_str) {
			var validate = true;
			if (self.isEmpty.test(self.name_fld)) {
				validate = false;
				self.name_fld.addClass('notify');
				dbug.info(self.isEmpty.getError(self.name_fld));
			}
			if (self.isEmpty.test(self.address_fld)) {
				validate = false;
				self.address_fld.addClass('notify');
			}
			if (validate) {
				self.edit_btn.value = self.edit_str;
				self.fieldState(true);
				if (self.selected) {
					self.modifyContact();
				} else {
					self.addContact();
				}
			}
		} else {
			self.edit_btn.value = self.save_str;
			self.fieldState(false);
			self.name_fld.select();
		}
	},

	function joinState(self, state) {
		if (state == 0) {
			self.join_btn.disabled = false;							
			self.join_btn.value = self.join_str;							
		} else if (state > 0 && state < 3) {
			self.join_btn.disabled = false;
			self.join_btn.value = self.unjoin_str;							
		} else {
			self.join_btn.disabled = true;
			self.join_btn.value = self.join_str;							
		}
	},

	function select(self, name) {
		if (name){
			name.fireEvent('click');
		} else {
			self.contact_flds.each(function(elem) {
				elem.value = '';
			});
			self.fieldState(true);
		}
	},

	function fieldState(self, state) {
		self.contact_flds.each(function(elem) {
			elem.disabled = state;
		});
	},

	function cleanFields(self) {
		self.name_fld.removeClass('notify');
		self.address_fld.removeClass('notify');
		self.fieldState(true);
		self.edit_btn.value = self.edit_str;
	},

	function cancelEdit(self) {
		if (self.selected) {
			var elem = self.list.getElement('li[name=' + self.selected +']');
			if (elem) {
				elem.fireEvent('click');
			}
		}
	},

	function deSelect(self, name) {
		self.selected = name;
		self.list.getElements('li.color_selected').removeClass('color_selected')
		self.cleanFields();
	},

	function start_connection(self) {
		self.callRemote('rc_start_connection', self.selected);
		dbug.info(self.selected);
	},
	
	function add(self) {
		self.deSelect(null)
		self.edit_btn.value = self.save_str;	
		self.edit_btn.disabled = false;
		self.contact_flds.each(function(elem) {
			elem.value = '';
		});
		self.fieldState(false);
		self.name_fld.select();
	},
	
	function remove(self) {
		if (self.selected) {
			var selected = self.list.getElement('li.color_selected');
			var new_selection = null;
			if (selected) {
				new_selection = selected.getPrevious('li');
				if (!new_selection) {
					new_selection = selected.getNext('li');
				}
			}
			var removed = self.selected;
			if (new_selection) {
				self.selected = new_selection.get('name');				
			} else {
				self.selected = null;
			}
			self.callRemote('rc_remove_contact', removed);
		}
	}
	
);


