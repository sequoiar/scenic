// import Nevow.Athena

/**
 * Help for Miville's web interface.
 *
 * @author Society for Arts and Technology
 */


Help = Nevow.Athena.Widget.subclass('Help');

/**
 * This is the base class of the Help Widget.
 *
 * @class Help
 * @base Nevow.Athena.Widget
 */
Help.methods(
    /**
     * Constructor for the help widget.
     *
     * @constructor
     * @member Help
     * @param node Please document this.
     */
    function __init__(self, node) {
        Help.upcall(self, "__init__", node);
    },  

    /**
     * Call the server side manual method.
     *
     * @member Help
     */
    function manual(self) {
        self.callRemote("rc_manual");
        return false;
    }
);


// global variable that content the current help URL (in function of the mouse context)
var help_docURL = '';

window.addEvent('domready', function() {
	
	var help_content = $('help_content');
	var help_content_base = help_content.get('html');
	
	// load the base help message and URL
	function baseHelp() {
		help_content.set('html', help_content_base);
		help_docURL = '';
	}
	
	baseHelp();
	
	// add mouseover and mouseout event listeners on elements giving contextual help (element with class 'js_help')
	var js_help = $$('.js_help');
	
	js_help.addEvent('mouseover', function() {
		// get the element that contains the help text and URL for the current element
		var elem = $('help_' + this.id);
		if (elem) {
			// get the HTML content and send it to the help window if it's not empty
			var content = elem.get('html');
			if (content) {
				help_content.set('html', content);
			}
			// set the URL if it's not null
			var docURL = elem.getProperty('url')
			if (docURL) {
				help_docURL = '/' + docURL;
			}
		}
	});

	// reload base help text and URL on mouseout
	js_help.addEvent('mouseout', baseHelp);

});

// global variable necessary to reuse always the same window in some browsers
var help_window;

// add key listener (control-H) to the window to call the help URL
window.addEvent('keydown', function(event) {
	if (event.key == 'h' && event.control) {
		help_window = window.open('https://svn.sat.qc.ca/trac/miville/wiki/Documentation' + help_docURL, 'propulseartHelp');
		help_window.focus();	// doesn't work with tab in firefox
	}
});
