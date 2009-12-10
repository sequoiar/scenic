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
		self.devices_div = $('deviceswidget_devices');
		// Get string translations.
        // TODO : use translated strings in the GUI.
		
		// Register to the widgets communicator.
		register('deviceswidget', self);
        self.callRemote('rc_devices_list_all');
	},
	
	/**
	 * -----------------
	 * Utility functions
	 * -----------------
	 */
	/**
	 * Notifies all the controllers (buttons/fields/etc) state when an event call
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
        // pass
	},
    
	/**
	 * ------------------
	 * Called from Client
	 * ------------------
	 */
	/**
	 * Get info from others widgets.
	 * (called from the client)
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
     * Refreshes the list of all devices attributes.
	 * (called from the js client)
	 * 
	 * @member Devices
	 */
	function devices_list_all(self) {
        self.callRemote('rc_devices_list_all');
    },

	/**
     * Changes the norm of a V4L2 video device.
	 * (called from the js client)
	 * @member Devices
	 */
	function set_norm(self, dev_name, norm_value) {
        dbug.info("set_norm" + dev_name + " " + norm_value)
        self.callRemote('rc_set_norm', dev_name, norm_value);
    },
	/**
     * Changes the input number of a V4L2 video device.
	 * (called from the js client)
	 * @member Devices
	 */
	function set_input(self, dev_name, input_value) {
        dbug.info("set_input" + dev_name + " " + input_value)
        self.callRemote('rc_set_input', dev_name, input_value);
    },
	/**
	 * ------------------
	 * Update controllers
	 * ------------------
	 */
    // nothing here for now.

	/**
	 * (called from Python)
	 * @member Devices
     * @param {devs_list} list of dict ...
	 */
	function rc_devices_list_all(self, devs_list) {
        dbug.info("DEVICES: rc_devices_list_all called");
        self.devices_div.empty();
        var table = new Element("table", {"class": "devs_table"});
        table.inject(self.devices_div);
        var has_jackd = false;
        var is_odd = true;
        var style_name = "";
        if (devs_list.length > 0) {
            devs_list.each(function(dev) 
            {
                if (dev.dr_name == "jackd") 
                {
                    has_jackd = true;
                }    
                var _tr = new Element("tr");
                var title = new Element('th', {"colspan": 3, "class": "dev_name"}).inject(_tr);
                _tr.inject(table);
                // Something like : "default" audio device using the "jackd" driver
                title.appendText('"' + dev.dev_name + '" ' + dev.dr_kind + " device using the \"" + dev.dr_name + 
"\" driver" );
                is_odd = true;
                dev.attributes.each(function(attr) 
                {
                    if (is_odd) {
                        style_name = "color_zebra";
                    } else {
                        style_name = "";
                    }
                    is_odd = ! is_odd;
                    var tr2 = new Element('tr', {"class": style_name}).inject(table);
                    var td1 = new Element("td", {"class": "dev_attr_name"}).inject(tr2).appendText(attr.name + " :");
                    var td2 = new Element("td", {"class": "dev_attr_value"}).inject(tr2);
                    // V4L2 NORM:
                    if (dev.dr_name == "v4l2" && attr.name == "norm")
                    {
                        var sel = new Element("select", {
                            "dev_name": dev.dev_name
                            });
                        //dbug.info("attr.options:");
                        //dbug.info(attr.options);
                        //dbug.info(typeof attr.options);
                        attr.options.each(function(opt) {
                            var o = new Element("option", {
                                "html": opt,
                                "selected":opt == attr.value,
                                "value": opt});
                            o.inject(sel);
                        });
                        dbug.info("adding onChange callback");
                        sel.addEvent('change', function(event) {
                            dbug.info("on change norm");
                            var dev_name = event.target.getProperty("dev_name");
                            var norm_value = event.target.value;
                            self.set_norm(dev_name, norm_value); 
                            event.target.blur(); // lose focus on form element
                        });
                        sel.inject(td2);
                    }
                    // V4L2 INPUT:
                    else if (dev.dr_name == "v4l2" && attr.name == "input")
                    {
                        var sel = new Element("select", {
                            "dev_name": dev.dev_name
                            });
                        attr.options.each(function(opt) {
                            var o = new Element("option", {
                                "html": opt,
                                "selected":opt == attr.value,
                                "value": opt});
                            o.inject(sel);
                        });
                        dbug.info("adding onChange callback");
                        sel.addEvent('change', function(event) {
                            dbug.info("on change input");
                            var dev_name = event.target.getProperty("dev_name");
                            var input_value = event.target.value;
                            self.set_input(dev_name, input_value); 
                            event.target.blur(); // lose focus on form element
                        });
                        sel.inject(td2);
                    }
                    else 
                    {
                        td2.appendText(attr.value);
                    }
                }); // end for each attr
            }); // end for each dev
        } else {
            var p = new Element('p');
            p.appendText("No device to list.");
            p.inject(self.devices_div);
        }
        if (has_jackd == false)
        {
            var blinker = new Element("blink", {
                "html":"The JACK Audio Connection Kit server (jackd) is not running !",
                "style":"color:#f00;"
                });
            blinker.inject(self.devices_div);
            var msg = new Element("p", {"html":"You should start jackd or qjackctl."});
            msg.inject(self.devices_div);
        }
	}
);
