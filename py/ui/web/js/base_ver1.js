/**
 * @author etienne
 */

 window.addEvent('domready', function(){
 	
	// Media/Network toggle
	$('media_but').addEvent('click', function() {
		$('modules').setStyle('display', 'block');
		$('media_thumb').setStyle('display', 'block');
		$('network').setStyle('display', 'none');
	});
	
	$('network_but').addEvent('click', function() {
		$('modules').setStyle('display', 'none');
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
			var modPad = $('modules');
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

	// Module close button
	$$('div.close').addEvent('click', function(){
		var mod_parent = this.getParent().getParent();
		mod_parent.style.display = "none";
		new Element('div', {
 		   'styles': {
		        'width': '20px',
		        'height': '20px',
				'cursor': 'pointer'
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
 
 	// Module reorder (drag and drop)
	 new Sortables($('modules'), {
	 	handles: '.mod_title'
	 }); 
 
});

