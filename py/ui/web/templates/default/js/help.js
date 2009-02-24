
window.addEvent('domready', function(){
	
	var help_content = $('help_content');
	
	function baseHelp(){
		help_content.set('html', $('help_help').get('html'));
	}
	
	baseHelp();
	
	$$('.js_help').addEvent('mouseover', function(){
		var elem = $('help_' + this.id);
		if (elem) {
			help_content.set('html', elem.get('html'));
			console.info(elem.getProperty('url'));
		}
	});

	$$('.js_help').addEvent('mouseout', baseHelp);
});