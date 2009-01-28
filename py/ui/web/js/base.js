/**
 * @author etienne
 */

 window.addEvent('domready', function(){
 	
	$('c_name').setProperty('value', '');
 	
	// Media/Network toggle
	$('media_but').addEvent('click', function() {
		$('modules_pan').setStyle('display', 'block');
		$('media_thumb').setStyle('display', 'block');
		$('network').setStyle('display', 'none');
	});
	
	$('network_but').addEvent('click', function() {
		$('modules_pan').setStyle('display', 'none');
		$('media_thumb').setStyle('display', 'none');
		$('network').setStyle('display', 'block');
	});
	
	// Chat/Help collapse button (triangle)
	var bot_controls = new Array();
	
	$$('.bot_controls').each(function(control) {
		var topId = control.getParent().id;
		bot_controls[topId] = new Fx.Slide(control);
		bot_controls[topId].hide();
	});

	$$('div.bot_triangle').addEvent('click', function() {
		thisId = this.getParent().getParent().id;
		var control = bot_controls[thisId];
		if (thisId == "help") {
			var modPad = $('modules_pan');
		}
		if (control.wrapper.offsetHeight == 0) {
			this.addClass('bot_triangle_up');
			control.slideIn();
			if (thisId == "help") {	// Should add transition effect for the padding and scroll up
				modPad.setStyle('padding-bottom', modPad.getStyle('padding-bottom').toInt() + 50 + 'px');
			}
		} else {
			this.removeClass('bot_triangle_up');
			control.slideOut();
			if (thisId == "help") {	// Should add transition effect for the padding and scroll down
				modPad.setStyle('padding-bottom', modPad.getStyle('padding-bottom').toInt() - 50 + 'px');
			}
		}
	});

	$$('div.bot_trianble').addEvent('click', function() {
		thisId = this.getParent().getParent().id;
		var control = bot_controls[thisId];
		if (control.wrapper.offsetHeight == 0) {
			this.addClass('bot_trianble_up');
			control.slideIn();
		} else {
			this.removeClass('bot_trianble_up');
			control.slideOut();
		}
	});

	// Module close button
	$$('div.close').addEvent('click', function(){
		var mod_parent = this.getParent().getParent();
		mod_parent.style.display = "none";
		new Element('div', {
 		   'styles': {
		        'width': '20px',
		        'height': '20px',
				'cursor': 'pointer',
				'float': 'left',
				'margin': '5px 2px'
		    },
		    'events': {
		        'click': function(){
		            $(mod_parent.id).style.display = 'list-item';
					this.remove();
		        }
		    },
			'id': 'thumb_' + mod_parent.id
		}).injectInside('media_thumb');
		
	});

	// Module collapse button (triangle)
	var controls = new Array();
	
	$$('.controls').each(function(control) {
		controls[control.getParent().id] = new Fx.Slide(control);
	});

 	$$('div.triangle').addEvent('click', function(){
		var control = controls[this.getParent().getParent().id];
		if (control.wrapper.offsetHeight == 0) {
			this.removeClass('triangle_up');
			control.slideIn();
		} else {
			this.addClass('triangle_up');
			control.slideOut();
		}
	});

	// Network test
	var net_results = new Fx.Slide($('net_results'));
	net_results.hide();
	$('test_start').addEvent('click', function(){
		net_results.toggle();
	});
 
 	//Change net test for net status
	var net_test = new Fx.Style('net_test', 'opacity', {duration:200});
	$('conn_status').setStyle('opacity', '0');
	var net_status = new Fx.Style('conn_status', 'opacity', {duration:2000});
	
	$('connect_but').addEvent('click', function(){
		net_test.start(0).chain(function(){
			$('net_test').setStyle('display', 'none');
			net_status.start(100);
		});
	});
 
 	// Module reorder (drag and drop)
	 new Sortables($('modules'), {
	 	handles: '.mod_title'
	 }); 

	// Contact list selection
	var contact_rows = $$('#contacts table tr')
	contact_rows.addEvent('click', function(){
		contact_rows.setStyle('background-color', '#FFF');
		this.setStyle('background-color', '#e2f6ff');
		$('c_name').setProperty('value', this.getChildren()[0].getText());
	});
	
	// remove contact
	$('moins').addEvent('click', function(){
		$('c_name').setProperty('value', '');
		contact_rows.each(function(row){
			if (row.getStyle('background-color') == '#e2f6ff') {
				row.remove();
			}
		});
	});

	// add contact
	$('plus').addEvent('click', function(){
		contact_rows.setStyle('background-color', '#FFF');
		$('c_name').setProperty('value', 'New Contact');
		var control = bot_controls['contact_details'];
		if (control.wrapper.offsetHeight == 0) {
		$$('#contact_details div.bot_trianble')[0].addClass('bot_trianble_up');
			control.slideIn();
		}
	});
	
	$('c_name').addEvent('click', function(){
		if (this.getProperty('value') == 'New Contact') {
			this.setProperty('value', '');
		}
	});
});

