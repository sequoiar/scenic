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
		self.adb_remove = $('adb_remove');
		self.adb_join = $('adb_join');
		self.adb_status = $('adb_status');

		// get translations (maybe we can do this automatically?)
		self.save_str = $('js_adb_save').get('text');
		self.edit_str = $('js_adb_edit').get('text');
		self.join_str = $('js_adb_join').get('text');
		self.unjoin_str = $('js_adb_unjoin').get('text');
		
		// set translations
		self.adb_edit.value = self.edit_str;
		self.adb_join.value = self.join_str;

		// get the list
        self.callRemote("rc_get_list");
		
		// create new base element
		self.conn_icon = new Element('div', {
			'class': 'conn_icon'
		});
		
		self.isEmpty = new InputValidator('required', {
		    errorMsg: 'This field is required.',
		    test: function(field) {
		        return ((field.value == null) || (field.value.length == 0));
		    }
		});
		
		self.adb_fields.each(function(elem) {
			elem.addEvent('enter', function(event){
				self.editSave();
			});
			
			elem.addEvent('escape', function(event){
				self.cancelEdit();
			});
		});
    },  


	//////////////////////
	/* Call from Server */
	//////////////////////

    function updateList(self, contacts) {
		// dbug.info(self.adb_list.getScroll().y);
		self.adb_list.empty();
		var counter = 0;
		var selected = null;
		contacts.each(function(item){
			counter += 1;
			dbug.info(item)
			var li = new Element('li', {
				'class': 'color_selectable',
				'html': '',
				'text': item[0],
				'name': item[0],
				'state': item[1],
				'events': {
					'click': function() {	// should be move in the client section
				self.adb_list.getElements('li.color_selected').removeClass('color_selected')
				this.addClass('color_selected');
						var name = this.get('name');
						self.callRemote('rc_get_contact', name);
						self.adb_edit.disabled = false;
						self.joinState(this.get('state'));
						self.adb_remove.removeClass('button_disabled');
						self.adb_remove.removeEvents('click');
						self.adb_remove.addEvent('click', function(){
							self.remove();
						});
						self.selected = name;
						self.cleanFields();
						Cookie.write('adb_selected', name, {duration: 365});						
					},
					'dblclick': function() {
						self.editSave();
					}
				}
			});
			if (counter % 2) {
				li.addClass('color_zebra');
			}
			self.conn_icon.clone().inject(li, 'top');
			li.inject(self.adb_list);
			var conn_state = self.conn_icon.clone();
			if (item[1] > 0 && item[1] < 3) {
				conn_state.addClass('spinner_small');
			} else if (item[1] == 3) {
				conn_state.addClass('conn_connected');
			}
			conn_state.inject(li, 'top');
			if (item[0] == self.selected) {
				selected = li;
			}
		})

		if (selected){
			selected.fireEvent('click');
		} else {
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

	function status(self, msg, details) {
		if (details == null) {
			self.adb_status.set('text', msg);
			self.adb_status.set('title', '');
		} else {
			self.adb_status.set('text', msg);
			self.adb_status.set('title', details);
			dbug.info(details);
		}
	},


	//////////////////////
	/* Call from Client */
	//////////////////////
	
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


