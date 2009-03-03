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

		// get translations
		self.save_str = $('js_adb_save').get('text');
		self.edit_str = $('js_adb_edit').get('text');
		
		// set translations
		self.adb_edit.value = self.edit_str;

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
		});
    },  


	//////////////////////
	/* Call from Server */
	//////////////////////

    function updateList(self, contacts) {
// console.info(self.adb_list.getScroll().y);
		self.adb_list.empty();
		var counter = 0;
		var selected = null;
		contacts.each(function(item){
			counter += 1;
			var li = new Element('li', {
				'class': 'color_selectable',
				'html': '',
				'text': item[0],
				'name': item[0],
				'events': {
					'click': function() {	// should be move in the client section
				self.adb_list.getElements('li.color_selected').removeClass('color_selected')
				this.addClass('color_selected');
						var name = this.get('name');
						self.callRemote('rc_get_contact', name);
						self.adb_edit.disabled = false;
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
			self.conn_icon.clone().inject(li, 'top');
			li.inject(self.adb_list);
			if (item[0] == self.selected) {
				selected = li;
			}
		})
		if (selected){
			selected.fireEvent('click');
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


	//////////////////////
	/* Call from Client */
	//////////////////////
	
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

	function editSave(self) {
		if (self.adb_edit.value == self.save_str) {
			var validate = true;
			if (self.isEmpty.test(self.adb_name)) {
				validate = false;
				self.adb_name.addClass('notify');
				//console.info(self.isEmpty.getError(self.adb_name));
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

	function add(self) {
		self.deSelect(null)
		self.adb_edit.value = self.save_str;	
		self.adb_edit.disabled = false;
		self.adb_fields.each(function(elem) {
			elem.value = '';
		});
		self.fieldState(false);
		self.adb_name.select();
	}
	
);


