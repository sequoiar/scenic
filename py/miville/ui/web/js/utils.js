
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


/**
 * Event stuff
 */

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


/**
 * Return the current Mootools version number as an int.
 * 
 * @return {int} For "1.2.1" you will get 121.
 */
function get_version() {
	return parseInt(MooTools.version.split('.').join(''))	
}


/**
 * Overwrite Mootools Selectors.RegExps.splitter to fix a bug with element
 * attribute with some space character in the value.
 * 
 * Should be validated that doesn't break something
 * Original regex::
 * 
 *     (/\s*([+>~\s])\s*([a-zA-Z#.*:\[])/g)
 * 
 * @addon
 */
if (get_version() <= 121) {
	Selectors.RegExps.splitter = (/\s*([^\s~+>\[\(]+((?:\[[^\]]*\]|\([^\)]*\)))*)\s*([\s~+>])/g);
	
	Selectors.Utils.search = function(self, expression, local){
		var splitters = [];
	
			var selectors = (expression.trim() + ' ').replace(Selectors.RegExps.splitter, function(m0, m1, m2, m3){
				splitters.push(m3);
				return m1 + ':)';
			}).split(':)');
	
		var items, filtered, item;
	
			for (var i = 0, l = selectors.length - 1; i < l; i++){
	
			var selector = selectors[i];
	
			if (i == 0 && Selectors.RegExps.quick.test(selector)){
				items = self.getElementsByTagName(selector);
				continue;
			}
	
			var splitter = splitters[i - 1];
	
			var tagid = Selectors.Utils.parseTagAndID(selector);
			var tag = tagid[0], id = tagid[1];
	
			if (i == 0){
				items = Selectors.Utils.getByTagAndID(self, tag, id);
			} else {
				var uniques = {}, found = [];
				for (var j = 0, k = items.length; j < k; j++) found = Selectors.Getters[splitter](found, items[j], tag, id, uniques);
				items = found;
			}
	
			var parsed = Selectors.Utils.parseSelector(selector);
	
			if (parsed){
				filtered = [];
				for (var m = 0, n = items.length; m < n; m++){
					item = items[m];
					if (Selectors.Utils.filter(item, parsed, local)) filtered.push(item);
				}
				items = filtered;
			}
	
		}
	
		return items;
	
	}
}




