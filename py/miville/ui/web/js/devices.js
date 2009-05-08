// import Nevow.Athena

Devices = Nevow.Athena.Widget.subclass('Devices');
/**
 * This is subclass of Nevow.Athena.Widget.
 * It represents the Devices Widget.
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
 * @class DevicesWidget
 * @base Nevow.Athena.Widget
 */
Devices.methods(
    /**
     * Initialisation method.
     *
     * @constructor
     * @member Streams
     * @param node The DOM node that hold the widget associated with this class.
     */
    function __init__(self, node) {
        Devices.upcall(self, "__init__", node);
		
		// State variables
		self.contact = null;
		
		// Get elements.
		//self.list_v4l2_btn = $('deviceswidget_list_v4l2');
		//self.list_jackd_btn = $('deviceswidget_list_jackd');
		self.devices_div = $('deviceswidget_devices');
        //self.v4l2_div = $('deviceswidget_v4l2');
        //self.v4l2_input_popup = $('deviceswidget_unit');
		
		// Get string translations.
        // TODO : use only translated strings in the GUI.
		//self.start_str = $('js_deviceswidget_start').get('text'); // start string
		//self.stop_str = $('js_deviceswidget_stop').get('text'); // stop string
		
		// Set translations.
		
		// Register to the widgets communicator.
		register('deviceswidget', self);
        self.callRemote('rc_devices_list_all');

        //self.list_jackd_btn.addEvent('click', function(){
        //    self.devices_list('audio');
        //    self.devices_list_all();
        //});
        //self.list_v4l2_btn.addEvent('click', function(){
        //    self.devices_list('video');
        //});
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
		// self.upd_start_btn(event);
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
        // pass
        // see networktest.js for inspiration.
	},

	/**
     * Refreshes the list of devices.
	 * (called from the js client)
	 * 
	 * @member Devices
	 */
	function devices_list(self, driver_kind) {
        /* 
         * if (driver_name == 'jackd') {
            kind = 'audio';
        } else if (driver_name == 'v4l2') {
            kind = 'video';
        }
        */
        self.callRemote('rc_devices_list', driver_kind);
    },

	/**
     * Refreshes the list of all devices attributes.
	 * (called from the js client)
	 * 
	 * @member Devices
	 */
	function devices_list_all(self) {
        self.callRemote('rc_devices_list_all');
    },
	/**
	 * ------------------
	 * Update controllers
	 * ------------------
	 */
    // nothing here for now.

	/**
     * 
     *
	 * (called from Python)
     *
	 * @member Devices
     * @param {list} lines List of text lines to display.
	 */
	function rc_devices_list_all(self, txt, devs_list) {
        dbug.info("DEVICES: rc_devices_list_all called");
        self.devices_div.empty();
        var p = new Element('pre').appendText('' + txt).inject(self.devices_div); 
        
        // var txt = "";
        // txt += devs_list.toString();
        // if (devs_list.length == 0) {
        //     var p = new Element('p').appendText('No devices to list').inject(self.devices_div); 
        // } else {
        //     for (var dev in devs_list) {
        //         txt += dev.kind + '/';
        //         txt += dev.driver_name + '/';
        //         txt += dev.device_name + ' \n';
        //         for (var attr in dev.attributes) {
        //             txt += "    " + attr.name + "=";
        //             if (attr.kind == 'int' || attr.kind == 'float') {
        //                 txt += (attr.value).toString();
        //                 // int, string, boolean, options
        //             } else {
        //                 txt += attr.value;
        //             }
        //             if (attr.kind == 'options') {
        //                 txt += attr.options.toString();
        //             }
        //         }
        //     }
        // }
        // var p = new Element('pre').appendText('' + txt).inject(self.devices_div); 
	},
	/**
     * 
     *
	 * (called from server)
	 * 
     * See miville/network.py for the list of fields received from Python.
     *
	 * @member NetworkTesting
     * @param {string} contact_name The name of the contact . 
     * @param {string} local_data Dict/object with iperf stats.
     * @param {string} remote_data Dict/object with iperf stats.
	 */
	function rc_devices_list(self, driver_name, devices_list) {
        dbug.info("DEVICES: rc_devices_list called");
        if (driver_name == 'jackd') {
            self.jackd_div.empty();
            var p = new Element('pre').appendText('List of JACK devices: ' + devices_list).inject(self.jackd_div); 
        } else if (driver_name == 'v4l2') {
            self.v4l2_div.empty();
            var p = new Element('pre').appendText('List of V4L2 devices: ' + devices_list).inject(self.v4l2_div); 
        }
	}
);
