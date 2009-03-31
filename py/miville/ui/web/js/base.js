/**
 * Mockup for the web interface.
 *
 * @author etienne
 */

 window.addEvent('domready', function(){
 	
	$('c_name').setProperty('value', '');
 	
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

	$$('#global_stat div.bot_trianble')[0].addClass('bot_trianble_up');
	bot_controls['global_stat'].show();

	// Module close button
	$$('div.close').addEvent('click', function(){
		var mod_parent = this.getParent().getParent();
		mod_parent.style.display = "none";
		var thumb = new Element('div', {
 		   'styles': {
		        'padding': '0 3px',
		        'height': '20px',
				'cursor': 'pointer',
				'float': 'left',
				'margin': '5px 2px'
		    },
		    'events': {
		        'click': function(){
		            $(mod_parent.id).style.display = 'list-item';
					this.remove();
					globalSave();
		        }
		    },
			'id': 'thumb_' + mod_parent.id
		});
		thumb.appendText($E('span', this.getParent()).getText());
		thumb.injectInside('media_thumb');
		
	});

	$E('div.close', 'data_loc').fireEvent('click');
	
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
        /**
         * metwork test
         */
		if (this.value == "Start") {
			this.value = "Stop";
			net_results.hide();
			$('stat_nettest').style.visibility = 'visible';
			$('stream_but').disabled = true;
			auto_connect(true);
		} else {
			this.value = "Start";
			net_results.slideIn();
			$('stat_nettest').style.visibility = 'hidden';
			$('stream_but').disabled = false;
		}
	});
 
 	// Slide out network status
	var net_stat_pan = new Fx.Slide($('status_pan'), {mode: 'horizontal'});
	var right_pan = new Fx.Style($('right_pan'), 'margin-left');
	var modules_tab = new Fx.Style($('modules_tab'), 'left');
	net_stat_pan.hide();
//	slidePan();
	$('stat_pan_but').addEvent('click', slidePan);

	function slidePan(){
 	    /**
         * Slide out network status
         */
		var pan_size = $('modules_tab').getStyle('left').toInt();
		if (net_stat_pan.wrapper.offsetWidth == 0) {
			net_stat_pan.slideIn();
			right_pan.start($('status_pan').getSize().size.x);
			modules_tab.start(pan_size + $('status_pan').getSize().size.x);
		} else {
			net_stat_pan.slideOut();
			right_pan.start(0);
			modules_tab.start(pan_size - $('status_pan').getSize().size.x);
		};
	}
	
	// Stream button
	$('stream_but').addEvent('click', function(){
	    /**
         * Stream button
         */
		if (this.value == 'Stream') {
			this.value = 'Stop';
			gray(true);
			auto_connect(true);
			$('glob_net_stat').setStyle('display', 'inline');
			switchGraph(true);
		} else {
			this.value = 'Stream';
			gray(false);
			switchGraph(false);
			$('glob_net_stat').setStyle('display', 'none');
		}
	});
 
 	function gray(state) {
		$$('#net_test input', '#net_test select').each(function(elem){
			elem.disabled = state;
		});
	}
 
 	//Change net test for net status
	var net_test = new Fx.Style('net_test', 'opacity', {duration:200});
	var net_status = new Fx.Style('conn_status', 'opacity', {duration:2000});
	
	$('connect_but').addEvent('click', function(){
		if (this.value == "Connect") {
			auto_connect(true);
		} else {
			auto_connect(false);
		}
	});

	function auto_connect(state) {
		if (state) {
			$('connect_but').value = "Disconnect";
			$('stat_connect').style.visibility = 'visible';
		} else {
			$('connect_but').value = "Connect";
			$('stat_connect').style.visibility = 'hidden';
		}
	}
	
 
 	// Module reorder (drag and drop)
	 var sort_boxes = new Sortables($('modules'), {
	 	handles: '.mod_title'
	 }); 

	$$('div.mod_title').each(function(handle){
		handle.getChildren().addEvent('mousedown', function(){
			sort_boxes.detach();
		});
		handle.getChildren().addEvent('mouseup', function(){
			sort_boxes.attach();
		});
	});

	// Contact list selection
	var contact_rows = $$('#contacts_list table tr')
	contact_rows.addEvent('click', function(){
		contact_rows.setStyle('background-color', '#FFF');
		this.setStyle('background-color', '#e2f6ff');
		$('c_name').setProperty('value', this.getChildren()[0].getText());
		set_fields(false);
		$('connect_but').disabled = false;
		gray(false);
		$('stream_but').disabled = false;
	});
	
	// remove contact
	$('moins').addEvent('click', function(){
		$('c_name').setProperty('value', '');
		contact_rows.each(function(row){
			if (row.getStyle('background-color') == '#e2f6ff') {
				row.remove();
			}
		});
		set_fields(false);
	});

	// add contact
	$('plus').addEvent('click', function(){
		/**
         * Adds a contact
         */
        contact_rows.setStyle('background-color', '#FFF');
		$('c_name').setProperty('value', 'New Contact');
		set_fields(true);
	});
	
	$('c_name').addEvent('click', function(){
		if (this.getProperty('value') == 'New Contact') {
			this.setProperty('value', '');
		}
	});

	// edit contact
	function set_fields(state){
		/**
         * Edits a contact
         */
		var element = $('contact_edit');
		var fields = $$('#contact_field input');
		element.disabled = false;
		if (element.value == 'Edit' && state) {
			element.value = 'Save';
			fields[0].disabled = false;
			fields[1].disabled = false;
			fields[2].disabled = false;
			$('contact_field').addClass('active_field');
		} else if (element.value == 'Save' || !state) {
			element.value = 'Edit';
			fields[0].disabled = true;
			fields[1].disabled = true;
			fields[2].disabled = true;
			$('contact_field').removeClass('active_field');
		}
	}
		
	$('contact_edit').addEvent('click', function(){
		set_fields(true);
	});
	
	// Warning
	$('audio_input').addEvent('change', function(){
		var title = $$('#audio_loc span')[0];
		var warning = $('warning');
		if (this.value == 'JACK Audio') {
			title.setStyle('background-color', '#f4002b');
			warning.setStyle('display', 'block');
			this.setStyle('border', '1px solid #f4002b');
		} else {
			title.setStyle('background-color', 'transparent');
			warning.setStyle('display', 'none');
			this.setStyle('border', '1px solid #999');
		}
	});
	
	// Disable meters
	var black = new Element('div', {
		'class':'black'
	});
	var meters = $$('#status_pan img').filterByAttribute('width', '=', '50');
	meters.setStyle('display', 'none');
	meters.each(function(elem){
		black.clone().injectBefore(elem);
	});
	
	function switchGraph(state) {
		if (state) {
			$$('div.black').setStyle('display', 'none');
			meters.setStyle('display', 'block');
		} else {
			$$('div.black').setStyle('display', 'block');
			meters.setStyle('display', 'none');
		}
	}
	
	// Show save settings
	$$('div.subsettings select', 'div.subsettings input').addEvent('change', function(){
        /**
         * Shows save settings
         */
		var gp = getGrandParent(this, 10);
		var menu = $E('select', gp);
		var index = menu.selectedIndex;
		var option = menu.options[index];
		if (!option.text.contains('(modified)', '')) {
			option.text += ' (modified)';
			$ES('input.save', gp).setStyle('display', 'inline');			
		}
	});
	
	// Show global save settings
	$$('div.mod_title select').addEvent('change', globalSave);
	$E('td select', 'modules_tab').addEvent('change', globalSave);
	$$('div.close').addEvent('click', globalSave);
	
	function globalSave(state) {
		/**
         * Saves global settings
         */
        var menu = $('globalSett');
		var index = menu.selectedIndex;
		var option = menu.options[index];
		if (!option.text.contains('(modified)', '')) {
			option.text += ' (modified)';
			$ES('input.save', 'modules_tab').setStyle('display', 'inline');
		}		
	}
	
	// Hide save settings
	$$('input.save').addEvent('click', function(){
		var parent = this.getParent();
		$ES('input.save', parent).setStyle('display', 'none');
		var menu = $E('select', parent);
		var index = menu.selectedIndex;
		var option = menu.options[index];
		option.text = option.text.replace(' (modified)', '');
	});
	
	function getGrandParent(elem, level){
		var grandParent = elem;
		for (var i=0; i<level; i++){
			grandParent = grandParent.getParent();
		}
		return grandParent;
	}
	
});

