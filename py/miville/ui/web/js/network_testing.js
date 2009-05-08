// import Nevow.Athena

NetworkTesting = Nevow.Athena.Widget.subclass('NetworkTesting');
/*
 *
var RE = /^\d*$/;
document.write(RE.test(2)); //true
document.write(RE.test('')); //true
document.write(RE.test('a')); //false

The regular expression in the preceding example specifies that the string being tested must have zero or more digit characters. Anything else is invalid.
 */
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
		self.message_div = $('nettest_message');
        self.duration_fld = $('nettest_duration');
        self.bandwidth_fld = $('nettest_bandwidth');
        self.unit_popup = $('nettest_unit');
        self.kind_popup = $('nettest_kind');
        self.progress_img = $('nettest_progress');
		
		// Get string translations.
        // TODO : use only translated strings in the GUI.
		self.start_str = $('js_nettest_start').get('text'); // start string
		self.stop_str = $('js_nettest_stop').get('text'); // stop string
		
		// Set translations.
		self.start_btn.value = self.start_str;
		
		// Register to the widgets communicator.
		register('nettest', self);
        
		// Create the empty field validator. TODO: (move to utils?)
        // uses clientcide
        // TODO: display error msg 
		self.is_number = new InputValidator('required_number', {
		    errorMsg: 'This field is required and must be a number.',
		    test: function(field) {
                var regexp = /^\d+$/;
                if (field.value == null) {
                    return false;
                } else if (field.value.length == 0) {
                    return false;
                } else {
                    return regexp.test(field.value);
                }
		    }
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
	 * @member Streams
     * @param {string} event The event that fire the notification.
	 */
	function notify_controllers(self, event){
		self.upd_start_btn(event);
	},
    
	/**
	 * ------------------
	 * Called from Client
	 * ------------------
	 */
	
	/**
	 * Get info from others widgets.
	 * (call from the client)
	 * 
	 * Called when a contact is selected.
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
			// cancel_edit & selection keys : 
            if (['selection', 'cancel_edit'].contains(key)) {
				if (value == null) {
					self.notify_controllers('contact_unselected');
                    //test_aborted
				} else {
                    self.contact = value;
		            dbug.info('contact selected: ' + value);
					self.notify_controllers('contact_selected');
				}
			} else if (['edit', 'add'].contains(key)) {
				self.notify_controllers('contact_unselected');
			}
		}
	},

	/**
	 * Starts the network test with the currenlty selected contact.
	 * (called from the js client)
	 * 
	 * @member Streams
	 */
	function start_test(self) {
        // TODO: add duration, bandwidth and kind
		var valid = true;

        // validation
		//self.is_number
        self.duration_fld = $('nettest_duration');
        self.bandwidth_fld = $('nettest_bandwidth');
        // input validation
		if (self.is_number.test(self.bandwidth_fld) == false) {
			valid = false;
			self.bandwidth_fld.addClass('notify'); // makes it red
			dbug.info(self.is_number.getError(self.bandwidth_fld));
		}
		if (self.is_number.test(self.duration_fld) == false) {
			valid = false;
			self.duration_fld.addClass('notify'); // makes it red
			dbug.info(self.is_number.getError(self.duration_fld));
		}
        if (valid == true) {
            var duration = parseInt(self.duration_fld.value);
            var bandwidth = parseInt(self.bandwidth_fld.value);
            var unit = self.unit_popup.value;
            var kind = self.kind_popup.value;

			self.bandwidth_fld.removeClass('notify'); // remove the red
			self.duration_fld.removeClass('notify'); // remove the red
			dbug.info('bw:' + bandwidth + " dur:" + duration + ' unit:' + unit + ' kind:' + kind);
            self.callRemote('rc_start_test', self.contact.get('name'), bandwidth, duration, kind, unit);
            // this will call self.make_look_like_test_occurs
	    }
    },
    /**
     * Changes the appearance of this widget when a test starts
     * (called from the python widget)
     * @members NetworkTesting
     */
    function make_look_like_test_occurs(self, message) {
		self.start_btn.value = self.stop_str;
        self.start_btn.disabled = false;
        self.start_btn.removeEvents('click');
        self.start_btn.addEvent('click', function() {
            self.abort_test();
        });
        self.message_div.empty(); // innerHTML = "";
        //var img = new Element('img').setProperty('src', 'img/macthrob-small.png').inject(self.message_div);
        //self.progress_img.setProperty
        //img.setProperty("style", "position:relative;top:4px;");
        if (! self.progress_img.hasClass('nettest_progress')) { 
            self.progress_img.addClass('nettest_progress');
        }
        var span = new Element('span').appendText(message).inject(self.message_div);
    },
    
	/**
	 * Stops the streams of this contact.
	 * (called from the client)
	 * 
	 * @member Streams
	 */
	function abort_test(self) {
		self.callRemote('rc_abort_test', self.contact.get('name'));
	},

	/**
	 * ------------------
	 * Update controllers
	 * ------------------
	 */

    /**
     * Updates the start button according to the selected contact.
     *
     * @member Streams
     * @param {string} event The event that trigger the update.
     */
	function upd_start_btn(self, event) 
    {
		dbug.info("event: " + event);
		// list of events that "list" should react to
		if (['contact_selected', 'test_aborted', 'test_done'].contains(event)) 
        {
			// set the default state
			var button_state = 'enabled'; // default...
			var button_name = self.start_str; // "start" string that is i18nized
			// get the state of other controls necessary to find the state
			var stream_state = self.contact.get('stream_state');
			var connection_state = self.contact.get('state').toInt();
            dbug.info("connection_state: " + connection_state);
            dbug.info("stream_state: " + stream_state);
			
            self.start_btn.removeEvents('click');
			if ([0, 3].contains(connection_state)) {
				if (stream_state == 0) {
					button_state = 'enabled';
					self.start_btn.addEvent('click', function() {
						self.start_test();
					});
				} else if (stream_state == 1) {
                    dbug.info("Seems like we are streaming. Disabling the nettest button.");
					button_state = 'disabled';
					button_name = self.start_str;
				} else {
                    dbug.info("Enabling the nettest button.");
					button_state = 'enabled';
					button_name = self.stop_str;
					self.start_btn.addEvent('click', function(){
						self.abort_test();
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
	},
    /**
     * Error message that makes the whole window black
     *
     * Should be called when invalid info is given by the user.
     *
	 * @member NetworkTesting
     * @param {string} Error message.
     */
    function blackscreen_error(self, msg) {
    	StickyWin.alert('Error', msg);
    },
	/**
     * Called when a network test error occurs.
	 * (called from python server)
	 * 
	 * @member NetworkTesting
     * @param {string} Error message.
	 */
    function nettest_error(self, error_text) {
        self.message_div.empty(); // innerHTML = ""
        var p = new Element('p').appendText(error_text).inject(self.message_div);
        self.start_btn.disabled = false;
        if (self.progress_img.hasClass('nettest_progress')) { 
            self.progress_img.removeClass('nettest_progress');
        }
		self.notify_controllers('test_aborted');
    },
    function _insert_result_li(self, ul, key, val) {
        var li = new Element('li', {'class':'nettest_result'}).inject(ul);
        var span = new Element('span', {'class':'nettest_key'}).appendText(key + " : ").inject(li);
        var span = new Element('span', {'class':'nettest_value'}).appendText(val).inject(li);
    },
	/**
     * Called when a network test is done.
	 * (called from server)
	 * 
     * See miville/network.py for the list of fields received from Python.
     *
	 * @member NetworkTesting
     * @param {string} contact_name The name of the contact . 
     * @param {string} local_data Dict/object with iperf stats.
     * @param {string} remote_data Dict/object with iperf stats.
	 */
	function test_results(self, contact_name, local_data, remote_data) {
		self.start_btn.value = self.start_str;
        if (self.progress_img.hasClass('nettest_progress')) { 
            self.progress_img.removeClass('nettest_progress');
        }
        dbug.info("NETTEST: test_results called");
		// check if contact exist in the list and get it
        //var owner = self.list.getElement('li[name=' + contact_name + ']')
        self.message_div.empty(); // innerHTML = "";
        
        var latency = 0.0;
        var kind = 0;

        if (local_data != null) {
            latency = local_data.latency * 1000.0; // from s to ms
            kind = local_data.test_kind;
        } else {
            latency = remote_data.latency * 1000.0;
            kind = remote_data.test_kind;
        }
        dbug.info("NETTEST kind is " + kind);
        self.message_div.empty();
        var ul = new Element('ul', {'class':'basic_list js_help nettest_list'}).inject(self.message_div);
        var li = new Element('li', {'class':'nettest_title'}).appendText('Peformance Test Results with ').inject(ul); 
        var b = new Element('b').appendText(contact_name).inject(li)

        var latency = 0.0;
        var txt = "";
        if (kind == 1) {
            txt += "Unidirectional from local to remote";
        } else if (kind == 2) {
            txt += "Bidirectional Sequential";
        } else if (kind == 3) {
            txt += "Bidirectional Simultaneous";
        } else {
            txt += "Unknown kind of test.";
        }
        var li = new Element('li', {'class':'nettest_subtitle'}).appendText("(" + txt + ")").inject(ul);
        //txt += "ComChan Latency : " + latency + " ms !\n\n";
        // TODO : latency by wrapping ping
        
        if (local_data != null) {
            var txt = "From local to remote";
            var li = new Element('li', {'class':'nettest_subtitle'}).appendText(txt).inject(ul);
            self._insert_result_li(ul, "Bandwidth", (local_data.speed / 1000000.0).toFixed(2) + " Mbps");
            self._insert_result_li(ul, "Jitter", local_data.jitter + " ms");
            self._insert_result_li(ul, "Packet loss", local_data.percent_errors + " %");
        }
        if (remote_data != null) {
            var txt = "From remote to local \n";
            var li = new Element('li', {'class':'nettest_subtitle'}).appendText(txt).inject(ul);
            self._insert_result_li(ul, "Bandwidth", (remote_data.speed / 1000000.0).toFixed(2)  + " Mbps");
            self._insert_result_li(ul, "Jitter", remote_data.jitter + " ms");
            self._insert_result_li(ul, "Packet loss", remote_data.percent_errors + " %");
        }
        // txt += data;
        // var pre = new Element('pre').appendText(txt).inject(self.message_div);
        // TODO: check if a contact to which we are connected is selected.
        // self.start_btn.disabled = false;
		self.notify_controllers('test_done');
	}
);
