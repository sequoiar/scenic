
/**
 * Inter widgets communication.
 * 
 * @fileoverview
 */ 

/**
 * List of registered widgets.
 */
var widgets = new Hash();

/**
 * Register a widget.
 * 
 * @param {string} name The short name of the widget.
 * @param {instance} widget Class instance of the widget.
 */
function register(name, widget) {
	widgets.set(name, widget);
}

/**
 * Notify all registered widgets (except the caller) of this event.
 * 
 * @param {string} caller The short name of the caller widget.
 * @param {string} key The name of the receive information.
 * @param value The receive information.
 */
function notify(caller, key, value) {
	widgets.each(function(widget, name) {
		if (caller != name) {
			widget.update(caller, key, value);
		}
	});
}


/////////////////
/* Event stuff */
/////////////////

/**
 * Extend Element.Events to add a event for the enter/return key.
 * 
 * @addon
 */
Element.Events.enter = {
    base: 'keyup',
    condition: function(event){
        return (event.key == 'enter');
    }
};

/**
 * Extend Element.Events to add a event for the escape key.
 * 
 * @addon
 */
Element.Events.escape = {
    base: 'keyup',
    condition: function(event){
        return (event.key == 'esc');
    }
};
