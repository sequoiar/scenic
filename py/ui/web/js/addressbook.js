// import Nevow.Athena

Addressbook = Nevow.Athena.Widget.subclass('Addressbook');

Addressbook.methods(
    function __init__(self, node) {
        Addressbook.upcall(self, "__init__", node);
		
		// state variables
		// get the selected cookie
		self.selected = Cookie.read('adb_selected');
		
		// get elements
        self.adb_list = $('adb_list');
		self.adb_edit = $('adb_edit');
		self.adb_name = $('adb_name');
		self.adb_address = $('adb_address');
		self.adb_port = $('adb_port');
		self.adb_fields = [self.adb_name, self.adb_address, self.adb_port]
		self.adb_add = $('adb_add');
		self.adb_remove = $('adb_remove');
		self.adb_join = $('adb_join');
		self.adb_status = $('adb_status');

		// get string translations (maybe we can do this automatically?)
		self.save_str = $('js_adb_save').get('text');
		self.edit_str = $('js_adb_edit').get('text');
		self.join_str = $('js_adb_join').get('text');
		self.unjoin_str = $('js_adb_unjoin').get('text');
		
		// set translations
		self.adb_edit.value = self.edit_str;
		self.adb_join.value = self.join_str;

		// create new base elements
		self.conn_icon = new Element('div', {
			'class': 'conn_icon'
		});
				
		// get the contact list
        self.callRemote("rc_get_list");
		
		// create the empty field validator
		self.isEmpty = new InputValidator('required', {
		    errorMsg: 'This field is required.',
		    test: function(field) {
		        return ((field.value == null) || (field.value.length == 0));
		    }
		});
		
		// add some keyboard events to the info fields 
		self.adb_fields.each(function(elem) {
			elem.addEvent('enter', function(event){
				self.editSave();
			});
			
			elem.addEvent('escape', function(event){
				self.cancelEdit();
			});
		});
		
		// register to the widgets communicator
		register('adb', self);
    },  


	//////////////////////
	/* Call from Server */
	//////////////////////

	/* Update the contact list to reflect the state of the server */
	/* Changed only what was change */
    function updateList(self, contacts) {
		// maybe we will have to deal with the scroll position in the future
		// dbug.info(self.adb_list.getScroll().y);
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
		var client_list = self.adb_list.getChildren('li').get('name');
		
		// remove deleted contacts on the client side 
		client_list.each(function(item){
			if (item && !server_list.contains(item)) {
				self.adb_list.getElement('li[name=' + item + ']').dispose();
			}
		});
		
		// add/update each contact from the server list
		contacts.each(function(item){
			dbug.info(item)

			// counter to know if the row are odd or even
			counter += 1;
			
			// update info
			if (client_list.contains(item[0])) {
				var li = self.adb_list.getElement('li[name=' + item[0] + ']');
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
							self.editSave();
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
					li.inject(self.adb_list, 'top');
				} else {
					li.inject(self.adb_list);
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
			self.adb_fields.each(function(elem) {
				elem.value = '';
			});
			self.fieldState(true);
		}
		return false;
	},
	
	function showContact(self, contact) {
		self.adb_name.value = contact.name;
		self.adb_address.value = contact.address;
		self.adb_port.value = contact.port;
	},

	function modifyContact(self) {
		self.callRemote('rc_modify_contact', self.selected, self.adb_name.value, self.adb_address.value, self.adb_port.value);
		self.selected = self.adb_name.value;
	},

	function addContact(self) {
		self.callRemote('rc_add_contact', self.adb_name.value, self.adb_address.value, self.adb_port.value);
		self.selected = self.adb_name.value;
	},

	function error(self, msg) {
		StickyWin.alert('Error', msg);
	},

	function status(self, contact, msg, details) {
		// check if owner is null
		var owner = self.adb_list.getElement('li[name=' + contact + ']')
		owner.set('status', msg);
		if (details == null) {
			owner.set('error', '');
		} else {
			owner.set('error', details);
			dbug.info(details);
		}
		if (contact == self.selected) {
			self.adb_status.set('text', msg);
			if (details == null) {
				self.adb_status.set('title', '');
			} else {
				self.adb_status.set('title', details);
			}
		}
	},


	//////////////////////
	/* Call from Client */
	//////////////////////
	
	// get info from others widgets
	function update(self, caller, call, data) {
		dbug.info(caller);
		dbug.info(call);
		dbug.info(data);
	},

	// update the interface in function of the selected contact
	function contact_selected(self, contact) {
		// updated the selection color
		var curr_selection = self.adb_list.getElements('li.color_selected')
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
		self.remove_state('enable');
		
		self.adb_edit.disabled = state;
		self.joinState(state);
		self.adb_status.set('text', contact.get('status'));
		self.adb_status.set('title', contact.get('error'));
		self.adb_remove.removeEvents('click');
		self.adb_remove.addClass('button_disabled');
		if (state == 0) {
			self.adb_remove.removeClass('button_disabled');
			self.adb_remove.addEvent('click', function(){
				self.remove();
			});
		}
		self.cleanFields();
		
		// save the selected contact name
		self.selected = name;
		Cookie.write('adb_selected', name, {duration: 365});
		
		// notify the others widgets of this selection
		notify('adb', 'selection', self.selected);
	},

	// manage add button state
	function add_state(self, state) {
		(state == 'enable') ? state=false : state=true
		self.adb_add.disabled = state;
	},

	// manage remove button state
	function remove_state(self, state) {
		(state == 'enable') ? state=false : state=true
		self.adb_remove.disabled = state;
	},

	function joinState(self, state) {
		if (state == 0) {
			self.adb_join.disabled = false;							
			self.adb_join.value = self.join_str;							
		} else if (state > 0 && state < 3) {
			self.adb_join.disabled = false;
			self.adb_join.value = self.unjoin_str;							
		} else {
			self.adb_join.disabled = true;
			self.adb_join.value = self.join_str;							
		}
	},

	function select(self, name) {
		if (name){
			name.fireEvent('click');
		} else {
			self.adb_fields.each(function(elem) {
				elem.value = '';
			});
			self.fieldState(true);
		}
	},

	function fieldState(self, state) {
		self.adb_fields.each(function(elem) {
			elem.disabled = state;
		});
	},

	function cleanFields(self) {
		self.adb_name.removeClass('notify');
		self.adb_address.removeClass('notify');
		self.fieldState(true);
		self.adb_edit.value = self.edit_str;
	},

	function cancelEdit(self) {
		if (self.selected) {
			var elem = self.adb_list.getElement('li[name=' + self.selected +']');
			if (elem) {
				elem.fireEvent('click');
			}
		}
	},

	function editSave(self) {
		if (self.adb_edit.value == self.save_str) {
			var validate = true;
			if (self.isEmpty.test(self.adb_name)) {
				validate = false;
				self.adb_name.addClass('notify');
				dbug.info(self.isEmpty.getError(self.adb_name));
			}
			if (self.isEmpty.test(self.adb_address)) {
				validate = false;
				self.adb_address.addClass('notify');
			}
			if (validate) {
				self.adb_edit.value = self.edit_str;
				self.fieldState(true);
				if (self.selected) {
					self.modifyContact();
				} else {
					self.addContact();
				}
			}
		} else {
			self.adb_edit.value = self.save_str;
			self.fieldState(false);
			self.adb_name.select();
		}
	},

	function deSelect(self, name) {
		self.selected = name;
		self.adb_list.getElements('li.color_selected').removeClass('color_selected')
		self.cleanFields();
	},

	function start_connection(self) {
		self.callRemote('rc_start_connection', self.selected);
		dbug.info(self.selected);
	},
	
	function add(self) {
		self.deSelect(null)
		self.adb_edit.value = self.save_str;	
		self.adb_edit.disabled = false;
		self.adb_fields.each(function(elem) {
			elem.value = '';
		});
		self.fieldState(false);
		self.adb_name.select();
	},
	
	function remove(self) {
		if (self.selected) {
			var selected = self.adb_list.getElement('li.color_selected');
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


