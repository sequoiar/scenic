
////////////////////////////////
/* Inter widget communication */
////////////////////////////////

// List of registered widgets
var widgets = new Hash();

// Register a widget
function register(name, widget) {
	widgets.set(name, widget);
}

// Notify all registered widgets (except the caller) of this event
function notify(caller, call, data) {
	widgets.each(function(widget, name) {
		if (caller != name) {
			widget.update(caller, call, data);
		}
	});
}


/////////////////
/* Event stuff */
/////////////////

// Extend Element.Events to add a event for the enter/return key
Element.Events.enter = {
    base: 'keyup',
    condition: function(event){
        return (event.key == 'enter');
    }
};

// Extend Element.Events to add a event for the escape key
Element.Events.escape = {
    base: 'keyup',
    condition: function(event){
        return (event.key == 'esc');
    }
};
