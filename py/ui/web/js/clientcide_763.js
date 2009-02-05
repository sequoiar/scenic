/*
Script: Clientcide.js
	The Clientcide namespace.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var Clientcide = {
	version: ' 763',
	setAssetLocation: function(baseHref) {
		if (window.StickyWin && StickyWin.ui) {
			StickyWin.UI.refactor({
				options: {
					baseHref: baseHref + '/stickyWinHTML/'
				}
			});
			if (StickyWin.alert) {
				var CGFsimpleErrorPopup = StickyWin.alert.bind(window);
				StickyWin.alert = function(msghdr, msg, base) {
				    return CGFsimpleErrorPopup(msghdr, msg, base||baseHref + "/simple.error.popup");
				};
			}
		}
		if (window.TagMaker) {
			TagMaker = TagMaker.refactor({
			    options: {
			        baseHref: baseHref + '/tips/'
			    }
			});
		}
		if (window.ProductPicker) {
			ProductPicker.refactor({
			    options:{
			        baseHref: baseHref + '/Picker'
			    }
			});
		}

		if (window.Autocompleter) {
			var AcClientcide = {
					options: {
						baseHref: baseHref + '/autocompleter/'
					}
			};
			Autocompleter.Base.refactor(AcClientcide);
			if (Autocompleter.Ajax) {
				["Base", "Xhtml", "Json"].each(function(c){
					if(Autocompleter.Ajax[c]) Autocompleter.Ajax[c].refactor(AcClientcide);
				});
			}
			if (Autocompleter.Local) Autocompleter.Local.refactor(AcClientcide);
			if (Autocompleter.JsonP) Autocompleter.JsonP.refactor(AcClientcide);
		}

		if (window.Lightbox) {
			Lightbox.refactor({
			    options: {
			        assetBaseUrl: baseHref + '/slimbox/'
			    }
			});
		}

		if (window.Waiter) {
			Waiter.refactor({
				options: {
					baseHref: baseHref + '/waiter/'
				}
			});
		}
	},
	preLoadCss: function(){
		if (window.DatePicker) new DatePicker();
		if (window.ProductPicker) new ProductPicker();
		if (window.TagMaker) new TagMaker();
		if (window.StickyWin && StickyWin.ui) StickyWin.ui();
		if (window.StickyWin && StickyWin.pointy) StickyWin.pointy();
		Clientcide.preloaded = true;
		return true;
	},
	preloaded: false
};
(function(){
	if (!window.addEvent) return;
	var preload = function(){
		if (window.dbug) dbug.log('preloading clientcide css');
		if (!Clientcide.preloaded) Clientcide.preLoadCss();
	};
	window.addEvent('domready', preload);
	window.addEvent('load', preload);
})();
setCNETAssetBaseHref = Clientcide.setAssetLocation;

/*
Script: dbug.js
	A wrapper for Firebug console.* statements.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var dbug = {
	logged: [],	
	timers: {},
	firebug: false, 
	enabled: false, 
	log: function() {
		dbug.logged.push(arguments);
	},
	nolog: function(msg) {
		dbug.logged.push(arguments);
	},
	time: function(name){
		dbug.timers[name] = new Date().getTime();
	},
	timeEnd: function(name){
		if (dbug.timers[name]) {
			var end = new Date().getTime() - dbug.timers[name];
			dbug.timers[name] = false;
			dbug.log('%s: %s', name, end);
		} else dbug.log('no such timer: %s', name);
	},
	enable: function(silent) { 
		if(dbug.firebug) {
			try {
				dbug.enabled = true;
				dbug.log = function(){
						(console.debug || console.log).apply(console, arguments);
				};
				dbug.time = function(){
					console.time.apply(console, arguments);
				};
				dbug.timeEnd = function(){
					console.timeEnd.apply(console, arguments);
				};
				if(!silent) dbug.log('enabling dbug');
				for(var i=0;i<dbug.logged.length;i++){ dbug.log.apply(console, dbug.logged[i]); }
				dbug.logged=[];
			} catch(e) {
				dbug.enable.delay(400);
			}
		}
	},
	disable: function(){ 
		if(dbug.firebug) dbug.enabled = false;
		dbug.log = dbug.nolog;
		dbug.time = function(){};
		dbug.timeEnd = function(){};
	},
	cookie: function(set){
		var value = document.cookie.match('(?:^|;)\\s*jsdebug=([^;]*)');
		var debugCookie = value ? unescape(value[1]) : false;
		if((!$defined(set) && debugCookie != 'true') || ($defined(set) && set)) {
			dbug.enable();
			dbug.log('setting debugging cookie');
			var date = new Date();
			date.setTime(date.getTime()+(24*60*60*1000));
			document.cookie = 'jsdebug=true;expires='+date.toGMTString()+';path=/;';
		} else dbug.disableCookie();
	},
	disableCookie: function(){
		dbug.log('disabling debugging cookie');
		document.cookie = 'jsdebug=false;path=/;';
	}
};

(function(){
	var fb = typeof console != "undefined";
	var debugMethods = ['debug','info','warn','error','assert','dir','dirxml'];
	var otherMethods = ['trace','group','groupEnd','profile','profileEnd','count'];
	function set(methodList, defaultFunction) {
		for(var i = 0; i < methodList.length; i++){
			dbug[methodList[i]] = (fb && console[methodList[i]])?console[methodList[i]]:defaultFunction;
		}
	};
	set(debugMethods, dbug.log);
	set(otherMethods, function(){});
})();
if (typeof console != "undefined" && console.warn){
	dbug.firebug = true;
	var value = document.cookie.match('(?:^|;)\\s*jsdebug=([^;]*)');
	var debugCookie = value ? unescape(value[1]) : false;
	if(window.location.href.indexOf("jsdebug=true")>0 || debugCookie=='true') dbug.enable();
	if(debugCookie=='true')dbug.log('debugging cookie enabled');
	if(window.location.href.indexOf("jsdebugCookie=true")>0){
		dbug.cookie();
		if(!dbug.enabled)dbug.enable();
	}
	if(window.location.href.indexOf("jsdebugCookie=false")>0)dbug.disableCookie();
}


/*
Script: Class.Refactor.js
	Extends a class onto itself with new property, preserving any items attached to the class's namespace.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
Class.refactor = function(orig, props) {
	props = $extend($unlink(props), { Extends: orig	});
	var update = new Class(props);
	$each(orig, function(v, k) {
		update[k] = update[k] || v;
	});
	return update;
};

$extend(Class.prototype, { 
	refactor: function(props){ 
		this.prototype = Class.refactor(this, props).prototype;
		return this;
	} 
});

/*
Script: Class.Binds.js
	Automatically binds specified methods in a class to the instance of the class.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
(function(){
	var binder = function(self, binds){
		var oldInit = self.initialize;
		self.initialize = function() {
			Array.flatten(binds).each(function(binder) {
				var original = this[binder];
				this[binder] = function(){
					return original.apply(this, arguments);
				}.bind(this);
				this[binder].parent = original.parent;
			}, this);
			return oldInit.apply(this,arguments);
		};
		return self;
	};
	Class.Mutators.Binds = function(self, binds) {
		if (!self.Binds) return self;
		delete self.Binds;
		return binder(self, binds);
	};
	Class.Mutators.binds = function(self, binds) {
		if (!self.binds) return self;
		delete self['binds'];
		return binder(self, binds);
	};
})();

/*
Script: Chain.Wait.js
	Adds a method to inject pauses between chained events.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
(function(){
	var wait = {
		wait: function(duration){
			return this.chain(function(){
				this.callChain.delay($pick(duration, 500), this);
			}.bind(this));
		}
	};
	Chain.implement(wait);
	if (window.Fx) {
		Fx.implement(wait);
		['Css', 'Tween', 'Elements'].each(function(cls) {
			if (Fx[cls]) Fx[cls].implement(wait);
		});
	}

	try {
		Element.implement({
			chains: function(effects){
				$splat($pick(effects, ['tween', 'morph', 'reveal'])).each(function(effect){
					this.get(effect).setOptions({
						link:'chain'
					});
				}, this);
				return this;
			},
			pauseFx: function(duration, effect) {
				this.chains(effect).get($pick(effect, 'tween')).wait(duration);
				return this;
			}
		});
	} catch(e){}
})();

/*
Script: Occlude.js
	Prevents a class from being applied to a DOM element twice.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var Occlude = new Class({
	// usage: if(this.occlude()) return this.occluded;
	occlude: function(property, element) {
		element = $(element || this.element);
		var instance = element.retrieve(property || this.property);
		if (instance && (this.occluded === null || this.occluded)) {
			this.occluded = instance; 
		} else {
			this.occluded = false;
			element.store(property || this.property, this);
		}
		return this.occluded||false;
	}
});

/*
Script: ToElement.js
	Defines the toElement method for a class.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var ToElement = new Class({
	toElement: function(){
		return this.element;
	}
});

/*
Script: Browser.Extras.js
	Extends the Window native object to include methods useful in managing the window location and urls.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

$extend(Browser, {
	getHost:function(url){
		url = $pick(url, window.location.href);
		var host = url;
		if(url.test('http://')){
			url = url.substring(url.indexOf('http://')+7,url.length);
			if(url.test(':')) url = url.substring(0, url.indexOf(":"));
			if(url.test('/')) return url.substring(0,url.indexOf('/'));
			return url;
		}
		return false;
	},
	getQueryStringValue: function(key, url) {
		try { 
			return Browser.getQueryStringValues(url)[key];
		}catch(e){return null;}
	},
	getQueryStringValues: function(url){
		var qs = $pick(url, window.location.search, '').split('?')[1]; //get the query string
		if (!$chk(qs)) return {};
		if (qs.test('#')) qs = qs.substring(0, qs.indexOf('#'));
		try {
      if (qs) return qs.parseQuery();
		} catch(e){
			return null;
		}
		return {}; //if there isn't one, return null
	},
	getPort: function(url) {
		url = $pick(url, window.location.href);
		var re = new RegExp(':([0-9]{4})');
		var m = re.exec(url);
	  if (m == null) return false;
	  else {
			var port = false;
			m.each(function(val){
				if($chk(parseInt(val))) port = val;
			});
	  }
		return port;
	},
  redraw: function(element){
    var n = document.createTextNode(' ');
    this.adopt(n);
    (function(){n.dispose()}).delay(1);
    return this;
	}
});
window.addEvent('domready', function(){
	var count = 0;
	//this is in case domready fires before string.extras loads
	function setQs(){
		function retry(){
			count++;
			if (count < 20) setQs.delay(50);
		}; 
		try {
			if (!Browser.getQueryStringValues()) retry();
			else Browser.qs = Browser.getQueryStringValues();
		} catch(e){
			retry();
		}
	}
	setQs();
});

/*
Script: FixPNG.js
	Extends the Browser hash object to include methods useful in managing the window location and urls.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
$extend(Browser, {
	fixPNG: function(el) {
		try {
			if (Browser.Engine.trident4){
				el = $(el);
				if (!el) return el;
				if (el.get('tag') == "img" && el.get('src').test(".png")) {
					var vis = el.isVisible();
					try { //safari sometimes crashes here, so catch it
						dim = el.getSize();
					}catch(e){}
					if(!vis){
						var before = {};
						//use this method instead of getStyles 
						['visibility', 'display', 'position'].each(function(style){
							before[style] = this.style[style]||'';
						}, this);
						//this.getStyles('visibility', 'display', 'position');
						this.setStyles({
							visibility: 'hidden',
							display: 'block',
							position:'absolute'
						});
						dim = el.getSize(); //works now, because the display isn't none
						this.setStyles(before); //put it back where it was
						el.hide();
					}
					var replacement = new Element('span', {
						id:(el.id)?el.id:'',
						'class':(el.className)?el.className:'',
						title:(el.title)?el.title:(el.alt)?el.alt:'',
						styles: {
							display: vis?'inline-block':'none',
							width: dim.x,
							height: dim.y,
							filter: "progid:DXImageTransform.Microsoft.AlphaImageLoader (src='" 
								+ el.src + "', sizingMethod='scale');"
						},
						src: el.src
					});
					if(el.style.cssText) {
						try {
							var styles = {};
							var s = el.style.cssText.split(';');
							s.each(function(style){
								var n = style.split(':');
								styles[n[0]] = n[1];
							});
							replacement.setStyle(styles);
						} catch(e){ dbug.log('fixPNG1: ', e)}
					}
					if(replacement.cloneEvents) replacement.cloneEvents(el);
					replacement.replaces(el);
				} else if (el.get('tag') != "img") {
				 	var imgURL = el.getStyle('background-image');
				 	if (imgURL.test(/\((.+)\)/)){
				 		el.setStyles({
				 			background: '',
				 			filter: "progid:DXImageTransform.Microsoft.AlphaImageLoader(enabled='true', sizingMethod='crop', src=" + imgURL.match(/\((.+)\)/)[1] + ")"
				 		});
				 	};
				}
			}
		} catch(e) {dbug.log('fixPNG2: ', e)}
	},
  pngTest: /\.png$/, // saves recreating the regex repeatedly
  scanForPngs: function(el, className) {
    className = className||'fixPNG';
    //TODO: should this also be testing the css background-image property for pngs?
    //Q: should it return an array of all those it has tweaked?
    if (document.getElements){ // more efficient but requires 'selectors'
      el = $(el||document.body);
      el.getElements('img[src$=.png]').addClass(className);
    } else { // scan the whole page
      var els = $$('img').each(function(img) {
        if (Browser.pngTest(img.src)){
          img.addClass(className);
        }
      });
    }
  }
});
if(Browser.Engine.trident4) window.addEvent('domready', function(){$$('img.fixPNG').each(Browser.fixPNG)});


/*
Script: IframeShim.js
	Defines IframeShim, a class for obscuring select lists and flash objects in IE.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/	
var IframeShim = new Class({
	Implements: [Options, Events],
	options: {
		name: '',
		className:'iframeShim',
		display:false,
		zindex: null,
		margin: 0,
		offset: {
			x: 0,
			y: 0
		},
		browsers: (Browser.Engine.trident4 || (Browser.Engine.gecko && !Browser.Engine.gecko19 && Browser.Platform.mac))
	},
	initialize: function (element, options){
		this.setOptions(options);
		//legacy
		if(this.options.offset && this.options.offset.top) this.options.offset.y = this.options.offset.top;
		if(this.options.offset && this.options.offset.left) this.options.offset.x = this.options.offset.left;
		this.element = $(element);
		this.makeShim();
		return;
	},
	makeShim: function(){
		this.shim = new Element('iframe');
		this.id = this.options.name || new Date().getTime() + "_shim";
		if(!this.options.browsers) return;
		if(this.element.getStyle('z-Index').toInt()<1 || isNaN(this.element.getStyle('z-Index').toInt()))
			this.element.setStyle('z-Index',5);
		var z = this.element.getStyle('z-Index')-1;
		
		if($chk(this.options.zindex) && 
			 this.element.getStyle('z-Index').toInt() > this.options.zindex)
			 z = this.options.zindex;
			
 		this.shim.setStyles({
			'position': 'absolute',
			'zIndex': z,
			'border': 'none',
			'filter': 'progid:DXImageTransform.Microsoft.Alpha(style=0,opacity=0)'
		}).setProperties({
			'src': (window.location.protocol == 'https') ? '://0' : 'javascript:void(0)',
			'frameborder':'0',
			'scrolling':'no',
			'id':this.id
		}).addClass(this.options.className);
		
		this.element.store('shim', this);

		var inject = function(){
			this.shim.inject(this.element, 'after');
			if(this.options.display) this.show();
			else this.hide();
			this.fireEvent('onInject');
		};
		if(this.options.browsers){
			if(Browser.Engine.trident && !IframeShim.ready) {
				window.addEvent('load', inject.bind(this));
			} else {
				inject.run(null, this);
			}
		}
	},
	position: function(shim){
		if(!this.options.browsers || !IframeShim.ready) return this;
		var putItBack = this.element.expose();
		var size = this.element.getSize();
		putItBack();
		if($type(this.options.margin)){
			size.x = size.x-(this.options.margin*2);
			size.y = size.y-(this.options.margin*2);
			this.options.offset.x += this.options.margin; 
			this.options.offset.y += this.options.margin;
		}
 		this.shim.setStyles({
			'width': size.x,
			'height': size.y
		}).setPosition({
			relativeTo: this.element,
			offset: this.options.offset
		});
		return this;
	},
	hide: function(){
		if(this.options.browsers) this.shim.setStyle('display','none');
		return this;
	},
	show: function(){
		if(!this.options.browsers) return this;
		this.shim.setStyle('display','block');
		return this.position();
	},
	dispose: function(){
		if(this.options.browsers) this.shim.dispose();
		return this;
	}
});
window.addEvent('load', function(){
	IframeShim.ready = true;
});


/*
Script: Popup.js
	Defines the Popup class useful for making popup windows.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

Browser.Popup = new Class({
	Implements:[Options, Events],
	options: {
		width: 500,
		height: 300,
		x: 50,
		y: 50,
		toolbar: 0,
		location: 0,
		directories: 0,
		status: 0,
		scrollbars: 'auto',
		resizable: 1,
		name: 'popup'
//	onBlock: $empty
	},
	initialize: function(url, options){
		this.url = url || false;
		this.setOptions(options);
		if(this.url) this.openWin();
	},
	openWin: function(url){
		url = url || this.url;
		var options = 'toolbar='+this.options.toolbar+
			',location='+this.options.location+
			',directories='+this.options.directories+
			',status='+this.options.status+
			',scrollbars='+this.options.scrollbars+
			',resizable='+this.options.resizable+
			',width='+this.options.width+
			',height='+this.options.height+
			',top='+this.options.y+
			',left='+this.options.x;
		this.window = window.open(url, this.options.name, options);
		if (!this.window) {
			this.window = window.open('', this.options.name, options);
			this.window.location.href = url;
		}
		this.focus.delay(100, this);
		return this;
	},
	focus: function(){
		if (this.window) this.window.focus();
		else if (this.focusTries<10) this.focus.delay(100, this); //try again
		else {
			this.blocked = true;
			this.fireEvent('onBlock');
		}
		return this;
	},
	focusTries: 0,
	blocked: null,
	close: function(){
		this.window.close();
		return this;
	}
});

/*
Script: Date.js
	Extends the Date native object to include methods useful in managing dates.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

new Native({name: 'Date', initialize: Date, protect: true});
['now','parse','UTC'].each(function(method){
	Native.genericize(Date, method, true);
});
Date.$Methods = new Hash();
["Date", "Day", "FullYear", "Hours", "Milliseconds", "Minutes", "Month", "Seconds", "Time", "TimezoneOffset", 
	"Week", "Timezone", "GMTOffset", "DayOfYear", "LastMonth", "UTCDate", "UTCDay", "UTCFullYear",
	"AMPM", "UTCHours", "UTCMilliseconds", "UTCMinutes", "UTCMonth", "UTCSeconds"].each(function(method) {
	Date.$Methods.set(method.toLowerCase(), method);
});
$each({
	ms: "Milliseconds",
	year: "FullYear",
	min: "Minutes",
	mo: "Month",
	sec: "Seconds",
	hr: "Hours"
}, function(value, key){
	Date.$Methods.set(key, value);
});


Date.implement({
	set: function(key, value) {
		key = key.toLowerCase();
		var m = Date.$Methods;
		if (m.has(key)) this['set'+m.get(key)](value);
		return this;
	},
	get: function(key) {
		key = key.toLowerCase();
		var m = Date.$Methods;
		if (m.has(key)) return this['get'+m.get(key)]();
		return null;
	},
	clone: function() {
		return new Date(this.get('time'));
	},
	increment: function(interval, times) {
		return this.multiply(interval, times);
	},
	decrement: function(interval, times) {
		return this.multiply(interval, times, false);
	},
	multiply: function(interval, times, increment){
		interval = interval || 'day';
		times = $pick(times, 1);
		increment = $pick(increment, true);
		var multiplier = increment?1:-1;
		var month = this.format("%m").toInt()-1;
		var year = this.format("%Y").toInt();
		var time = this.get('time');
		var offset = 0;
		switch (interval) {
				case 'year':
					times.times(function(val) {
						if (Date.isLeapYear(year+val) && month > 1 && multiplier > 0) val++;
						if (Date.isLeapYear(year+val) && month <= 1 && multiplier < 0) val--;
						offset += Date.$units.year(year+val);
					});
					break;
				case 'month':
					times.times(function(val){
						if (multiplier < 0) val++;
						var mo = month+(val*multiplier);
						var year = year;
						if (mo < 0) {
							year--;
							mo = 12+mo;
						}
						if (mo > 11 || mo < 0) {
							year += (mo/12).toInt()*multiplier;
							mo = mo%12;
						}
						offset += Date.$units.month(mo, year);
					});
					break;
				case 'day':
					return this.set('date', this.get('date')+(multiplier*times));
				default:
					offset = Date.$units[interval]()*times;
					break;
		}
		this.set('time', time+(offset*multiplier));
		return this;
	},
	isLeapYear: function() {
		return Date.isLeapYear(this.get('year'));
	},
	clearTime: function() {
		['hr', 'min', 'sec', 'ms'].each(function(t){
			this.set(t, 0);
		}, this);
		return this;
	},
	diff: function(d, resolution) {
		resolution = resolution || 'day';
		if($type(d) == 'string') d = Date.parse(d);
		switch (resolution) {
			case 'year':
				return d.format("%Y").toInt() - this.format("%Y").toInt();
				break;
			case 'month':
				var months = (d.format("%Y").toInt() - this.format("%Y").toInt())*12;
				return months + d.format("%m").toInt() - this.format("%m").toInt();
				break;
			default:
				var diff = d.get('time') - this.get('time');
				if (diff < 0 && Date.$units[resolution]() > (-1*(diff))) return 0;
				else if (diff >= 0 && diff < Date.$units[resolution]()) return 0;
				return ((d.get('time') - this.get('time')) / Date.$units[resolution]()).round();
		}
	},
	getWeek: function() {
		var day = (new Date(this.get('year'), 0, 1)).get('date');
		return Math.round((this.get('dayofyear') + (day > 3 ? day - 4 : day + 3)) / 7);
	},
	getTimezone: function() {
		return this.toString()
			.replace(/^.*? ([A-Z]{3}).[0-9]{4}.*$/, '$1')
			.replace(/^.*?\(([A-Z])[a-z]+ ([A-Z])[a-z]+ ([A-Z])[a-z]+\)$/, '$1$2$3');
	},
	getGMTOffset: function() {
		var off = this.get('timezoneOffset');
		return ((off > 0) ? '-' : '+')
			+ Math.floor(Math.abs(off) / 60).zeroise(2)
			+ (off % 60).zeroise(2);
	},
	parse: function(str) {
		this.set('time', Date.parse(str));
		return this;
	},
	format: function(f) {
		f = f || "%x %X";
		if (!this.valueOf()) return 'invalid date';
		//replace short-hand with actual format
		if (Date.$formats[f.toLowerCase()]) f = Date.$formats[f.toLowerCase()];
		var d = this;
		return f.replace(/\%([aAbBcdHIjmMpSUWwxXyYTZ])/g,
			function($1, $2) {
				switch ($2) {
					case 'a': return Date.$days[d.get('day')].substr(0, 3);
					case 'A': return Date.$days[d.get('day')];
					case 'b': return Date.$months[d.get('month')].substr(0, 3);
					case 'B': return Date.$months[d.get('month')];
					case 'c': return d.toString();
					case 'd': return d.get('date').zeroise(2);
					case 'H': return d.get('hr').zeroise(2);
					case 'I': return ((d.get('hr') % 12) || 12);
					case 'j': return d.get('dayofyear').zeroise(3);
					case 'm': return (d.get('mo') + 1).zeroise(2);
					case 'M': return d.get('min').zeroise(2);
					case 'p': return d.get('hr') < 12 ? 'AM' : 'PM';
					case 'S': return d.get('seconds').zeroise(2);
					case 'U': return d.get('week').zeroise(2);
					case 'W': throw new Error('%W is not supported yet');
					case 'w': return d.get('day');
					case 'x': 
						var c = Date.$cultures[Date.$culture];
						//return d.format("%{0}{3}%{1}{3}%{2}".substitute(c.map(function(s){return s.substr(0,1)}))); //grr!
						return d.format('%' + c[0].substr(0,1) +
							c[3] + '%' + c[1].substr(0,1) +
							c[3] + '%' + c[2].substr(0,1).toUpperCase());
					case 'X': return d.format('%I:%M%p');
					case 'y': return d.get('year').toString().substr(2);
					case 'Y': return d.get('year');
					case 'T': return d.get('GMTOffset');
					case 'Z': return d.get('Timezone');
					case '%': return '%';
				}
				return $2;
			}
		);
	},
	setAMPM: function(ampm){
		ampm = ampm.toUpperCase();
		if (this.format("%H").toInt() > 11 && ampm == "AM") 
			return this.decrement('hour', 12);
		else if (this.format("%H").toInt() < 12 && ampm == "PM")
			return this.increment('hour', 12);
		return this;
	}
});

Date.prototype.compare = Date.prototype.diff;
Date.prototype.strftime = Date.prototype.format;

Date.$nativeParse = Date.parse;

$extend(Date, {
	$months: ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'],
	$days: ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'],
	$daysInMonth: function(monthIndex, year) {
		if (Date.isLeapYear(year.toInt()) && monthIndex === 1) return 29;
		return [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31][monthIndex];
	},
	$epoch: -1,
	$era: -2,
	$units: {
		ms: function(){return 1},
		second: function(){return 1000},
		minute: function(){return 60000},
		hour: function(){return 3600000},
		day: function(){return 86400000},
		week: function(){return 608400000},
		month: function(monthIndex, year) {
			var d = new Date();
			return Date.$daysInMonth($pick(monthIndex,d.format("%m").toInt()), $pick(year,d.format("%Y").toInt())) * 86400000;
		},
		year: function(year){
			year = year || new Date().format("%Y").toInt();
			return Date.isLeapYear(year.toInt())?31622400000:31536000000;
		}
	},
	$formats: {
		db: '%Y-%m-%d %H:%M:%S',
		compact: '%Y%m%dT%H%M%S',
		iso8601: '%Y-%m-%dT%H:%M:%S%T',
		rfc822: '%a, %d %b %Y %H:%M:%S %Z',
		'short': '%d %b %H:%M',
		'long': '%B %d, %Y %H:%M'
	},
	
	isLeapYear: function(yr) {
		return new Date(yr,1,29).getDate()==29;
	},

	parseUTC: function(value){
		var localDate = new Date(value);
		var utcSeconds = Date.UTC(localDate.get('year'), localDate.get('mo'),
		localDate.get('date'), localDate.get('hr'), localDate.get('min'), localDate.get('sec'));
		return new Date(utcSeconds);
	},
	
	parse: function(from) {
		var type = $type(from);
		if (type == 'number') return new Date(from);
		if (type != 'string') return from;
		if (!from.length) return null;
		for (var i = 0, j = Date.$parsePatterns.length; i < j; i++) {
			var r = Date.$parsePatterns[i].re.exec(from);
			if (r) {
				try {
					return Date.$parsePatterns[i].handler(r);
				} catch(e) {
					dbug.log('date parse error: ', e);
					return null;
				}
			}
		}
		return new Date(Date.$nativeParse(from));
	},

	parseMonth: function(month, num) {
		var ret = -1;
		switch ($type(month)) {
			case 'object':
				ret = Date.$months[month.get('mo')];
				break;
			case 'number':
				ret = Date.$months[month - 1] || false;
				if (!ret) throw new Error('Invalid month index value must be between 1 and 12:' + index);
				break;
			case 'string':
				var match = Date.$months.filter(function(name) {
					return this.test(name);
				}, new RegExp('^' + month, 'i'));
				if (!match.length) throw new Error('Invalid month string');
				if (match.length > 1) throw new Error('Ambiguous month');
				ret = match[0];
		}
		return (num) ? Date.$months.indexOf(ret) : ret;
	},

	parseDay: function(day, num) {
		var ret = -1;
		switch ($type(day)) {
			case 'number':
				ret = Date.$days[day - 1] || false;
				if (!ret) throw new Error('Invalid day index value must be between 1 and 7');
				break;
			case 'string':
				var match = Date.$days.filter(function(name) {
					return this.test(name);
				}, new RegExp('^' + day, 'i'));
				if (!match.length) throw new Error('Invalid day string');
				if (match.length > 1) throw new Error('Ambiguous day');
				ret = match[0];
		}
		return (num) ? Date.$days.indexOf(ret) : ret;
	},
	
	fixY2K: function(d){
		if (!isNaN(d)) {
			var newDate = new Date(d);
			if (newDate.get('year') < 2000 && d.toString().indexOf(newDate.get('year')) < 0) {
				newDate.increment('year', 100);
			}
			return newDate;
		} else return d;
	},

	$cultures: {
		'US': ['month', 'date', 'year', '/'],
		'GB': ['date', 'month', 'year', '/']
	},

	$culture: 'US',
	
	$language: 'enUS',
	
	$cIndex: function(unit){
		return Date.$cultures[Date.$culture].indexOf(unit)+1;
	},

	$parsePatterns: [
		{
			//"12.31.08", "12-31-08", "12/31/08", "12.31.2008", "12-31-2008", "12/31/2008"
			re: /^(\d{1,2})[\.\-\/](\d{1,2})[\.\-\/](\d{2,4})$/,
			handler: function(bits){
				var d = new Date();
				var culture = Date.$cultures[Date.$culture];
				d.set('year', bits[Date.$cIndex('year')]);
				d.set('month', bits[Date.$cIndex('month')] - 1);
				d.set('date', bits[Date.$cIndex('date')]);
				return Date.fixY2K(d);
			}
		},
		//"12.31.08", "12-31-08", "12/31/08", "12.31.2008", "12-31-2008", "12/31/2008"
		//above plus "10:45pm" ex: 12.31.08 10:45pm
		{
			re: /^(\d{1,2})[\.\-\/](\d{1,2})[\.\-\/](\d{2,4})\s(\d{1,2}):(\d{1,2})(\w{2})$/,
			handler: function(bits){
				var d = new Date();
				d.set('year', bits[Date.$cIndex('year')]);
				d.set('month', bits[Date.$cIndex('month')] - 1);
				d.set('date', bits[Date.$cIndex('date')]);
				d.set('hr', bits[4]);
				d.set('min', bits[5]);
				d.set('ampm', bits[6]);
				return Date.fixY2K(d);
			}
		},
		{
			//"12.31.08 11:59:59", "12-31-08 11:59:59", "12/31/08 11:59:59", "12.31.2008 11:59:59", "12-31-2008 11:59:59", "12/31/2008 11:59:59"
			re: /^(\d{1,2})[\.\-\/](\d{1,2})[\.\-\/](\d{2,4})\s(\d{1,2}):(\d{1,2}):(\d{1,2})/,
			handler: function(bits) {
				var d = new Date();
				var culture = Date.$cultures[Date.$culture];
				d.set('year', bits[Date.$cIndex('year')]);
				d.set('month', bits[Date.$cIndex('month')] - 1);
				d.set('date', bits[Date.$cIndex('date')]);
				d.set('hours', bits[4]);
				d.set('minutes', bits[5]);
				d.set('seconds', bits[6]);
				return Date.fixY2K(d);
			}
		}
	]
});

Number.implement({
	zeroise: function(length) {
		return String(this).zeroise(length);
	}
});

String.implement({
	repeat: function(times) {
		var ret = [];
		for (var i = 0; i < times; i++) ret.push(this);
		return ret.join('');
	},
	zeroise: function(length) {
		return '0'.repeat(length - this.length) + this;
	}

});


/*
Script: Date.Extras.js
	Extends the Date native object to include extra methods (on top of those in Date.js).

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

["LastDayOfMonth", "Ordinal"].each(function(method) {
	Date.$Methods.set(method.toLowerCase(), method);
});

Date.implement({
	timeDiffInWords: function(){
		var relative_to = (arguments.length > 0) ? arguments[1] : new Date();
		return Date.distanceOfTimeInWords(this, relative_to);
	},
	getOrdinal: function() {
		var test = this.get('date');
		return (test > 3 && test < 21) ? 'th' : ['th', 'st', 'nd', 'rd', 'th'][Math.min(test % 10, 4)];
	},
	getDayOfYear: function() {
		return ((Date.UTC(this.getFullYear(), this.getMonth(), this.getDate() + 1, 0, 0, 0)
			- Date.UTC(this.getFullYear(), 0, 1, 0, 0, 0) ) / Date.$units.day());
	},
	getLastDayOfMonth: function() {
		var ret = this.clone();
		ret.setMonth(ret.getMonth() + 1, 0);
		return ret.getDate();
	}
});

Date.alias('timeDiffInWords', 'timeAgoInWords');

$extend(Date, {
	distanceOfTimeInWords: function(fromTime, toTime) {
		return Date.getTimePhrase(((toTime.getTime() - fromTime.getTime()) / 1000).toInt(), fromTime, toTime);
	},
	getTimePhrase: function(delta, fromTime, toTime) {
		var res = Date.$resources[Date.$language]; //saving bytes
		var getPhrase = function(){
			if (delta >= 0) {
				if (delta < 60) {
					return res.ago.lessThanMinute;
				} else if (delta < 120) {
					return res.ago.minute;
				} else if (delta < (45*60)) {
					delta = (delta / 60).round();
					return res.ago.minutes;
				} else if (delta < (90*60)) {
					return res.ago.hour;
				} else if (delta < (24*60*60)) {
					delta = (delta / 3600).round();
					return res.ago.hours;
				} else if (delta < (48*60*60)) {
					return res.ago.day;
				} else {
					delta = (delta / 86400).round();
					return res.ago.days;
				}
			}
			if (delta < 0) {
				delta = delta * -1;
				if (delta < 60) {
					return res.until.lessThanMinute;
				} else if (delta < 120) {
					return res.until.minute;
				} else if (delta < (45*60)) {
					delta = (delta / 60).round();
					return res.until.minutes;
				} else if (delta < (90*60)) {
					return res.until.hour;
				} else if (delta < (24*60*60)) {
					delta = (delta / 3600).round();
					return res.until.hours;
				} else if (delta < (48*60*60)) {
					return res.until.day;
				} else  {
					delta = (delta / 86400).round();
					return res.until.days;
				}
			}
		};
		return getPhrase().substitute({delta: delta});
	}
});

Date.$resources = {
	enUS: {
		ago: {
			lessThanMinute: 'less than a minute ago',
			minute: 'about a minute ago',
			minutes: '{delta} minutes ago',
			hour: 'about an hour ago',
			hours: 'about {delta} hours ago',
			day: '1 day ago',
			days: '{delta} days ago'
		},
		until: {
			lessThanMinute: 'less than a minute from now',
			minute: 'about a minute from now',
			minutes: '{delta} minutes from now',
			hour: 'about an hour from now',
			hours: 'about {delta} hours from now',
			day: '1 day from now',
			days: '{delta} days from now'
		}
	}
};

Date.$parsePatterns.extend([
	{
		//"1999-12-31 23:59:59"
		re: /^(\d{4})[\.\-\/](\d{1,2})[\.\-\/](\d{2,4})\s(\d{1,2}):(\d{1,2}):(\d{1,2})/,
		handler: function(bits) {			
			var d = new Date();
			var culture = Date.$cultures[Date.$culture];
			d.set('year', bits[1]);
			d.set('month', bits[2] - 1);
			d.set('date', bits[3]);
			d.set('hours', bits[4]);
			d.set('minutes', bits[5]);
			d.set('seconds', bits[6]);
			return d;
		}
	},
	{		
		// yyyy-mm-ddTHH:MM:SS-0500 (ISO8601) i.e.2007-04-17T23:15:22Z
		// inspired by: http://delete.me.uk/2005/03/iso8601.html
		re: /^(\d{4})(?:-?(\d{2})(?:-?(\d{2})(?:[T ](\d{2})(?::?(\d{2})(?::?(\d{2})(?:\.(\d+))?)?)?(?:Z|(?:([-+])(\d{2})(?::?(\d{2}))?)?)?)?)?)?$/,
		handler: function(bits) {
			var offset = 0;
			var d = new Date(bits[1], 0, 1);
			if (bits[2]) d.setMonth(bits[2] - 1);
			if (bits[3]) d.setDate(bits[3]);
			if (bits[4]) d.setHours(bits[4]);
			if (bits[5]) d.setMinutes(bits[5]);
			if (bits[6]) d.setSeconds(bits[6]);
			if (bits[7]) d.setMilliseconds(('0.' + bits[7]).toInt() * 1000);
			if (bits[9]) {
				offset = (bits[9].toInt() * 60) + bits[10].toInt();
				offset *= ((bits[8] == '-') ? 1 : -1);
			}
			//offset -= d.getTimezoneOffset();
			d.setTime((d * 1) + (offset * 60 * 1000).toInt());
			return d;
		}
	}, {
		//"today"
		re: /^tod/i,
		handler: function() {
			return new Date();
		}
	}, {
		//"tomorow"
		re: /^tom/i,
		handler: function() {
			return new Date().increment();
		}
	}, {
		//"yesterday"
		re: /^yes/i,
		handler: function() {
			return new Date().decrement();
		}
	}, {
		//4th, 23rd
		re: /^(\d{1,2})(st|nd|rd|th)?$/i,
		handler: function(bits) {
			var d = new Date();
			d.setDate(bits[1].toInt());
			return d;
		}
	}, {
		//4th Jan, 23rd May
		re: /^(\d{1,2})(?:st|nd|rd|th)? (\w+)$/i,
		handler: function(bits) {
			var d = new Date();
			d.setMonth(Date.parseMonth(bits[2], true), bits[1].toInt());
			return d;
		}
	}, {
		//4th Jan 2000, 23rd May 2004
		re: /^(\d{1,2})(?:st|nd|rd|th)? (\w+),? (\d{4})$/i,
		handler: function(bits) {
			var d = new Date();
			d.setMonth(Date.parseMonth(bits[2], true), bits[1].toInt());
			d.setYear(bits[3]);
			return d;
		}
	}, {
		//Jan 4th
		re: /^(\w+) (\d{1,2})(?:st|nd|rd|th)?,? (\d{4})$/i,
		handler: function(bits) {
			var d = new Date();
			d.setMonth(Date.parseMonth(bits[1], true), bits[2].toInt());
			d.setYear(bits[3]);
			return d;
		}
	}, {
		//Jan 4th 2003
		re: /^next (\w+)$/i,
		handler: function(bits) {
			var d = new Date();
			var day = d.getDay();
			var newDay = Date.parseDay(bits[1], true);
			var addDays = newDay - day;
			if (newDay <= day) {
				addDays += 7;
			}
			d.setDate(d.getDate() + addDays);
			return d;
		}
	}, {
		//4 May 08:12
		re: /^\d+\s[a-zA-z]..\s\d.\:\d.$/,
		handler: function(bits){
			var d = new Date();
			bits = bits[0].split(" ");
			d.setDate(bits[0]);
			var m;
			Date.$months.each(function(mo, i){
				if (new RegExp("^"+bits[1]).test(mo)) m = i;
			});
			d.setMonth(m);
			d.setHours(bits[2].split(":")[0]);
			d.setMinutes(bits[2].split(":")[1]);
			d.setMilliseconds(0);
			return d;
		}
	},
	{
		re: /^last (\w+)$/i,
		handler: function(bits) {
			return Date.parse('next ' + bits[0]).decrement('day', 7);
		}
	}
]);


/*
Script: Hash.Extras.js
	Extends the Hash native object to include getFromPath which allows a path notation to child elements.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

Hash.implement({
	getFromPath: function(notation) {
		var source = this.getClean();
		notation.replace(/\[([^\]]+)\]|\.([^.[]+)|[^[.]+/g, function(match) {
			if (!source) return;
			var prop = arguments[2] || arguments[1] || arguments[0];
			source = (prop in source) ? source[prop] : null;
			return match;
		});
		return source;
	},
	cleanValues: function(method){
		method = method||$defined;
		this.each(function(v, k){
			if (!method(v)) this.erase(k);
		}, this);
		return this;
	},
	run: function(){
		var args = $arguments;
		this.each(function(v, k){
			if ($type(v) == "function") v.run(args);
		});
	}
});

/*
Script: String.Extras.js
	Extends the String native object to include methods useful in managing various kinds of strings (query strings, urls, html, etc).

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
String.implement({
	stripTags: function() {
		return this.replace(/<\/?[^>]+>/gi, '');
	},
	parseQuery: function(encodeKeys, encodeValues) {
		encodeKeys = $pick(encodeKeys, true);
		encodeValues = $pick(encodeValues, true);
		var vars = this.split(/[&;]/);
		var rs = {};
		if (vars.length) vars.each(function(val) {
			var keys = val.split('=');
			if (keys.length && keys.length == 2) {
				rs[(encodeKeys)?encodeURIComponent(keys[0]):keys[0]] = (encodeValues)?encodeURIComponent(keys[1]):keys[1];
			}
		});
		return rs;
	},
	tidy: function() {
		var txt = this.toString();
		$each({
			"[\xa0\u2002\u2003\u2009]": " ",
			"\xb7": "*",
			"[\u2018\u2019]": "'",
			"[\u201c\u201d]": '"',
			"\u2026": "...",
			"\u2013": "-",
			"\u2014": "--",
			"\uFFFD": "&raquo;"
		}, function(value, key){
			txt = txt.replace(new RegExp(key, 'g'), value);
		});
		return txt;
	},
	cleanQueryString: function(method){
		return this.split("&").filter(method||function(set){
			return $chk(set.split("=")[1]);
		}).join("&");
	},
	findAllEmails: function(){
			return this.match(new RegExp("[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*@(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?", "gi")) || [];
	}
});


/*
Script: Element.Forms.js
	Extends the Element native object to include methods useful in managing inputs.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
Element.implement({
	tidy: function(){
		this.set('value', this.get('value').tidy());
	},
	getTextInRange: function(start, end) {
		return this.get('value').substring(start, end);
	},
	getSelectedText: function() {
		if(Browser.Engine.trident) return document.selection.createRange().text;
		return this.get('value').substring(this.getSelectionStart(), this.getSelectionEnd());
	},
	getIERanges: function(){
		this.focus();
		var range = document.selection.createRange();
		var re = this.createTextRange();
		var dupe = re.duplicate();
		re.moveToBookmark(range.getBookmark());
		dupe.setEndPoint('EndToStart', re);
		return { start: dupe.text.length, end: dupe.text.length + range.text.length, length: range.text.length, text: range.text };
	},
	getSelectionStart: function() {
		if(Browser.Engine.trident) return this.getIERanges().start;
		return this.selectionStart;
	},
	getSelectionEnd: function() {
		if(Browser.Engine.trident) return this.getIERanges().end;
		return this.selectionEnd;
	},
	getSelectedRange: function() {
		return {
			start: this.getSelectionStart(),
			end: this.getSelectionEnd()
		}
	},
	setCaretPosition: function(pos) {
		if(pos == 'end') pos = this.get('value').length;
		this.selectRange(pos, pos);
		return this;
	},
	getCaretPosition: function() {
		return this.getSelectedRange().start;
	},
	selectRange: function(start, end) {
		this.focus();
		if(Browser.Engine.trident) {
			var range = this.createTextRange();
			range.collapse(true);
			range.moveStart('character', start);
			range.moveEnd('character', end - start);
			range.select();
			return this;
		}
		this.setSelectionRange(start, end);
		return this;
	},
	insertAtCursor: function(value, select) {
		var start = this.getSelectionStart();
		var end = this.getSelectionEnd();
		this.set('value', this.get('value').substring(0, start) + value + this.get('value').substring(end, this.get('value').length));
 		if($pick(select, true)) this.selectRange(start, start + value.length);
		else this.setCaretPosition(start + value.length);
		return this;
	},
	insertAroundCursor: function(options, select) {
		options = $extend({
			before: '',
			defaultMiddle: 'SOMETHING HERE',
			after: ''
		}, options);
		value = this.getSelectedText() || options.defaultMiddle;
		var start = this.getSelectionStart();
		var end = this.getSelectionEnd();
		if(start == end) {
			var text = this.get('value');
			this.set('value', text.substring(0, start) + options.before + value + options.after + text.substring(end, text.length));
			this.selectRange(start + options.before.length, end + options.before.length + value.length);
			text = null;
		} else {
			text = this.get('value').substring(start, end);
			this.set('value', this.get('value').substring(0, start) + options.before + text + options.after + this.get('value').substring(end, this.get('value').length));
			var selStart = start + options.before.length;
			if($pick(select, true)) this.selectRange(selStart, selStart + text.length);
			else this.setCaretPosition(selStart + text.length);
		}	
		return this;
	}
});


Element.Properties.inputValue = {
 
    get: function(){
			 switch(this.get('tag')) {
			 	case 'select':
					vals = this.getSelected().map(function(op){ 
						var v = $pick(op.get('value'),op.get('text')); 
						return (v=="")?op.get('text'):v;
					});
					return this.get('multiple')?vals:vals[0];
				case 'input':
					switch(this.get('type')) {
						case 'checkbox':
							return this.get('checked')?this.get('value'):false;
						case 'radio':
							var checked;
							if (this.get('checked')) return this.get('value');
							$(this.getParent('form')||document.body).getElements('input').each(function(input){
								if (input.get('name') == this.get('name') && input.get('checked')) checked = input.get('value');
							}, this);
							return checked||null;
					}
			 	case 'input': case 'textarea':
					return this.get('value');
				default:
					return this.get('inputValue');
			 }
    },
 
    set: function(value){
			switch(this.get('tag')){
				case 'select':
					this.getElements('option').each(function(op){
						var v = $pick(op.get('value'), op.get('text'));
						if (v=="") v = op.get('text');
						op.set('selected', $splat(value).contains(v));
					});
					break;
				case 'input':
					if (['radio','checkbox'].contains(this.get('type'))) {
						this.set('checked', $type(value)=="boolean"?value:$splat(value).contains(this.get('value')));
						break;
					}
				case 'textarea': case 'input':
					this.set('value', value);
					break;
				default:
					this.set('inputValue', value);
			}
			return this;
    },
		
	erase: function() {
		switch(this.get('tag')) {
			case 'select':
				this.getElements('option').each(function(op) {
					op.erase('selected');
				});
				break;
			case 'input':
				if (['radio','checkbox'].contains(this.get('type'))) {
					this.set('checked', false);
					break;
				}
			case 'input': case 'textarea':
				this.set('value', '');
				break;
			default:
				this.set('inputValue', '');
		}
		return this;
	}

};


/*
Script: Element.Measure.js
	Extends the Element native object to include methods useful in measuring dimensions.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

Element.implement({

	// Daniel Steigerwald - MIT licence
	measure: function(fn) {
		var restore = this.expose();
		fn.apply(this);
		return restore();
	},

	expose: function(){
		var style = this.style;
    var cssText = style.cssText;
    style.visibility = 'hidden';
    style.position = 'absolute';
    if (style.display == 'none') style.display = '';
		return (function(){ return this.set('style', cssText); }).bind(this);
	},
	
	getDimensions: function(options) {
		options = $merge({computeSize: false},options);
		var dim = {};
		function getSize(el, options){
			return (options.computeSize)?el.getComputedSize(options):el.getSize();
		};
		if(this.getStyle('display') == 'none'){
			var restore = this.expose();
			dim = getSize(this, options); //works now, because the display isn't none
			restore(); //put it back where it was
		} else {
			try { //safari sometimes crashes here, so catch it
				dim = getSize(this, options);
			}catch(e){}
		}
		return $chk(dim.x)?$extend(dim, {width: dim.x, height: dim.y}):$extend(dim, {x: dim.width, y: dim.height});
	},
	
	getComputedSize: function(options){
		options = $merge({
			styles: ['padding','border'],
			plains: {height: ['top','bottom'], width: ['left','right']},
			mode: 'both'
		}, options);
		var size = {width: 0,height: 0};
		switch (options.mode){
			case 'vertical':
				delete size.width;
				delete options.plains.width;
				break;
			case 'horizontal':
				delete size.height;
				delete options.plains.height;
				break;
		};
		var getStyles = [];
		//this function might be useful in other places; perhaps it should be outside this function?
		$each(options.plains, function(plain, key){
			plain.each(function(edge){
				options.styles.each(function(style){
					getStyles.push((style=="border")?style+'-'+edge+'-'+'width':style+'-'+edge);
				});
			});
		});
		var styles = this.getStyles.apply(this, getStyles);
		var subtracted = [];
		$each(options.plains, function(plain, key){ //keys: width, height, plains: ['left','right'], ['top','bottom']
			size['total'+key.capitalize()] = 0;
			size['computed'+key.capitalize()] = 0;
			plain.each(function(edge){ //top, left, right, bottom
				size['computed'+edge.capitalize()] = 0;
				getStyles.each(function(style,i){ //padding, border, etc.
					//'padding-left'.test('left') size['totalWidth'] = size['width']+[padding-left]
					if(style.test(edge)) {
						styles[style] = styles[style].toInt(); //styles['padding-left'] = 5;
						if(isNaN(styles[style]))styles[style]=0;
						size['total'+key.capitalize()] = size['total'+key.capitalize()]+styles[style];
						size['computed'+edge.capitalize()] = size['computed'+edge.capitalize()]+styles[style];
					}
					//if width != width (so, padding-left, for instance), then subtract that from the total
					if(style.test(edge) && key!=style && 
						(style.test('border') || style.test('padding')) && !subtracted.contains(style)) {
						subtracted.push(style);
						size['computed'+key.capitalize()] = size['computed'+key.capitalize()]-styles[style];
					}
				});
			});
		});
		if($chk(size.width)) {
			size.width = size.width+this.offsetWidth+size.computedWidth;
			size.totalWidth = size.width + size.totalWidth;
			delete size.computedWidth;
		}
		if($chk(size.height)) {
			size.height = size.height+this.offsetHeight+size.computedHeight;
			size.totalHeight = size.height + size.totalHeight;
			delete size.computedHeight;
		}
		return $extend(styles, size);
	}
});


/*
Script: Element.MouseOvers.js

Collection of mouseover behaviours (images, class toggles, etc.).

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

Element.implement({
	autoMouseOvers: function(options){
		
		options = $extend({
			outString: '_out',
			overString: '_over',
			cssOver: 'hover',
			cssOut: 'hoverOut',
			subSelector: '',
			applyToBoth: false
		}, options);
		el = this;
		if (options.subSelector) el = this.getElements(options.subSelector);
		if (el.retrieve('autoMouseOverSetup')) return this;
		el.store('autoMouseOverSetup', true);
		return el.addEvents({
			mouseenter: function(){
				this.swapClass(options.cssOut, options.cssOver);
				if (this.src && this.src.contains(options.outString))
					this.src = this.src.replace(options.outString, options.overString);
				if(options.applyToBoth && options.subSelector) {
					this.getElements(options.subSelector).each(function(el){
						el.swapClass(options.cssOut, options.cssOver);
					});
				}
			}.bind(this),
			mouseleave: function(){
				this.swapClass(options.cssOver, options.cssOut);
				if (this.src && this.src.contains(options.overString))
					this.src = this.src.replace(options.overString, options.outString);
				if(options.applyToBoth && options.subSelector) {
					this.getElements(options.subSelector).each(function(el){
						el.swapClass(options.cssOver, options.cssOut);
					});
				}
			}.bind(this)
		}).swapClass(options.cssOver, options.cssOut);
		el = null;
	}
});
window.addEvent('domready', function(){
	$$('img.autoMouseOver').each(function(img){
		img.autoMouseOvers();
	});
});


/*
Script: Element.Pin.js
	Extends the Element native object to include the pin method useful for fixed positioning for elements.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

window.addEvent('domready', function(){
	var test = new Element('div').setStyles({
		position: 'fixed',
		top: 0,
		right: 0
	}).inject(document.body);
	var supported = (test.offsetTop === 0);
	test.dispose();
	Browser.supportsPositionFixed = supported;
});

Element.implement({
	pin: function(enable){
		if(!Browser.loaded) dbug.log('cannot pin ' + this + ' natively because the dom is not ready');
		if (this.getStyle('display') == 'none') {
			dbug.log('cannot pin ' + this + ' because it is hidden');
			return;
		}
		if(enable!==false) {
			var p = this.getPosition();
			if(!this.retrieve('pinned')) {
				var pos = {
					top: (p.y - window.getScroll().y),
					left: (p.x - window.getScroll().x)
				};
				if(Browser.supportsPositionFixed) {
					this.setStyle('position','fixed').setStyles(pos);
				} else {
					this.store('pinnedByJS', true);
					this.setStyles({
						position: 'absolute',
						top: p.y,
						left: p.x
					});
					this.store('scrollFixer', function(){
						if(this.retrieve('pinned')) {
							var to = {
								top: (pos.top.toInt() + window.getScroll().y),
								left: (pos.left.toInt() + window.getScroll().x)
							};
							this.setStyles(to);
						}
					}.bind(this));
					window.addEvent('scroll', this.retrieve('scrollFixer'));
				}
				this.store('pinned', true);
			}
		} else {
			var op;
			if (!Browser.Engine.trident) {
				if (this.getParent().getComputedStyle('position') != 'static') op = this.getParent();
				else op = this.getParent().getOffsetParent();
			}
			var p = this.getPosition(op);
			this.store('pinned', false);
			var reposition;
			if (Browser.supportsPositionFixed && !this.retrieve('pinnedByJS')) {
				reposition = {
					top: (p.y + window.getScroll().y),
					left: (p.x + window.getScroll().x)
				};
			} else {
				this.store('pinnedByJS', false);
				window.removeEvent('scroll', this.retrieve('scrollFixer'));
				reposition = {
					top: (p.y),
					left: (p.x)
				};
			}
			this.setStyles($merge(reposition, {position: 'absolute'}));
		}
		return this.addClass('isPinned');
	},
	unpin: function(){
		return this.pin(false).removeClass('isPinned');
	},
	togglepin: function(){
		this.pin(!this.retrieve('pinned'));
	}
});


/*
Script: Element.Position.js
	Extends the Element native object to include methods useful positioning elements relative to others.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

Element.implement({

	setPosition: function(options){
		$each(options||{}, function(v, k){ if (!$defined(v)) delete options[k]; });
		options = $merge({
			relativeTo: document.body,
			position: {
				x: 'center', //left, center, right
				y: 'center' //top, center, bottom
			},
			edge: false,
			offset: {x:0,y:0},
			returnPos: false,
			relFixedPosition: false,
			ignoreMargins: false,
			allowNegative: false
		}, options);
		//compute the offset of the parent positioned element if this element is in one
		var parentOffset = {x: 0, y: 0};
		var parentPositioned = false;
		var putItBack = this.expose();
    /* dollar around getOffsetParent should not be necessary, but as it does not return 
     * a mootools extended element in IE, an error occurs on the call to expose. See:
		 * http://mootools.lighthouseapp.com/projects/2706/tickets/333-element-getoffsetparent-inconsistency-between-ie-and-other-browsers */
		var offsetParent = $(this.getOffsetParent());
		putItBack();
		if(offsetParent && offsetParent != this.getDocument().body) {
			var putItBack = offsetParent.expose();
			parentOffset = offsetParent.getPosition();
			putItBack();
			parentPositioned = true;
			options.offset.x = options.offset.x - parentOffset.x;
			options.offset.y = options.offset.y - parentOffset.y;
		}
		//upperRight, bottomRight, centerRight, upperLeft, bottomLeft, centerLeft
		//topRight, topLeft, centerTop, centerBottom, center
		function fixValue(option) {
			if($type(option) != "string") return option;
			option = option.toLowerCase();
			var val = {};
			if(option.test('left')) val.x = 'left';
			else if(option.test('right')) val.x = 'right';
			else val.x = 'center';

			if(option.test('upper')||option.test('top')) val.y = 'top';
			else if (option.test('bottom')) val.y = 'bottom';
			else val.y = 'center';
			return val;
		};
		options.edge = fixValue(options.edge);
		options.position = fixValue(options.position);
		if(!options.edge) {
			if(options.position.x == 'center' && options.position.y == 'center') options.edge = {x:'center',y:'center'};
			else options.edge = {x:'left',y:'top'};
		}
		
		this.setStyle('position', 'absolute');
		var rel = $(options.relativeTo) || document.body;
		var top = (rel == document.body)?window.getScroll().y:rel.getPosition().y;
		var left = (rel == document.body)?window.getScroll().x:rel.getPosition().x;
		var dim = this.getDimensions({computeSize: true, styles:['padding', 'border','margin']});
		if (options.ignoreMargins) {
			options.offset.x = options.offset.x - dim['margin-left'];
			options.offset.y = options.offset.y - dim['margin-top'];
		}
		var pos = {};
		var prefY = options.offset.y.toInt();
		var prefX = options.offset.x.toInt();
		switch(options.position.x) {
			case 'left':
				pos.x = left + prefX;
				break;
			case 'right':
				pos.x = left + prefX + rel.offsetWidth;
				break;
			default: //center
				pos.x = left + (((rel == document.body)?window.getSize().x:rel.offsetWidth)/2) + prefX;
				break;
		};
		switch(options.position.y) {
			case 'top':
				pos.y = top + prefY;
				break;
			case 'bottom':
				pos.y = top + prefY + rel.offsetHeight;
				break;
			default: //center
				pos.y = top + (((rel == document.body)?window.getSize().y:rel.offsetHeight)/2) + prefY;
				break;
		};
		
		if(options.edge){
			var edgeOffset = {};
			
			switch(options.edge.x) {
				case 'left':
					edgeOffset.x = 0;
					break;
				case 'right':
					edgeOffset.x = -dim.x-dim.computedRight-dim.computedLeft;
					break;
				default: //center
					edgeOffset.x = -(dim.x/2);
					break;
			};
			switch(options.edge.y) {
				case 'top':
					edgeOffset.y = 0;
					break;
				case 'bottom':
					edgeOffset.y = -dim.y-dim.computedTop-dim.computedBottom;
					break;
				default: //center
					edgeOffset.y = -(dim.y/2);
					break;
			};
			pos.x = pos.x+edgeOffset.x;
			pos.y = pos.y+edgeOffset.y;
		}
		pos = {
			left: ((pos.x >= 0 || parentPositioned || options.allowNegative)?pos.x:0).toInt(),
			top: ((pos.y >= 0 || parentPositioned || options.allowNegative)?pos.y:0).toInt()
		};
		if(rel.getStyle('position') == "fixed"||options.relFixedPosition) {
			pos.top = pos.top.toInt() + window.getScroll().y;
			pos.left = pos.left.toInt() + window.getScroll().x;
		}

		if(options.returnPos) return pos;
		else this.setStyles(pos);
		return this;
	}
});


/*
Script: Element.Shortcuts.js
	Extends the Element native object to include some shortcut methods.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

Element.implement({
	isVisible: function() {
		return this.getStyle('display') != 'none';
	},
	toggle: function() {
		return this[this.isVisible() ? 'hide' : 'show']();
	},
	hide: function() {
		var d;
		try {
			//IE fails here if the element is not in the dom
			if ('none' != this.getStyle('display')) d = this.getStyle('display');
		} catch(e){}
		this.store('originalDisplay', d||'block'); 
		this.setStyle('display','none');
		return this;
	},
	show: function(display) {
		original = this.retrieve('originalDisplay')?this.retrieve('originalDisplay'):this.get('originalDisplay');
		this.setStyle('display',(display || original || 'block'));
		return this;
	},
	swapClass: function(remove, add) {
		return this.removeClass(remove).addClass(add);
	},
	//TODO
	//DO NOT USE THIS METHOD
	//it is temporary, as Mootools 1.1 will negate its requirement
	fxOpacityOk: function(){
		return !Browser.Engine.trident4;
	} 
});


$E = document.getElement.bind(document);

//returns a collection given an id or a selector
$G = function(elements) {
	return $splat($(elements)||$$(elements));
};

/*
Script: Fx.Marquee.js
	Defines Fx.Marquee, a marquee class for animated notifications.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
Fx.Marquee = new Class({
	Extends: Fx.Morph,
	options: {
		mode: 'horizontal', //or vertical
		message: '', //the message to display
		revert: true, //revert back to the previous message after a specified time
		delay: 5000, //how long to wait before reverting
		cssClass: 'msg', //the css class to apply to that message
		showEffect: { opacity: 1 },
		hideEffect: {opacity: 0},
		revertEffect: { opacity: [0,1] },
		currentMessage: null
/*	onRevert: $empty,
		onMessage: $empty */
	},
	initialize: function(container, options){
		container = $(container); 
		var msg = this.options.currentMessage || (container.getChildren().length == 1)?container.getFirst():''; 
		var wrapper = new Element('div', {	
				styles: { position: 'relative' },
				'class':'fxMarqueeWrapper'
			}).inject(container); 
		this.parent(wrapper, options);
		this.current = this.wrapMessage(msg);
	},
	wrapMessage: function(msg){
		if($(msg) && $(msg).hasClass('fxMarquee')) { //already set up
			var wrapper = $(msg);
		} else {
			//create the wrapper
			var wrapper = new Element('span', {
				'class':'fxMarquee',
				styles: {
					position: 'relative'
				}
			});
			if($(msg)) wrapper.grab($(msg)); //if the message is a dom element, inject it inside the wrapper
			else if ($type(msg) == "string") wrapper.set('html', msg); //else set it's value as the inner html
		}
		return wrapper.inject(this.element); //insert it into the container
	},
	announce: function(options) {
		this.setOptions(options).showMessage();
		return this;
	},
	showMessage: function(reverting){
		//delay the fuction if we're reverting
		(function(){
			//store a copy of the current chained functions
			var chain = this.$chain?$A(this.$chain):[];
			//clear teh chain
			this.clearChain();
			this.element = $(this.element);
			this.current = $(this.current);
			this.message = $(this.message);
			//execute the hide effect
			this.start(this.options.hideEffect).chain(function(){
				//if we're reverting, hide the message and show the original
				if(reverting) {
					this.message.hide();
					if(this.current) this.current.show();
				} else {
					//else we're showing; remove the current message
					if(this.message) this.message.dispose();
					//create a new one with the message supplied
					this.message = this.wrapMessage(this.options.message);
					//hide the current message
					if(this.current) this.current.hide();
				}
				//if we're reverting, execute the revert effect, else the show effect
				this.start((reverting)?this.options.revertEffect:this.options.showEffect).chain(function(){
					//merge the chains we set aside back into this.$chain
					if (this.$chain) this.$chain.combine(chain);
					else this.$chain = chain;
					this.fireEvent((reverting)?'onRevert':'onMessage');
					//then, if we're reverting, show the original message
					if(!reverting && this.options.revert) this.showMessage(true);
					//if we're done, call the chain stack
					else this.callChain.delay(this.options.delay, this);
				}.bind(this));
			}.bind(this));
		}).delay((reverting)?this.options.delay:10, this);
		return this;
	}
});

/*
Script: Fx.Move.js
	Defines Fx.Move, a class that works with Element.Position.js to transition an element from one location to another.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
Fx.Move = new Class({
	Extends: Fx.Morph,
	options: {
		relativeTo: document.body,
		position: 'center',
		edge: false,
		offset: {x:0,y:0}
	},
	start: function(destination){
		return this.parent(this.element.setPosition($merge(this.options, destination, {returnPos: true})));
	}
});

Element.Properties.move = {

	set: function(options){
		var morph = this.retrieve('move');
		if (morph) morph.cancel();
		return this.eliminate('move').store('move:options', $extend({link: 'cancel'}, options));
	},

	get: function(options){
		if (options || !this.retrieve('move')){
			if (options || !this.retrieve('move:options')) this.set('move', options);
			this.store('move', new Fx.Move(this, this.retrieve('move:options')));
		}
		return this.retrieve('move');
	}

};

Element.implement({

	move: function(options){
		this.get('move').start(options);
		return this;
	}

});


/*
Script: Fx.Reveal.js
	Defines Fx.Reveal, a class that shows and hides elements with a transition.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
Fx.Reveal = new Class({
	Extends: Fx.Morph,
	options: {
		styles: ['padding','border','margin'],
		transitionOpacity: true,
		mode:'vertical',
		heightOverride: null,
		widthOverride: null
/*	onShow: $empty,
		onHide: $empty */
	},
	dissolve: function(){
		try {
			if(!this.hiding && !this.showing) {
				if(this.element.getStyle('display') != 'none'){
					this.hiding = true;
					this.showing = false;
					this.hidden = true;
					var startStyles = this.element.getComputedSize({
						styles: this.options.styles,
						mode: this.options.mode
					});
					var setToAuto = this.element.style.height === ""||this.element.style.height=="auto";
					this.element.setStyle('display', 'block');
					if (this.element.fxOpacityOk() && this.options.transitionOpacity) startStyles.opacity = 1;
					var zero = {};
					$each(startStyles, function(style, name){
						zero[name] = [style, 0]; 
					}, this);
					var overflowBefore = this.element.getStyle('overflow');
					this.element.setStyle('overflow', 'hidden');
					//put the final fx method at the front of the chain
					this.$chain = this.$chain || [];
					this.$chain.unshift(function(){
						if(this.hidden) {
							this.hiding = false;
							$each(startStyles, function(style, name) {
								startStyles[name] = style;
							}, this);
							this.element.setStyles($merge({display: 'none', overflow: overflowBefore}, startStyles));
							if (setToAuto) this.element.setStyle('height', 'auto');
						}
						this.fireEvent('onHide', this.element);
						this.callChain();
					}.bind(this));
					this.start(zero);
				} else {
					this.callChain.delay(10, this);
					this.fireEvent('onComplete', this.element);
					this.fireEvent('onHide', this.element);
				}
			} else if (this.options.link == "chain") {
				this.chain(this.dissolve.bind(this));
			} else if (this.options.link == "cancel" && !this.hiding) {
				this.cancel();
				this.dissolve();
			}
		} catch(e) {
			this.hiding = false;
			this.element.hide();
			this.callChain.delay(10, this);
			this.fireEvent('onComplete', this.element);
			this.fireEvent('onHide', this.element);
		}
		return this;
	},
	reveal: function(){
		try {
			if(!this.showing && !this.hiding) {
				if(this.element.getStyle('display') == "none" || 
					 this.element.getStyle('visiblity') == "hidden" || 
					 this.element.getStyle('opacity')==0){
					this.showing = true;
					this.hiding = false;
					this.hidden = false;
					//toggle display, but hide it
					var before = this.element.getStyles('visibility', 'display', 'position');
					this.element.setStyles({
						visibility: 'hidden',
						display: 'block',
						position:'absolute'
					});
					var setToAuto = this.element.style.height === ""||this.element.style.height=="auto";
					//enable opacity effects
					if(this.element.fxOpacityOk() && this.options.transitionOpacity) this.element.setStyle('opacity',0);
					//create the styles for the opened/visible state
					var startStyles = this.element.getComputedSize({
						styles: this.options.styles,
						mode: this.options.mode
					});
					//reset the styles back to hidden now
					this.element.setStyles(before);
					$each(startStyles, function(style, name) {
						startStyles[name] = style;
					}, this);
					//if we're overridding height/width
					if($chk(this.options.heightOverride)) startStyles['height'] = this.options.heightOverride.toInt();
					if($chk(this.options.widthOverride)) startStyles['width'] = this.options.widthOverride.toInt();
					if(this.element.fxOpacityOk() && this.options.transitionOpacity) startStyles.opacity = 1;
					//create the zero state for the beginning of the transition
					var zero = { 
						height: 0,
						display: 'block'
					};
					$each(startStyles, function(style, name){ zero[name] = 0 }, this);
					var overflowBefore = this.element.getStyle('overflow');
					//set to zero
					this.element.setStyles($merge(zero, {overflow: 'hidden'}));
					//start the effect
					this.start(startStyles);
					if (!this.$chain) this.$chain = [];
					this.$chain.unshift(function(){
						if (!this.options.heightOverride && setToAuto) {
							if (["vertical", "both"].contains(this.options.mode)) this.element.setStyle('height', 'auto');
							if (["width", "both"].contains(this.options.mode)) this.element.setStyle('width', 'auto');
						}
						if(!this.hidden) this.showing = false;
						this.element.setStyle('overflow', overflowBefore);
						this.callChain();
						this.fireEvent('onShow', this.element);
					}.bind(this));
				} else {
					this.callChain();
					this.fireEvent('onComplete', this.element);
					this.fireEvent('onShow', this.element);
				}
			} else if (this.options.link == "chain") {
				this.chain(this.reveal.bind(this));
			} else if (this.options.link == "cancel" && !this.showing) {
				this.cancel();
				this.reveal();
			}
		} catch(e) {
			this.element.setStyles({
				display: 'block',
				visiblity: 'visible',
				opacity: 1
			});
			this.showing = false;
			this.callChain.delay(10, this);
			this.fireEvent('onComplete', this.element);
			this.fireEvent('onShow', this.element);
		}
		return this;
	},
	toggle: function(){
		try {
			if(this.element.getStyle('display') == "none" || 
				 this.element.getStyle('visiblity') == "hidden" || 
				 this.element.getStyle('opacity')==0){
				this.reveal();
		 	} else {
				this.dissolve();
			}
		} catch(e) { this.show(); }
	 return this;
	}
});

Element.Properties.reveal = {

	set: function(options){
		var reveal = this.retrieve('reveal');
		if (reveal) reveal.cancel();
		return this.eliminate('reveal').store('reveal:options', $extend({link: 'cancel'}, options));
	},

	get: function(options){
		if (options || !this.retrieve('reveal')){
			if (options || !this.retrieve('reveal:options')) this.set('reveal', options);
			this.store('reveal', new Fx.Reveal(this, this.retrieve('reveal:options')));
		}
		return this.retrieve('reveal');
	}

};

Element.Properties.dissolve = Element.Properties.reveal;

Element.implement({

	reveal: function(options){
		this.get('reveal', options).reveal();
		return this;
	},
	
	dissolve: function(options){
		this.get('reveal', options).dissolve();
		return this;
	},

	nix: function() {
		var  params = Array.link(arguments, {destroy: Boolean.type, options: Object.type});
		this.get('reveal', params.options).dissolve().chain(function(){
			this[params.destroy?'destroy':'dispose']();
		}.bind(this));
		return this;
	}
});


/*
Script: Fx.Sort.js
	Defines Fx.Sort, a class that reorders lists with a transition.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
Fx.Sort = new Class({
	Extends: Fx.Elements,
	options: {
			mode: 'vertical' //or 'horizontal'
	},
	initialize: function(elements, options){
			this.parent(elements, options);
			//set the position of each element to relative
			this.elements.each(function(el){
					if(el.getStyle('position') == 'static') el.setStyle('position', 'relative');
			});
			this.setDefaultOrder();
	},
	setDefaultOrder: function(){
			this.currentOrder = this.elements.map(function(el, index){
				return index;
			});
	},
	sort: function(newOrder){
		if($type(newOrder) != 'array') return false;
		var top = 0;
		var left = 0;
		var zero = {};
		var vert = this.options.mode == "vertical";
		//calculate the current location of all the elements
		var current = this.elements.map(function(el, index){
			var size = el.getComputedSize({styles:['border','padding','margin']});
			var val;
			if(vert) {
				val =	{
					top: top,
					margin: size['margin-top'],
					height: size.totalHeight
				};
				top += val.height - size['margin-top'];
			} else {
				val = {
					left: left,
					margin: size['margin-left'],
					width: size.totalWidth
				};
				left += val.width;
			}
			var plain = vert?'top':'left';
			zero[index]={};
			var start = el.getStyle(plain).toInt();
			zero[index][plain] = ($chk(start))?start:0;
			return val;
		}, this);
		this.set(zero);
		//if the array passed in is not the same size as
		//the amount of elements we have, fill it in
		//or cut it short
		newOrder = newOrder.map(function(i){ return i.toInt() });
		if (newOrder.length != this.elements.length){
			this.currentOrder.each(function(index) {
				if(!newOrder.contains(index)) newOrder.push(index);
			});
			if(newOrder.length > this.elements.length) {
				newOrder.splice(this.elements.length-1, newOrder.length-this.elements.length);
			}
		}
		var top = 0;
		var left = 0;
		var margin = 0;
		var next = {};
		//calculate the new location of each item
		newOrder.each(function(item, index){
			var newPos = {};
			if(vert) {
					newPos.top = top - current[item].top - margin;
					top += current[item].height;
			} else {
					newPos.left = left - current[item].left;	
					left += current[item].width;
			}
			margin = margin + current[item].margin;
			next[item]=newPos;
		}, this);
		var mapped = {};
		$A(newOrder).sort().each(function(index){
			mapped[index] = next[index];
		});
		//store the current order
		//execute the effect
		this.start(mapped);
		this.currentOrder = newOrder;
		return this;
	},
	rearrangeDOM: function(newOrder){
		newOrder = newOrder || this.currentOrder;
		var parent = this.elements[0].getParent();
		var rearranged = [];
		this.elements.setStyle('opacity', 0);
		//move each element and store the new default order
		newOrder.each(function(index) {
			rearranged.push(this.elements[index].inject(parent).setStyles({
				top: 0,
				left: 0
			}));
		}, this);
		this.elements.setStyle('opacity', 1);
		this.elements = $$(rearranged);
		this.setDefaultOrder();
		return this;
	},
	getDefaultOrder: function(){
		return this.elements.map(function(el, index) {
			return index;
		})
	},
	forward: function(){
		return this.sort(this.getDefaultOrder());
	},
	backward: function(){
		return this.sort(this.getDefaultOrder().reverse());
	},
	reverse: function(){
		return this.sort(this.currentOrder.reverse());
	},
	sortByElements: function(elements){
		return this.sort(elements.map(function(el){
			return this.elements.indexOf(el);
		}));
	},
	swap: function(one, two) {
		if($type(one) == 'element') {
			one = this.elements.indexOf(one);
			two = this.elements.indexOf(two);
		}
		var indexOne = this.currentOrder.indexOf(one);
		var indexTwo = this.currentOrder.indexOf(two);
		var newOrder = $A(this.currentOrder);
		newOrder[indexOne] = two;
		newOrder[indexTwo] = one;
		this.sort(newOrder);
	}
});


/*
Script: JsonP.js
	Defines JsonP, a class for cross domain javascript via script injection.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var JsonP = new Class({
	Implements: [Options, Events],
	options: {
//	onComplete: $empty,
//	globalFunction: '',
//	abortAfter: 0,
		callBackKey: "callback",
		queryString: "",
		data: {},
		timeout: 5000,
		retries: 0
	},
	initialize: function(url, options){
		this.setOptions(options);
		this.url = this.makeUrl(url).url;
		this.fired = false;
		this.scripts = [];
		this.requests = 0;
		this.triesRemaining = [];
	},
	request: function(url, requestIndex){
		var u = this.makeUrl(url);
		if(!$chk(requestIndex)) {
			requestIndex = this.requests;
			this.requests++;
		}
		if(!$chk(this.triesRemaining[requestIndex])) this.triesRemaining[requestIndex] = this.options.retries;
		var remaining = this.triesRemaining[requestIndex]; //saving bytes
		dbug.log('retrieving by json script method: %s', u.url);
		var dl = (Browser.Engine.trident)?50:0; //for some reason, IE needs a moment here...
		(function(){
			var script = new Element('script', {
				src: u.url, 
				type: 'text/javascript',
				id: 'jsonp_'+u.index+'_'+requestIndex
			});
			this.fired = true;
			this.addEvent('onComplete', function(){
				try {script.dispose();}catch(e){}
			}.bind(this));
			script.inject(document.head);
			
			if ($chk(this.options.abortAfter) && ! remaining) script.dispose.delay(this.options.abortAfter, script);

			if(remaining) {
				(function(){
					this.triesRemaining[requestIndex] = remaining - 1;
					if(script.getParent() && remaining) {
						dbug.log('removing script (%o) and retrying: try: %s, remaining: %s', requestIndex, remaining);
						script.dispose();
						this.request(url, requestIndex);
					}
				}).delay(this.options.timeout, this);
			}
		}.bind(this)).delay(dl);
		return this;
	},
	makeUrl: function(url){
		var index;
		if (JsonP.requestors.contains(this)) {
			index = JsonP.requestors.indexOf(this);
		} else {
			index = JsonP.requestors.push(this) - 1;
			JsonP.requestors['request_'+index] = this;
		}
		if(url) {
			var separator = (url.test('\\?'))?'&':'?';
			var jurl = url + separator + this.options.callBackKey + "=JsonP.requestors.request_" +
				index+".handleResults";
			if(this.options.queryString) jurl += "&"+this.options.queryString;
			jurl += "&"+Hash.toQueryString(this.options.data);
		} else {
			var jurl = this.url;
		}
		if ($chk(this.options.globalFunction)) {
			window[this.options.globalFunction] = function(r){
				JsonP.requestors[index].handleResults(r)
			};
		}
		return {url: jurl, index: index};
	},
	handleResults: function(data){
		dbug.log('jsonp received: ', data);
		this.fireEvent('onComplete', [data, this]);
	}
});
JsonP.requestors = [];


/*
Script: Request.NoCache.js
	Extends Request and Request.HTML to automatically include a unique noCache value to prevent request caching.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
(function(){
	var rqst = function(cls) {
		return Class.refactor(cls, {
/*		options: {
				noCache: false
			}, */
			send: function(options){
				if (this.options.noCache) {
					var type = $type(options);
					if (type == 'string' || type == 'element') options = {data: options};

					var old = this.options;
					options = $extend({data: old.data, url: old.url, method: old.method}, options);
					var data = options.data, url = options.url, method = options.method;

					if (options.url) {
						options.url += (options.url.contains("?")?"&":"?")+"noCache=" + new Date().getTime();
					} else  {
						switch ($type(data)){
							case 'element': data = $(data).toQueryString(); break;
							case 'object': case 'hash': data = Hash.toQueryString(data);
						}
						data += (data.length?"&":"") + "noCache=" + new Date().getTime();
						options.data = data;
					}
				}
				this.parent(options);
			}
		});
	};
	Request = rqst(Request);
	Request.HTML = rqst(Request.HTML);
})();

/*
Script: Request.Queue.js
	Controls several instances of Request and its variants to run only one request at a time.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
 */
Request.Queue = new Class({
	Implements: [Options, Events],
	Binds: ['attach', 'onRequest', 'onComplete', 'onCancel', 'onSuccess', 'onFailure', 'onException'],
	reqBinders: {},
	queue: [],
	options: {
/*	onRequestStart: $empty,
		onRequestEnd: $empty,
		onRequestSuccess: $empty,
		onRequestComplete: $empty,
		onRequestCancel: $empty,
		onRequestException: $empty,
		onRequestFailure: $empty, */
		stopOnFailure: true,
		autoAdvance: true,
		concurrent: 1,
		requests: {}
	},
	initialize: function(options){
		this.setOptions(options);
		this.requests = new Hash();
		this.addRequests(this.options.requests);
	},
	addRequest: function(name, request){
		this.requests.set(name, request);
		this.attach(name, request);
		return this;
	},
	addRequests: function(reqs){
		$each(reqs, function(v, k){
			this.addRequest(k, v);
		}, this);
		return this;
	},
	getName: function(req) {
		return this.requests.keyOf(req);
	},
	attach: function(name, req){
		if (req._groupSend) return this;
		['onRequest', 'onComplete', 'onCancel', 'onSuccess', 'onFailure', 'onException'].each(function(evt){
			this.reqBinders[name] = this.reqBinders[name] || {};
			this.reqBinders[name][evt] = function(){
				this[evt].apply(this, [name, req].extend(arguments));
			}.bind(this);
			req.addEvent(evt, this.reqBinders[name][evt]);
		}, this);
		req._groupSend = req.send;
		req.send = function(options){
			this.send(name, options);
			return req;
		}.bind(this);
		return this;
	},
	removeRequest: function(req) {
		var name = $type(req) == 'object' ? this.getName(req) : req;
		if (!name && $type(name) != 'string') return false;
		req = this.requests.get(name);
		if (!req) return false;
		['onRequest', 'onComplete', 'onCancel', 'onSuccess', 'onFailure', 'onException'].each(function(evt) {
			req.removeEvent(evt, this.reqBinders[name][evt]);
		}, this);
		req.send = req._groupSend;
		delete req._groupSend;
		return this;
	},
	getRunning: function(){
		var running = [];
		this.requests.each(function(req) {
			if (req.running) running.include(req);
		});
		return running;
	},
	isRunning: function(){ 
		return !!this.getRunning().length 
	},
	send: function(name, options) {
		var q;
		q = function(){
			this.requests.get(name)._groupSend(options);
			this.queue.erase(q);
		}.bind(this);
		q.name = name;
		if (this.getRunning().length >= this.options.concurrent || (this.error && this.options.stopOnFailure)) this.queue.push(q);
		else q();
		return this;
	},
	hasNext: function(name){
		if (!name) return !!this.queue.length;
		return !!this.queue.filter(function(q) { return q.name == name; }).length;
	},
	resume: function(){
		this.error = false;
		(this.options.concurrent - this.getRunning().length).times(this.runNext.bind(this));
		return this;
	},
	runNext: function(name){
		if (this.queue.length) {
			if (!name) {
				this.queue[0]();
			} else {
				var found;
				this.queue.each(function(q){
					if (!found && q.name == name) {
						found = true;
						q();
					}
				});
			}
		}
		return this;
	},
	clear: function(name){
		if (!name) {
			this.queue.empty();
		} else {
			this.queue = this.queue.map(function(q){
				if (q.name != name) return q;
				else return false;
			}).filter(function(q){ return q; });
		}
	},
	cancel: function(name) {
		this.requests.get(name).cancel();
		return this;
	},
	onRequest: function(){
		this.fireEvent('onRequest', arguments);
	},
	onComplete: function(){
		this.fireEvent('onComplete', arguments);		
	},
	onCancel: function(){
		if (this.options.autoAdvance && !this.error) this.runNext();
		this.fireEvent('onCancel', arguments);
	},
	onSuccess: function(){
		if (this.options.autoAdvance && !this.error) this.runNext();
		this.fireEvent('onSuccess', arguments);
	},
	onFailure: function(){
		this.error = true;
		if (!this.options.stopOnFailure && this.options.autoAdvance) this.runNext();
		this.fireEvent('onFailure', arguments);
	},
	onException: function(){
		this.error = true;
		if (!this.options.stopOnFailure && this.options.autoAdvance) this.runNext();
		this.fireEvent('onException', arguments);
	}
	
});

/*
Script: IconMenu.js
	Defines IconMenu, a simple icon (img) based menu.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var IconMenu = new Class({
	Implements: [Options, Events],
	options: {
		container: document,
		images: ".iconImgs",
		captions: ".iconCaptions",
		removeLinks: false,
		clearLinks: false,
		useAxis: 'x',
		onFocusDelay: 0,
		initialFocusDelay: 250,
		onBlurDelay: 0,
		length: 'auto',
		iconPadding: 1,
		scrollFxOptions: {
			duration: 1800,
			transition: 'cubic:in:out'
		},
		backScrollButtons: '#scrollLeft',
		forwardScrollButtons: '#scrollRight',
		onSelect: function(index, img){
			img.morph({
					'border-top-color': '#00A0C6',
					'border-left-color': '#00A0C6',
					'border-right-color': '#00A0C6',
					'border-bottom-color': '#00A0C6'
			});
		},
		onDeSelect: function(index, img){
			img.morph({
					'border-top-color': '#555',
					'border-left-color': '#555',
					'border-right-color': '#555',
					'border-bottom-color': '#555'
			});
//		onFocus: $empty, //mouseover of target area
//		onBlur: $empty, //mouseout of target area
//		onEmpty: $empty,
//		onRemoveItem: $empty,
//		onRemoveItems: $empty,
//		onScroll: $empty,
//		onPageForward: $empty,
//		onPageBack: $empty,
		}
	},
	imgs: [],
	selected: [],
	initialize: function(options) {
		//set the options
		this.setOptions(options);
		//save a reference to the container
		this.container = $(this.options.container);
		//get the captions from the options
		var captions = ($type(this.options.captions) == "string")
			?this.container.getElements(this.options.captions)
			:this.options.captions;
		//get the images from the options
		var imgs = ($type(this.options.images) == "string")
			?this.container.getElements(this.options.images)
			:this.options.images;
		//loop through each one
		imgs.each(function(img, index) {
			//add it to the menu
			this.addItem(img, captions[index], null);
		}, this);
		
		this.fireEvent('onItemsAdded', this.imgs, 50);
		this.side = (this.options.useAxis == 'x')?'left':'top';
		this.container.setStyle(this.side, this.container.getStyle(this.side).toInt()||0);
		this.onFocusDelay = this.options.initialFocusDelay;
		//set up the events
		this.setupEvents();
	},
	toElement: function(){
		return this.container;
	},
	bound: {
			mouseover: {},
			mouseout: {}
	},
	scrollTo: function(index, useFx){
		//set useFx default to true
		useFx = $pick(useFx, true);
		//get the current range in view
		var currentRange = this.calculateRange();
		//if we're there, exit
		if(index == currentRange.start) return this;
		//get the range for the new position
		var newRange = this.calculateRange(index);
		//if this returns no items, exit
		if(!newRange.elements.length) return this; //no next page! >> Ajax here
		//make sure the container has a position set
		if(this.container.getStyle('position') == 'static') this.container.setStyle('position', 'relative');
		//create the scroll effects if not present already
		if(!this.scrollerFx) 
			this.scrollerFx = new Fx.Tween(this.container, $merge(this.options.scrollFxOptions, {property: this.side, wait: false}));
		//scroll to the new location
		if(useFx) {
			this.scrollerFx.start(-newRange.elements[0].offset).chain(function(){
			//set the index to be this new location
				this.fireEvent('onScroll', [index, newRange]);
			}.bind(this));
		} else {
			//we're not using effects, so just jump to the location
			this.scrollerFx.set(-newRange.elements[0].offset);
			this.fireEvent('onScroll', [index, newRange]);
		}
		this.currentOffset = index;
		return this;
	},
	pageForward: function(howMany){
		var range = this.calculateRange();
		return this.scrollTo(($type(howMany) == "number")?range.start+howMany:range.end);
	},
	pageBack: function(howMany) {
		return this.scrollTo(($type(howMany) == "number")?this.currentOffset-howMany:this.calculateRange(this.currentOffset, true).start);
	},
	addItem: function(img, caption, where) {
		if (caption) {
			img.store('caption', caption);
			caption.store('image', img);
		}
		//figure out where to put it
		where = ($defined(where))?where:this.imgs.length;
		//if we've already got this image in there, remove it before putting it in the right place
		if(this.imgs.contains(img)) this.removeItems([img], true);
		//insert the image and caption into the array of these things
		this.imgs.splice(where, 0, $(img));

		//set up the events for the element
		this.setupIconEvents(img, caption);
		this.fireEvent('onAdd', [img, caption]);
		return this;
	},
	removeItems: function(imgs, useFx){
		var range = this.calculateRange();
		if(!imgs.length) return this;;
		//create a copy; this is because
		//IconMenu.empty passes *this.selected*
		//which we modify in the process of removing things
		//so we must work on a copy of that so we don't change
		//the list as we iterate over it
		imgs = $A(imgs);
		//set the fx default
		useFx = $pick(useFx, true);
		//placeholder for the items we're removing; the effect will
		//only be applied to the dom element that contains the image and the caption
		var fadeItems = [];
		var fadeItemImgs = [];
		//the effect we'll use
		var effect = {
				width: 0,
				'border-width':0
		};
		//an object to store all the copies of the effect; one for each item to be passed
		//to Fx.Elements
		var fadeEffects = {};
		//for items that aren't in the current view, we're not going to use a transition
		var itemsToQuietlyRemove = {
			before: [],
			beforeImgs: [],
			after: [],
			afterImgs: []
		};
				
		//a list of all the icons by index
		var indexes = [];
		//for each image in the set to be removed
		imgs.each(function(image){
			var index = this.imgs.indexOf(image);
			//if the image is visible
			if(index >= range.end) {
				itemsToQuietlyRemove.after.push(image.getParent());
				itemsToQuietlyRemove.afterImgs.push(image);
			} else if(index < range.start) {
				itemsToQuietlyRemove.before.push(image.getParent());
				itemsToQuietlyRemove.beforeImgs.push(image);
			} else {
				//store the parent of the image
				fadeItems.push(image.getParent());
				fadeItemImgs.push(image);
				//copy the effect value for this item
				fadeEffects[fadeItems.length-1] = $unlink(effect);
			}
			//remove the reference in the selected array
			//because when it's gone, it won't be selected anymore
			this.selected.erase(image);
			//store the index of where this image was in the menu
			indexes.push(index);
		}, this);
		//for the list of images in the menu that were selected, remove them
		//we didn't do this earlier so we could avoid changing
		//the array while we were working on it
		this.imgs = this.imgs.filter(function(img, index){
			return !indexes.contains(index);
		});
		var removed = [];
		//items page left, remove them, but then we have to update the scroll offset to account
		//for their departure
		if(itemsToQuietlyRemove.before.length) {
			var scrollTo = this.imgs.indexOf(range.elements[0].image);
			itemsToQuietlyRemove.before.each(function(el, index){
				this.fireEvent('onRemoveItem', [el]);
				var img = itemsToQuietlyRemove.beforeImgs[index];
				removed.push(img);
				try {
					el.dispose();
					//scroll to the current offset again quickly
				}catch(e){ dbug.log('before: error removing element %o, %o', el, e); }
			}, this);
			this.scrollTo(scrollTo, false);
		}
		//for items page right, just remove them quickly and quietly
		itemsToQuietlyRemove.after.each(function(el, index){
			this.fireEvent('onRemoveItem', [el]);
			removed.push(itemsToQuietlyRemove.afterImgs[index]);
			try {
				el.dispose(); 
			}catch(e){ dbug.log('after: error removing element %o, %o', el, e); }
		});

		//define a function that removes all the items from the dom
		function clean(range, additionalItems){
			var items = [];
			//then fade out the items that are currently visible
			fadeItems.each(function(el, index){
				this.fireEvent('onRemoveItem', [el]);
				items.push(fadeItemImgs[index]);
				try {
					el.dispose(); 
				}catch(e){ dbug.log('fade: error removing element %o, %o', el, e); }
			}, this);
			items.combine(additionalItems);
			this.fireEvent('onRemoveItems', [items]);
			range = this.calculateRange();
			if(range.elements == 0 && range.start > 0) this.pageBack();
			//if there aren't any items left, fire the onEmpty event
			if(!this.imgs.length) this.fireEvent('onEmpty');
		}
		//if we're using effects, do the transition then call clean()
		if(useFx) new Fx.Elements(fadeItems).start(fadeEffects).chain(clean.bind(this, [range, removed]));
		//else just clean
		else clean.apply(this, [range, removed]);
		return this;
	},
	removeSelected: function(useFx){
		this.removeItems(this.selected, useFx);
		return this;
	},
	empty: function(suppressEvent){
		//placeholder for the effects and items
		var effect = {};
		var items = [];
		//loop through all the images in the icon menu
		this.imgs.each(function(img, index){
			//add the icon container to the list of items to remove
			items.push(img.getParent());
			//create a reference for each one to pass to Fx.Elements
			effect[index] = {opacity: 0};
		});
		//create an instance of Fx.Elements and fade them all out
		new Fx.Elements(items).start(effect).chain(function(){
			//then remove them all instantly
			this.removeItems(this.imgs, false);
			//and fire the onEmpty event
			if(!suppressEvent) this.fireEvent('onEmpty');
		}.bind(this));
		return this;
	},
	selectItem: function(index, select){
		//get the image to select
		var img = this.imgs[index];
		//add or remove the "selected" class
		if($defined(select)) {
			if(select) img.addClass('selected');
			else img.removeClass('selected');
		} else {
			img.toggleClass('selected');
		}
		//if it has the class, then fade the border blue
		if(img.hasClass('selected')){
			//store this image in the index of selected images
			this.selected.push(img);
			this.fireEvent('select', [index, img]);
		} else {
			//else we're deselecting; remove the image from the index of selected images
			this.selected.erase(img);
			this.fireEvent('onDeSelect', [index, img]);
		}
		return this;
	},
	getDefaultWidth: function(){
		//if the user specified a width, just return it
		if($type(this.options.length) == "number") return this.options.length;
		//if, on the other hand, they specified another element than the container
		//to calculate the width, use it
		var container = $(this.options.length);
		//otherwise, use the container
		if(!container) container = this.container.getParent();
		//return the width or height of that element, depending on the axis chosen in the options
		return container.getSize()[this.options.useAxis];
	},
	getIconPositions: function(){
		var offsets = [];
		var cumulative = 0;
		var prev;
		//loop through all the items
		this.imgs.each(function(img, index){
			//we're measuring the element that contains the image
			var parent = img.getParent();
			//get the width or height of that parent using the appropriate axis
			cumulative += (prev)?img['offset'+this.side.capitalize()] - prev['offset'+this.side.capitalize()]:0;
			prev = img;
			//var size = parent.getSize().size[this.options.useAxis]
			//store the data
			offsets.push({
				image: img,
				size: parent.getSize()[this.options.useAxis],
				offset: cumulative,
				container: parent
			});
		}, this);
		return offsets;
	},
	calculateRange: function(index, fromEnd){
		if(!this.imgs.length) return {start: 0, end: 0, elements: []};
		index = $pick(index, this.currentOffset||0);
		if(index < 0) index = 0;
		//dbug.trace();
		//get the width of space that icons are visible
		var length = this.getDefaultWidth();
		//get the positions of all the icons
		var positions = this.getIconPositions();
		var referencePoint;
		//if we're paginating forward the reference point is the left edge 
		//of the range is the left edge of the first icon
		//but if we're going backwards, the referencePoint is the left edge of the first icon currently in range
		//the problem is if the user removes the entire last page of icons, then this
		//item no longer exists, so...
		if(positions[index]) {
			//if the item exists, use it
			referencePoint = positions[index].offset;
		} else {
			//else the right edge of the last icon is the reference point
			//the last icon is the container of the last image
			var lastIcon = this.imgs.getLast().getParent();
			var coords = lastIcon.getCoordinates();
			//and the reference point is that icon's width plus it's left offset minus the offset 
			//of the parent (which gets offset negatively and positively for scrolling
			referencePoint = coords.width + coords.left - lastIcon.getParent().getPosition().x;
		}
		//figure out which ones are in range
		var range = positions.filter(function(position, i){
			//if the index supplied is the endpoint
			//then it's in range if the index of the icon is less than the index,
			//and the left side is less than that of the one at the end point
			//and if the left side is greater than or equal to the end point's position minus the length
			if(fromEnd) return i < index && 
				position.offset < referencePoint &&
				position.offset >= referencePoint-length;
			
			//else we go forward...
			//if the item is after the index start and the posision is 
			//less than or equal to the max width defined, include it
			else return i >= index && position.offset+position.size <= length+positions[index].offset;
		});
		//return the data
		return (fromEnd)?{start: index-range.length, end: index, elements: range}
					 :{start: index, end: range.length+index, elements: range};
	},
	inRange: function(index) {
		//calculate the range
		var range = this.calculateRange();
		//return the result
		return index < range.end && index >= range.start;
	},
	setupEvents: function(){
		$(this.options.container).addEvents({
			"mouseleave": function() {
				if(this.inFocus) this.inFocus = null;
				this.imgOut(null, true);
			}.bind(this)
		});
		
		$$(this.options.backScrollButtons).each(function(el){
			el.addEvents({
				click: this.pageBack.bind(this),
				mouseover: function(){ this.addClass('hover'); },
				mouseout: function(){ this.removeClass('hover'); }
			});
		}, this);
		$$(this.options.forwardScrollButtons).each(function(el){
			el.addEvents({
				click: this.pageForward.bind(this),
				mouseover: function(){ this.addClass('hover'); },
				mouseout: function(){ this.removeClass('hover'); }
			});
		}, this);

		$$(this.options.clearLinks).each(function(el){
			el.addEvent('click', this.empty.bind(this));
		}, this);
		$$(this.options.removeLinks).each(function(el){
			el.addEvent('click', this.removeSelected.bind(this));
		}, this);
	},
	imgOver: function(img){
		//set the value of what's in focus to be this image
		this.inFocus = img;
		//clear the overTimeout
		$clear(this.overTimeout);
		//delay for the duration of the onFocusDelay option
		this.overTimeout = (function(){
			this.onFocusDelay = this.options.onFocusDelay;
			//if the user is still focused on the image, fire the onFocus event
			if (this.inFocus == img) this.fireEvent("onFocus", [img, this.imgs.indexOf(img)]);
		}).delay(this.onFocusDelay, this);
	},
	imgOut: function(img, force){
		if(!$defined(img) && force) img = this.prevFocus||this.imgs[0];
		//if the focused image is this one
		if(this.inFocus == img && img) {
			//set it to null
			this.inFocus = null;
			//clear the delay timeout
			$clear(this.outTimeout);
			//wait the duration of the onBlurDelay
			this.outTimeout = (function(){
				this.prevFocus = img;
				//if we're not still focused on this image, fire onBlur
				if (this.inFocus != img || (img == null && force)) this.fireEvent("onBlur", [img, this.imgs.indexOf(img)]);
				if (!this.inFocus) this.onFocusDelay = this.options.initialFocusDelay;
			}).delay(this.options.onBlurDelay, this);
		}
	},
	setupIconEvents: function(img, caption){
		//add the click event
		img.addEvents({
			click: function(e){
				//if the user is holding down control, select the image
				if(e.control||e.meta) {
					this.selectItem(this.imgs.indexOf(img));
					e.stop();
				}
			}.bind(this)
		});
		img.getParent().addEvents({
			mouseover: this.imgOver.bind(this, img),
			mouseout: this.imgOver.bind(this, img)
		});
	}
});

/*
Script: modalizer.js
	Defines Modalizer: functionality to overlay the window contents with a semi-transparent layer that prevents interaction with page content until it is removed

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var Modalizer = new Class({
	defaultModalStyle: {
		display:'block',
		position:'fixed',
		top:0,
		left:0,	
		'z-index':5000,
		'background-color':'#333',
		opacity:0.8
	},
	setModalOptions: function(options){
		this.modalOptions = $merge({
			width:(window.getScrollSize().x+300),
			height:(window.getScrollSize().y+300),
			elementsToHide: 'select',
			hideOnClick: true,
			modalStyle: {},
			updateOnResize: true,
			layerId: 'modalOverlay',
			onModalHide: $empty,
			onModalShow: $empty
		}, this.modalOptions, options);
		return this;
	},
	layer: function(){
		if (!this.modalOptions.layerId) this.setModalOptions();
		return $(this.modalOptions.layerId) || new Element('div', {id: this.modalOptions.layerId}).inject(document.body);
	},
	resize: function(){
		if(this.layer()) {
			this.layer().setStyles({
				width:(window.getScrollSize().x+300),
				height:(window.getScrollSize().y+300)
			});
		}
	},
	setModalStyle: function (styleObject){
		this.modalOptions.modalStyle = styleObject;
		this.modalStyle = $merge(this.defaultModalStyle, {
			width:this.modalOptions.width,
			height:this.modalOptions.height
		}, styleObject);
		if(this.layer()) this.layer().setStyles(this.modalStyle);
		return(this.modalStyle);
	},
	modalShow: function(options){
		this.setModalOptions(options);
		this.layer().setStyles(this.setModalStyle(this.modalOptions.modalStyle));
		if(Browser.Engine.trident4) this.layer().setStyle('position','absolute');
		this.layer().removeEvents('click').addEvent('click', function(){
			this.modalHide(this.modalOptions.hideOnClick);
		}.bind(this));
		this.bound = this.bound||{};
		if(!this.bound.resize && this.modalOptions.updateOnResize) {
			this.bound.resize = this.resize.bind(this);
			window.addEvent('resize', this.bound.resize);
		}
		if ($type(this.modalOptions.onModalShow)  == "function") this.modalOptions.onModalShow();
		this.togglePopThroughElements(0);
		this.layer().setStyle('display','block');
		return this;
	},
	modalHide: function(override, force){
		if(override === false) return false; //this is internal, you don't need to pass in an argument
		this.togglePopThroughElements(1);
		if ($type(this.modalOptions.onModalHide) == "function") this.modalOptions.onModalHide();
		this.layer().setStyle('display','none');
		if(this.modalOptions.updateOnResize) {
			this.bound = this.bound||{};
			if(!this.bound.resize) this.bound.resize = this.resize.bind(this);
			window.removeEvent('resize', this.bound.resize);
		}
		return this;
	},
	togglePopThroughElements: function(opacity){
		if(Browser.Engine.trident4 || (Browser.Engine.gecko && Browser.Platform.mac)) {
			$$(this.modalOptions.elementsToHide).each(function(sel){
				sel.setStyle('opacity', opacity);
			});
		}
	}
});

/*
Script: ObjectBrowser.js
	Creates a tree view of any javascript object.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

var ObjectBrowser = new Class({
	Implements: [Options, Events],
	options: {
//	onLeafClick: $empty,
		onBranchClick: function(data){
			this.showLevel(data.path?data.path+'.'+data.key:data.key, data.nodePath);
		},
		initPath: '',
		buildOnInit: true,
		data: {},
		excludeKeys: [],
		includeKeys: []
	},
	initialize: function(container, options){
		this.container = $(container);
		this.setOptions(options);
		this.data = $H(this.options.data);
		this.levels = {};
		this.elements = {};
		if(this.options.buildOnInit) this.showLevel(this.options.initPath, this.container);
	},
	toElement: function(){
		return this.container;
	},
	//gets a member of the object by path; eg "fruits.apples.green" will return the value at that path.
	//path - the string path
	//parent - (boolean) if true, will return the parent of the item found ( in the example above, fruits.apples)
	getMemberByPath: function(path, parent){
		if (path === "" || path == "top") return this.data.getClean();
		var h = parent?$H(parent):this.data;
		return h.getFromPath(path);
	},
	//replaceMemberByPath will set the location at the path to the value passed in
	replaceMemberByPath: function(path, value){
		if (path === "" || path == "top") return this.data = $H(value);
		var parentObj = this.getMemberByPath( path, true );
		parentObj[path.split(".").pop()] = value;
		return this.data;
	},
	//gets the path for a given dom node.
	getPathByNode: function(el) {
		return $H(this.elements).keyOf(el);
	},
	//validates that a key is a valid node value
	//against options.includeKeys and options.excludeKeys
	validLevel: function(key){
		return (!this.options.excludeKeys.contains(key) && 
			 (!this.options.includeKeys.length || this.options.includeKeys.contains(key)));
	},
	//builds a level into the interface given a path
	buildLevel:function(path) {
		//if the path ends in a dot, remove it
		if (path.test(".$")) path = path.substring(0, path.length);
		//get the corresponding level for the path
		var level = this.getMemberByPath(path);
		//if the path already has been built, return
		if (this.levels[path]) return this.levels[path];
		//create the section
		var section = new Element('ul');
		switch($type(level)) {
			case "function":
					this.buildNode(level, "function()", section, path, true);
				break;
			case "string": case "number":
					this.buildNode(level, null, section, path, true);
				break;
			case "array":
				level.each(function(node, index){
					this.buildNode(node, index, section, path, ["string", "function"].contains($type(node)));
				}.bind(this));
				break;
			default:
				$H(level).each(function(value, key){
					var db = false;
					if (key == "element_dimensions") db = true;
					if (db) dbug.log(key);
					if (this.validLevel(key)) {
						if (db) dbug.log('is valid level');
						var isLeaf;
						if ($type(value) == "object") {
							isLeaf = false;
							$each(value, function(v, k){
								if (this.validLevel(k)) {
									if (db) dbug.log('not a leaf!');
									isLeaf = false;
								} else {
									isLeaf = true;
								}
							}, this);
							if (isLeaf) value = false;
						}
						if (db) dbug.log(value, key, section, path, $chk(isLeaf)?isLeaf:null);
						this.buildNode(value, key, section, path, $chk(isLeaf)?isLeaf:null);
					}
				}, this);
		}
		//set the resulting DOM element to the levels map
		this.levels[path] = section;
		//return the section
		return section;
	},
	//gets the parent node for an element
	getParentFromPath: function(path){
		return this.elements[(path || "top")+'NODE'];
	},
	//displays a level given a path
	//if the level hasn't been built yet,
	//the level is built and then injected
	//into the target using the given method
	//example:
	//showLevel("fruit.apples", "fruit", "injectInside");
	//note that target and method are set to the parent path and injectInside by default
	showLevel: function(path, target, method){
		target = target || path;
		if (! this.elements[path]) 
			this.elements[path] = this.buildLevel(path)[method||"inject"](this.elements[target]||this.container);
		else this.elements[path].toggle();
		dbug.log('toggle class');
		this.elements[path].getParent().toggleClass('collapsed');
		return this;
	},
	//builds a node given the arguments:
	//value - the value of the node
	//key - the key of the node
	//section - the container where this node goes; typically a section generated by buildLevel
	//path - the path to this node
	//leaf - boolean; true if this is a leaf node
	//note: if the key or the value is an empty string, leaf will be set to true.
	buildNode: function(value, key, section, path, leaf){
		if (key==="" || value==="") leaf = true;
		if(!this.validLevel(key)) return null;
		var nodePath = (path?path+'.'+key:key)+'NODE';
		var lnk = this.buildLink((leaf)?value||key:$chk(key)?key:value, leaf);
		var li = new Element('li').addClass((leaf)?'leaf':'branch collapsed').adopt(lnk).inject(section);
		lnk.addEvent('click', function(e){
			e.stopPropagation();
			if (leaf) {
				this.fireEvent('onLeafClick', {
					li: li, 
					key: key, 
					value: value, 
					path: path,
					nodePath: nodePath,
					event: e
				});
			} else {
				this.fireEvent('onBranchClick', {
					li: li, 
					key: key, 
					value: value, 
					path: path,
					nodePath: nodePath,
					event: e
				});
			}							
		}.bind(this));
		this.elements[nodePath] = li;
		return li;
	},
	//builds a link for a given key
	buildLink: function(key) {
		if($type(key) == "function") {
			key = key.toString();
			key = key.substring(0, key.indexOf("{")+1)+"...";
		}
		return new Element('a', {
			href: "javascript: void(0);"
		}).set('html', key);
	}
});

/*
Script: PopupDetails.js
	Creates hover detail popups for a collection of elements and data.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var PopupDetail = new Class({
	Implements: [Options, Events],
	visible: false,
	observed: false,
	hasData: false,
	options: {
		observerAction: 'mouseenter', //or click
		closeOnMouseOut: true,
		linkPopup: false, //or true to use observer href, or url
		data: {}, //key/value parse to parse in to html
		templateOptions: {}, //see simple template parser
		useAjax: false,
		ajaxOptions:{
			method: 'get'
		},
		ajaxLink: false, //defaults to use observer.src
		ajaxCache: {},
		delayOn: 100,
		delayOff: 100,
		stickyWinOptions:{},
//	stickyWinToUse: null,
		showNow: false,
		htmlResponse: false,
		regExp: /\\?%([^%]+)%/g
/*	onPopupShow: $empty,
		onPopupHide: $empty */
	},
	initialize: function(html, observer, options){
		this.setOptions(options);
		try {
			this.options.stickyWinToUse = this.options.stickyWinToUse || StickyWin.Fx;
		} catch(e) {
			this.options.stickyWinToUse = StickyWin;
		}
		this.observer = $(observer);
		this.html = ($(html))?$(html).get('html'):html||'';
		if(this.options.showNow) this.show.delay(this.options.delayOn, this);
		this.setUpObservers();
	},
	setUpObservers: function(){
		var opt = this.options; //saving bytes here
		this.observer.addEvent(opt.observerAction, function(){
			this.observed = true;
			this.show.delay(opt.delayOn, this);
		}.bind(this));
		if((opt.observerAction == "mouseenter" || opt.observerAction == "mouseover") && this.options.closeOnMouseOut){
			this.observer.addEvent("mouseleave", function(){
				this.observed = false;
				this.hide.delay(opt.delayOff, this);
			}.bind(this));
		}
		return this;
	},
	parseTemplate: function(string, values){
		return string.substitute(values, this.options.regExp);
	},
	makePopup: function(){
		if(!this.stickyWin){
			var opt = this.options;//saving bytes
			if (opt.htmlResponse) this.content = this.data;
			else this.content = this.parseTemplate(this.html, opt.data);
			this.stickyWin = new opt.stickyWinToUse($merge(opt.stickyWinOptions, {
				relativeTo: this.observer,
				showNow: false,
				content: this.content,
				allowMultipleByClass: true
			}));
			if($(opt.linkPopup) || $type(opt.linkPopup)=='string') {
				this.stickyWin.win.setStyle('cursor','pointer').addEvent('click', function(){
					window.location.href = ($type(url)=='string')?url:url.src;
				});
			}
			this.stickyWin.win.addEvent('mouseenter', function(){
				this.observed = true;
			}.bind(this));
			this.stickyWin.win.addEvent('mouseleave', function(){
				this.observed = false;
				if(opt.closeOnMouseOut) this.hide.delay(opt.delayOff, this);
			}.bind(this));
		}
		return this;
	},
	getContent: function(){
		try {
			new Request($merge(this.options.ajaxOptions, {
					url: this.options.ajaxLink || this.observer.href,
					onSuccess: this.show.bind(this)
				})
			).send();
		} catch(e) {
			dbug.log('ajax error on PopupDetail: %s', e);
		}
	},
	show: function(data){
		var opt = this.options;
		if(data) this.data = data;
		if(this.observed && !this.visible) {
			if(opt.useAjax && !this.data) {
				var cachedVal = opt.ajaxCache[this.options.ajaxLink] || opt.ajaxCache[this.observer.href];
				if (cachedVal) {
					this.fireEvent('onPopupShow', this);
					return this.show(cachedVal);
				}
				this.cursorStyle = this.observer.getStyle('cursor');
				this.observer.setStyle('cursor', 'wait');
				this.getContent();
				return false;
			} else {
				if(this.cursorStyle) this.observer.setStyle('cursor', this.cursorStyle);
				if(opt.useAjax && !opt.htmlResponse) opt.data = JSON.decode(this.data);
				this.makePopup();
				this.fireEvent('onPopupShow', this);
				this.stickyWin.show();
				this.visible = true;
				return this;
			}
		}
		return this;
	},
	hide: function(){
		if(!this.observed){
			this.fireEvent('onPopupHide');
			if(this.stickyWin)this.stickyWin.hide();
			this.visible = false;
		}
		return this;
	}
});

var PopupDetailCollection = new Class({
	Implements: [Options],
	options: {
		details: {},
		links: [],
		ajaxLinks: [],
		useCache: true,
		template: '',
		popupDetailOptions: {}
	},
	cache: {},
	initialize: function(observers, options) {
		this.observers = $$(observers);
		this.setOptions(options);
		var ln = this.options.ajaxLinks.length;
		if(ln <= 0) ln = this.options.details.length;
		if (this.observers.length != ln) 
			dbug.log("warning: observers and details are out of sync.");
		this.makePopupDetails();
	},
	makePopupDetails: function(){
		this.popupDetailObjs = this.observers.map(function(observer, index){
			var opt = this.options.popupDetailOptions;//saving bytes
			var pd = new PopupDetail(this.options.template, observer, $merge(opt, {
				data: $pick(this.options.details[index], {}),
				linkItem: $pick(this.options.links[index], $pick(opt.linkItem, false)),
				ajaxLink: $pick(this.options.ajaxLinks[index], false),
				ajaxCache: (this.options.useCache)?this.cache:{},
				useAjax: this.options.ajaxLinks.length>0
			}));
			return pd;
		}, this);
	}
});


/*
Script: StyleWriter.js

Provides a simple method for injecting a css style element into the DOM if it's not already present.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

var StyleWriter = new Class({
	createStyle: function(css, id) {
		window.addEvent('domready', function(){
			try {
				if($(id) && id) return;
				var style = new Element('style', {id: id||''}).inject($$('head')[0]);
				if (Browser.Engine.trident) style.styleSheet.cssText = css;
				else style.set('text', css);
			}catch(e){dbug.log('error: %s',e);}
		}.bind(this));
	}
});

/*
Script: StickyWin.js

Creates a div within the page with the specified contents at the location relative to the element you specify; basically an in-page popup maker.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

var StickyWin = new Class({
	Implements: [Options, Events, StyleWriter, ToElement],
	options: {
//		onDisplay: $empty,
//		onClose: $empty,
		closeClassName: 'closeSticky',
		pinClassName: 'pinSticky',
		content: '',
		zIndex: 10000,
		className: '',
//		id: ... set above in initialize function
/*  	these are the defaults for setPosition anyway
		************************************************
		edge: false, //see Element.setPosition
		position: 'center', //center, corner == upperLeft, upperRight, bottomLeft, bottomRight
		offset: {x:0,y:0},
		relativeTo: document.body, */
		width: false,
		height: false,
		timeout: -1,
		allowMultipleByClass: false,
		allowMultiple: true,
		showNow: true,
		useIframeShim: true,
		iframeShimSelector: ''
	},
	css: '.SWclearfix:after {content: "."; display: block; height: 0; clear: both; visibility: hidden;}'+
			 '.SWclearfix {display: inline-table;}'+
			 '* html .SWclearfix {height: 1%;}'+
			 '.SWclearfix {display: block;}',
	initialize: function(options){
		this.options.inject = {
			target: document.body,
			where: 'bottom' 
		};
		this.setOptions(options);
		
		this.id = this.options.id || 'StickyWin_'+new Date().getTime();
		this.makeWindow();

		if(this.options.content) this.setContent(this.options.content);
		if(this.options.timeout > 0) {
			this.addEvent('onDisplay', function(){
				this.hide.delay(this.options.timeout, this)
			}.bind(this));
		}
		if(this.options.showNow) this.show();
		//add css for clearfix
		this.createStyle(this.css, 'StickyWinClearFix');
	},
	makeWindow: function(){
		this.destroyOthers();
		if(!$(this.id)) {
			this.win = new Element('div', {
				id:		this.id
			}).addClass(this.options.className).addClass('StickyWinInstance').addClass('SWclearfix').setStyles({
			 	display:'none',
				position:'absolute',
				zIndex:this.options.zIndex
			}).inject(this.options.inject.target, this.options.inject.where).store('StickyWin', this);			
		} else this.win = $(this.id);
		this.element = this.win;
		if(this.options.width && $type(this.options.width.toInt())=="number") this.win.setStyle('width', this.options.width.toInt());
		if(this.options.height && $type(this.options.height.toInt())=="number") this.win.setStyle('height', this.options.height.toInt());
		return this;
	},
	show: function(suppressEvent){
		this.showWin();
		if (!suppressEvent) this.fireEvent('onDisplay');
		if(this.options.useIframeShim) this.showIframeShim();
		this.visible = true;
		return this;
	},
	showWin: function(){
		if(!this.positioned) this.position();
		this.win.show();
	},
	hide: function(suppressEvent){
		if(!suppressEvent) this.fireEvent('onClose');
		this.hideWin();
		if(this.options.useIframeShim) this.hideIframeShim();
		this.visible = false;
		return this;
	},
	hideWin: function(){
		this.win.setStyle('display','none');
	},
	destroyOthers: function() {
		if(!this.options.allowMultipleByClass || !this.options.allowMultiple) {
			$$('div.StickyWinInstance').each(function(sw) {
				if(!this.options.allowMultiple || (!this.options.allowMultipleByClass && sw.hasClass(this.options.className))) 
					sw.retrieve('StickyWin').destroy();
			}, this);
		}
	},
	setContent: function(html) {
		if(this.win.getChildren().length>0) this.win.empty();
		if($type(html) == "string") this.win.set('html', html);
		else if ($(html)) this.win.adopt(html);
		this.win.getElements('.'+this.options.closeClassName).each(function(el){
			el.addEvent('click', this.hide.bind(this));
		}, this);
		this.win.getElements('.'+this.options.pinClassName).each(function(el){
			el.addEvent('click', this.togglepin.bind(this));
		}, this);
		return this;
	},	
	position: function(options){
		this.positioned = true;
		this.setOptions(options);
		this.win.setPosition({
			allowNegative: true,
			relativeTo: this.options.relativeTo,
			position: this.options.position,
			offset: this.options.offset,
			edge: this.options.edge
		});
		if(this.shim) this.shim.position();
		return this;
	},
	pin: function(pin) {
		if(!this.win.pin) {
			dbug.log('you must include element.pin.js!');
			return this;
		}
		this.pinned = $pick(pin, true);
		this.win.pin(pin);
		return this;
	},
	unpin: function(){
		return this.pin(false);
	},
	togglepin: function(){
		return this.pin(!this.pinned);
	},
	makeIframeShim: function(){
		if(!this.shim){
			var el = (this.options.iframeShimSelector)?this.win.getElement(this.options.iframeShimSelector):this.win;
			this.shim = new IframeShim(el, {
				display: false,
				name: 'StickyWinShim'
			});
		}
	},
	showIframeShim: function(){
		if(this.options.useIframeShim) {
			this.makeIframeShim();
			this.shim.show();
		}
	},
	hideIframeShim: function(){
		if(this.shim) this.shim.hide();
	},
	destroy: function(){
		if (this.win) this.win.dispose();
		if(this.options.useIframeShim && this.shim) this.shim.dispose();
		if($('modalOverlay'))$('modalOverlay').dispose();
	}
});


/*
Script: StickyWin.Fx.js

Extends StickyWin to create popups that fade in and out and can be dragged and resized (requires StickyWin.Fx.Drag.js).

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
StickyWin.Fx = new Class({
	Extends: StickyWin,
	options: {
		fade: true,
		fadeDuration: 150,
//	fadeTransition: 'sine:in:out',
		draggable: false,
		dragOptions: {},
		dragHandleSelector: '.dragHandle',
		resizable: false,
		resizeOptions: {},
		resizeHandleSelector: ''
	},
	setContent: function(html){
		this.parent(html);
		if(this.options.draggable) this.makeDraggable();
		if(this.options.resizable) this.makeResizable();
		return this;
	},	
	hideWin: function(){
		if(this.options.fade) this.fade(0);
		else this.parent();
	},
	showWin: function(){
		if(this.options.fade) this.fade(1);
		else this.parent();
	},
	hide: function(){
		this.parent(this.options.fade);
	},
	show: function(){
		this.parent(this.options.fade);
	},
	fade: function(to){
		if(!this.fadeFx) {
			this.win.setStyles({
				opacity: 0,
				display: 'block'
			});
			var opts = {
				property: 'opacity',
				duration: this.options.fadeDuration
			};
			if (this.options.fadeTransition) opts.transition = this.options.fadeTransition;
			this.fadeFx = new Fx.Tween(this.win, opts);
		}
		if (to > 0) {
			this.win.setStyle('display','block');
			this.position();
		}
		this.fadeFx.clearChain();
		this.fadeFx.start(to).chain(function (){
			if(to == 0) {
				this.win.setStyle('display', 'none');
				this.fireEvent('onClose');
			} else {
				this.fireEvent('onDisplay');
			}
		}.bind(this));
		return this;
	},
	makeDraggable: function(){
		dbug.log('you must include Drag.js, cannot make draggable');
	},
	makeResizable: function(){
		dbug.log('you must include Drag.js, cannot make resizable');
	}
});
//legacy
var StickyWinFx = StickyWin.Fx;

/*
Script: StickyWin.Fx.Drag.js

Implements drag and resize functionaity into StickyWin.Fx. See StickyWin.Fx for the options.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

if(typeof Drag != "undefined"){
	StickyWin.Fx.implement({
		makeDraggable: function(){
			var toggled = this.toggleVisible(true);
			if(this.options.useIframeShim) {
				this.makeIframeShim();
				var onComplete = (this.options.dragOptions.onComplete || $empty);
				this.options.dragOptions.onComplete = function(){
					onComplete();
					this.shim.position();
				}.bind(this);
			}
			if(this.options.dragHandleSelector) {
				var handle = this.win.getElement(this.options.dragHandleSelector);
				if (handle) {
					handle.setStyle('cursor','move');
					this.options.dragOptions.handle = handle;
				}
			}
			this.win.makeDraggable(this.options.dragOptions);
			if (toggled) this.toggleVisible(false);
		}, 
		makeResizable: function(){
			var toggled = this.toggleVisible(true);
			if(this.options.useIframeShim) {
				this.makeIframeShim();
				var onComplete = (this.options.resizeOptions.onComplete || $empty);
				this.options.resizeOptions.onComplete = function(){
					onComplete();
					this.shim.position();
				}.bind(this);
			}
			if(this.options.resizeHandleSelector) {
				var handle = this.win.getElement(this.options.resizeHandleSelector);
				if(handle) this.options.resizeOptions.handle = this.win.getElement(this.options.resizeHandleSelector);
			}
			this.win.makeResizable(this.options.resizeOptions);
			if (toggled) this.toggleVisible(false);
		},
		toggleVisible: function(show){
			if(!this.visible && Browser.Engine.webkit && $pick(show, true)) {
				this.win.setStyles({
					display: 'block',
					opacity: 0
				});
				return true;
			} else if(!$pick(show, false)){
				this.win.setStyles({
					display: 'none',
					opacity: 1
				});
				return false;
			}
			return false;
		}
	});
}


/*
Script: StickyWin.Modal.js

This script extends StickyWin and StickyWin.Fx classes to add Modalizer functionality.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
(function(){
var modalWinBase = function(extend){
	return {
		Extends: extend,
		initialize: function(options){
			options = options||{};
			this.setModalOptions($merge(options.modalOptions||{}, {
				onModalHide: function(){
						this.hide(false);
					}.bind(this)
				}));
			this.parent(options);
		},
		show: function(showModal){
			if($pick(showModal, true)) {
				this.modalShow();
				if (this.modalOptions.elementsToHide) this.win.getElements(this.modalOptions.elementsToHide).setStyle('opacity', 1);
			}
			this.parent();
		},
		hide: function(hideModal){
			if($pick(hideModal, true)) this.modalHide();
			this.parent();
		}
	}
};

StickyWin.Modal = new Class(modalWinBase(StickyWin));
StickyWin.Modal.implement(new Modalizer());
if (StickyWin.Fx) StickyWin.Fx.Modal = new Class(modalWinBase(StickyWin.Fx));
try { StickyWin.Fx.Modal.implement(new Modalizer()); }catch(e){}
})();
//legacy
var StickyWinModal = StickyWin.Modal;
if (StickyWin.Fx) var StickyWinFxModal = StickyWin.Fx.Modal;

/*
Script: StickyWin.Ajaxjs

Adds ajax functionality to all the StickyWin classes.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
(function(){
	var SWA = function(extend){
		return {
			Extends: extend,
			options: {
				url: '',
				showNow: false,
				requestOptions: {
					method: 'get'
				},
				wrapWithUi: false, 
				caption: '',
				uiOptions:{},
				handleResponse: function(response){
					var responseScript = "";
					this.Request.response.text.stripScripts(function(script){	responseScript += script; });
					if(this.options.wrapWithUi) response = StickyWin.ui(this.options.caption, response, this.options.uiOptions);
					this.setContent(response);
					this.show();
					if (this.evalScripts) $exec(responseScript);
				}
			},
			initialize: function(options){
				this.parent(options);
				this.evalScripts = this.options.requestOptions.evalScripts;
				this.options.requestOptions.evalScripts = false;
				this.createRequest();
			},
			createRequest: function(){
				this.Request = new Request(this.options.requestOptions).addEvent('onSuccess',
					this.options.handleResponse.bind(this));
			},
			update: function(url, options){
				this.Request.setOptions(options).send({url: url||this.options.url});
				return this;
			}
		};
	};
	try {	StickyWin.Ajax = new Class(SWA(StickyWin)); } catch(e){}
	try {	StickyWin.Fx.Ajax = new Class(SWA(StickyWin.Fx)); } catch(e){}
	try {	StickyWin.Modal.Ajax = new Class(SWA(StickyWin.Modal)); } catch(e){}
	try {	StickyWin.Fx.Modal.Ajax = new Class(SWA(StickyWin.Fx.Modal)); } catch(e){}
})();
//legacy
if (window.StickyWinModal) StickyWinModal.Ajax = StickyWin.Modal.Ajax;
if (StickyWin.Fx) {
	StickyWinFx.Ajax = StickyWin.Fx.Ajax;
	StickyWinFxModal.Ajax = StickyWin.Fx.Modal.Ajax;
}


/*
Script: StickyWin.alert.js
	Defines StickyWin.alert, a simple little alert box with a close button.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
StickyWin.alert = function(msghdr, msg, baseHref) {
	baseHref = baseHref||"http://www.cnet.com/html/rb/assets/global/simple.error.popup";
	msg = '<p class="errorMsg SWclearfix" style="margin: 0px;min-height:10px">' +
						'<img src="'+baseHref+'/icon_problems_sm.gif"'+
						' class="bang clearfix" style="float: left; width: 30px; height: 30px; margin: 3px 5px 5px 0px;">'
						 + msg + '</p>';
	var body = StickyWin.ui(msghdr, msg, {width: 250});
	return new StickyWin.Modal({
		modalOptions: {
			modalStyle: {
				zIndex: 11000
			}
		},
		zIndex: 110001,
		content: body,
		position: 'center' //center, corner
	});
};

/*
Script: StickyWin.ui.js

Creates an html holder for in-page popups using a default style.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
StickyWin.UI = new Class({
	Implements: [Options, ToElement, StyleWriter],
	options: {
		width: 300,
		css: "div.DefaultStickyWin div.body{font-family:verdana; font-size:11px; line-height: 13px;}"+
			"div.DefaultStickyWin div.top_ul{background:url({%baseHref%}full.png) top left no-repeat; height:30px; width:15px; float:left}"+
			"div.DefaultStickyWin div.top_ur{position:relative; left:0px !important; left:-4px; background:url({%baseHref%}full.png) top right !important; height:30px; margin:0px 0px 0px 15px !important; margin-right:-4px; padding:0px}"+
			"div.DefaultStickyWin h1.caption{clear: none !important; margin:0px 5px 0px 0px !important; overflow: hidden; padding:0 !important; font-weight:bold; color:#555; font-size:14px !important; position:relative; top:8px !important; left:5px !important; float: left; height: 22px !important;}"+
			"div.DefaultStickyWin div.middle, div.DefaultStickyWin div.closeBody {background:url({%baseHref%}body.png) top left repeat-y; margin:0px 20px 0px 0px !important;	margin-bottom: -3px; position: relative;	top: 0px !important; top: -3px;}"+
			"div.DefaultStickyWin div.body{background:url({%baseHref%}body.png) top right repeat-y; padding:8px 30px 8px 0px !important; margin-left:5px !important; position:relative; right:-20px !important;}"+
			"div.DefaultStickyWin div.bottom{clear:both}"+
			"div.DefaultStickyWin div.bottom_ll{background:url({%baseHref%}full.png) bottom left no-repeat; width:15px; height:15px; float:left}"+
			"div.DefaultStickyWin div.bottom_lr{background:url({%baseHref%}full.png) bottom right; position:relative; left:0px !important; left:-4px; margin:0px 0px 0px 15px !important; margin-right:-4px; height:15px}"+
			"div.DefaultStickyWin div.closeButtons{text-align: center; background:url({%baseHref%}body.png) top right repeat-y; padding: 0px 30px 8px 0px; margin-left:5px; position:relative; right:-20px}"+
			"div.DefaultStickyWin a.button:hover{background:url({%baseHref%}big_button_over.gif) repeat-x}"+
			"div.DefaultStickyWin a.button {background:url({%baseHref%}big_button.gif) repeat-x; margin: 2px 8px 2px 8px; padding: 2px 12px; cursor:pointer; border: 1px solid #999 !important; text-decoration:none; color: #000 !important;}"+
			"div.DefaultStickyWin div.closeButton{width:13px; height:13px; background:url({%baseHref%}closebtn.gif) no-repeat; position: absolute; right: 0px; margin:10px 15px 0px 0px !important; cursor:pointer;top:0px}"+
			"div.DefaultStickyWin div.dragHandle {	width: 11px;	height: 25px;	position: relative;	top: 5px;	left: -3px;	cursor: move;	background: url({%baseHref%}drag_corner.gif); float: left;}",
		cornerHandle: false,
		cssClass: '',
		baseHref: 'http://www.cnet.com/html/rb/assets/global/stickyWinHTML/',
		buttons: [],
		cssId: 'defaultStickyWinStyle',
		cssClassName: 'DefaultStickyWin',
		closeButton: true
/*	These options are deprecated:		
		closeTxt: false,
		onClose: $empty,
		confirmTxt: false,
		onConfirm: $empty	*/
	},
	initialize: function() {
		var args = this.getArgs(arguments);
		this.setOptions(args.options);
		this.legacy();
		var css = this.options.css.substitute({baseHref: this.options.baseHref}, /\\?\{%([^}]+)%\}/g);
		if (Browser.Engine.trident4) css = css.replace(/png/g, 'gif');
		this.createStyle(css, this.options.cssId);
		this.build();
		this.setContent(args.caption, args.body);
	},
	getArgs: function(){
		return StickyWin.UI.getArgs.apply(this, arguments);
	},
	legacy: function(){
		var opt = this.options; //saving bytes
		//legacy support
		if(opt.confirmTxt) opt.buttons.push({text: opt.confirmTxt, onClick: opt.onConfirm || $empty});
		if(opt.closeTxt) opt.buttons.push({text: opt.closeTxt, onClick: opt.onClose || $empty});
	},
	build: function(){
		var opt = this.options;

		var container = new Element('div', {
			'class': opt.cssClassName
		});
		if (opt.width) container.setStyle('width', opt.width);
		this.element = container;
		this.element.store('StickyWinUI', this);
		if(opt.cssClass) container.addClass(opt.cssClass);
		

		var bodyDiv = new Element('div').addClass('body');
		this.body = bodyDiv;
		
		var top_ur = new Element('div').addClass('top_ur');
		this.top_ur = top_ur;
		this.top = new Element('div').addClass('top').adopt(
				new Element('div').addClass('top_ul')
			).adopt(top_ur);
		container.adopt(this.top);
		
		if(opt.cornerHandle) new Element('div').addClass('dragHandle').inject(top_ur, 'top');
		
		//body
		container.adopt(new Element('div').addClass('middle').adopt(bodyDiv));
		//close buttons
		if(opt.buttons.length > 0){
			var closeButtons = new Element('div').addClass('closeButtons');
			opt.buttons.each(function(button){
				if(button.properties && button.properties.className){
					button.properties['class'] = button.properties.className;
					delete button.properties.className;
				}
				var properties = $merge({'class': 'closeSticky'}, button.properties);
				new Element('a').addEvent('click',
					button.onClick || $empty).appendText(
					button.text).inject(closeButtons).setProperties(properties).addClass('button');
			});
			container.adopt(new Element('div').addClass('closeBody').adopt(closeButtons));
		}
		//footer
		container.adopt(
			new Element('div').addClass('bottom').adopt(
					new Element('div').addClass('bottom_ll')
				).adopt(
					new Element('div').addClass('bottom_lr')
			)
		);
		if (this.options.closeButton) container.adopt(new Element('div').addClass('closeButton').addClass('closeSticky'));
		return this;
	},
	makeCaption: function(caption) {
		if (!caption) return this.destroyCaption();
		this.caption = caption;
		var opt = this.options;
		var h1Caption = new Element('h1').addClass('caption');
		if (opt.width) h1Caption.setStyle('width', (opt.width-(opt.cornerHandle?70:60)));
		if($(this.caption)) h1Caption.adopt(this.caption);
		else h1Caption.set('html', this.caption);
		this.top_ur.adopt(h1Caption);
		this.h1 = h1Caption;
		if(!this.options.cornerHandle) this.h1.addClass('dragHandle');
		return this;
	},
	destroyCaption: function(){
		if (this.h1) {
			this.h1.destroy();
			this.h1 = null;
		}
		return this;
	},
	setContent: function(){
		var args = this.getArgs.apply(this, arguments);
		var caption = args.caption;
		var body = args.body;
		if (this.h1) this.destroyCaption();
		this.makeCaption(caption);
		if ($(body)) this.body.empty().adopt(body);
		else this.body.set('html', body);	
		return this;
	}
});
StickyWin.UI.getArgs = function(){
	var input = $type(arguments[0]) == "arguments"?arguments[0]:arguments;
	var cap = input[0], bod = input[1];
	var args = Array.link(input, {options: Object.type});
	if (input.length == 3 || (!args.options && input.length == 2)) {
		args.caption = cap;
		args.body = bod;
	} else if (($type(bod) == 'object' || !bod) && cap && $type(cap) != 'object'){
		args.body = cap;
	}
	return args;
};

StickyWin.ui = function(caption, body, options){
	return $(new StickyWin.UI(caption, body, options))
};


/*
Script: StickyWin.ui.pointy.js

Creates an html holder for in-page popups using a default style - this one including a pointer in the specified direction.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
StickyWin.UI.Pointy = new Class({
	Extends: StickyWin.UI,
	options: {
		theme: 'dark',
		themes: {
			dark: {
				bgColor: '#333',
				fgColor: '#ddd',
				imgset: 'dark'
			},
			light: {
				bgColor: '#ccc',
				fgColor: '#333',
				imgset: 'light'
			}
		},
		css: "div.DefaultPointyTip {position: relative}"+
		"div.DefaultPointyTip div.body{background: {%bgColor%}; color: {%fgColor%}; right: 0px !important;padding:  0px 10px !important;margin-left: 0px !important;font-family: verdana;font-size: 11px;line-height: 13px;position: relative;}"+
		"div.DefaultPointyTip div.top {position: relative;height: 25px; overflow: visible}"+
		"div.DefaultPointyTip div.top_ul{background: url({%baseHref%}{%imgset%}_back.png) top left no-repeat;width: 8px;height: 25px; position: absolute; left: 0px;}"+
		"div.DefaultPointyTip div.top_ur{background: url({%baseHref%}{%imgset%}_back.png) top right !important;margin: 0 0 0 8px !important;height: 25px;position: relative;left: 0px !important;padding: 0;}"+
		"div.DefaultPointyTip h1.caption{color: {%fgColor%};left: 0px !important;top: 4px !important;clear: none !important;overflow: hidden;font-weight: 700;font-size: 12px !important;position: relative;float: left;height: 22px !important;margin: 0 22px 0 0 !important;padding: 0 !important;}"+
		"div.DefaultPointyTip div.middle,div.DefaultPointyTip div.closeBody{background:  {%bgColor%};margin: 0 0px 0 0 !important;position: relative;top: 0 !important;}"+
		"div.DefaultPointyTip div.bottom {clear: both;} "+
		"div.DefaultPointyTip div.bottom_ll{font-size:1; background: url({%baseHref%}{%imgset%}_back.png) bottom left no-repeat;width: 6px;height: 6px;position: absolute; left: 0px;}"+
		"div.DefaultPointyTip div.bottom_lr{font-size:1; background: url({%baseHref%}{%imgset%}_back.png) bottom right;height: 6px;margin: 0 0 0 6px !important;position: relative;left: 0 !important;}"+
		"div.DefaultPointyTip div.noCaption{height: 6px; overflow: hidden}"+
		"div.DefaultPointyTip div.closeButton{width:13px; height:13px; background:url({%baseHref%}{%imgset%}_x.png) no-repeat; position: absolute; right: 0px; margin:4px 0px 0px !important; cursor:pointer; z-index: 1; top: 0px;}",
		baseHref: 'http://cnetjavascript.googlecode.com/svn/trunk/Assets/PointyTip/',
		divot: '{%baseHref%}{%imgset%}_divot.png',
		divotSize: 22,
		direction: 12,
		cssId: 'defaultPointyTipStyle',
		cssClassName: 'DefaultPointyTip'
	},
	initialize: function() {
		var args = this.getArgs(arguments);
		this.setOptions(args.options);
		$extend(this.options, this.options.themes[this.options.theme]);
		this.options.css = this.options.css.substitute(this.options, /\\?\{%([^}]+)%\}/g);
		if (args.options && args.options.theme) {
			while (!this.id) {
				var id = $random(0, 999999999);
				if (!StickyWin.UI.Pointy[id]) {
					StickyWin.UI.Pointy[id] = this;
					this.id = id;
				}
			}
			this.options.css = this.options.css.replace(/div\.DefaultPointyTip/g, "div#pointy_"+this.id);
			this.options.cssId = "pointyTipStyle_" + this.id;
		}
		if ($type(this.options.direction) == 'string') {
			var map = {
				left: 9,
				right: 3,
				up: 12,
				down: 6
			};
			this.options.direction = map[this.options.direction];
		}
		
		this.options.divot = this.options.divot.substitute(this.options, /\\?\{%([^}]+)%\}/g);
		if (Browser.Engine.trident4) this.options.divot = this.options.divot.replace(/png/g, 'gif');
		
		this.parent(args.caption, args.body, this.options);
		if (this.id) $(this).set('id', "pointy_"+this.id);
	},
	build: function(){
		this.parent();
		var opt = this.options;
		this.pointyWrapper = new Element('div', {
			'class': 'pointyWrapper'
		}).inject($(this));
		$(this).getChildren().each(function(el){
			if (el != this.pointyWrapper) this.pointyWrapper.grab(el);
		}, this);

		var w = opt.divotSize;
		var h = w;
		var left = (opt.width - opt.divotSize)/2;
		var orient = function(){
			switch(opt.direction) {
				case 12: case 1: case 11:
					return {
						height: h/2
					};
				case 5: case 6: case 7:
					return {
						height: h/2,
						backgroundPosition: '0 -'+h/2+'px'
					};
				case 8: case 9: case 10:
					return {
						width: w/2
					};
				case 2: case 3: case 4:
					return {
						width: w/2,
						backgroundPosition: '100%'
					};
			};
		};
		this.pointer = new Element('div', {
			styles: $extend({
				background: "url("+opt.divot+") no-repeat",
				width: w,
				height: h,
				overflow: 'hidden'
			}, orient()),
			'class': 'pointyDivot pointy_'+opt.direction
		}).inject(this.pointyWrapper);
		this.positionPointer();
	},
	expose: function(){
		if ($(this).getStyle('display') != 'none' && $(document.body).hasChild($(this))) return $empty;
		$(this).setStyles({
			visibility: 'hidden',
			position: 'absolute'
		});
		var dispose;
		if (!document.body.hasChild($(this))) {
			$(this).inject(document.body);
			dispose = true;
		}
		return (function(){
			if (dispose) $(this).dispose();
			$(this).setStyles({
				visibility: 'visible',
				position: 'relative'
			});
		}).bind(this);
	},
	positionPointer: function(options){
		if (!this.pointer) return;

		var opt = this.options;
		var pos;
		var d = opt.direction;
		switch (d){
			case 12: case 1: case 11:
				pos = {
					edge: {x: 'center', y: 'bottom'},
					position: {
						x: d==12?'center':d==1?'right':'left', 
						y: 'top'
					},
					offset: {
						x: (d==12?0:d==1?-1:1)*opt.divotSize,
						y: 1
					}
				};
				break;
			case 2: case 3: case 4:
				pos = {
					edge: {x: 'left', y: 'center'},
					position: {
						x: 'right', 
						y: d==3?'center':d==2?'top':'bottom'
					},
					offset: {
						x: -1,
						y: (d==3?0:d==4?-1:1)*opt.divotSize
					}
				};
				break;
			case 5: case 6: case 7:
				pos = {
					edge: {x: 'center', y: 'top'},
					position: {
						x: d==6?'center':d==5?'right':'left', 
						y: 'bottom'
					},
					offset: {
						x: (d==6?0:d==5?-1:1)*opt.divotSize,
						y: -1
					}
				};
				break;
			case 8: case 9: case 10:
				pos = {
					edge: {x: 'right', y: 'center'},
					position: {
						x: 'left', 
						y: d==9?'center':d==10?'top':'bottom'
					},
					offset: {
						x: 1,
						y: (d==9?0:d==8?-1:1)*opt.divotSize
					}
				};
				break;
		};
		var putItBack = this.expose();
		this.pointer.setPosition($extend({
			relativeTo: this.pointyWrapper
		}, pos, options));
		putItBack();
	},
	setContent: function(){
		this.parent.apply(this, arguments);
		this.top[this.h1?'removeClass':'addClass']('noCaption');
		if (Browser.Engine.trident4) $(this).getElements('.bottom_ll, .bottom_lr').setStyle('font-size', 1); //IE6 bullshit
		this.positionPointer();
		return this;
	}
});

StickyWin.ui.pointy = function(caption, body, options){
	return $(new StickyWin.UI.Pointy(caption, body, options));
};

StickyWin.PointyTip = new Class({
	Extends: StickyWin,
	options: {
		point: "left",
		pointyOptions: {}
	},
	initialize: function(){
		var args = this.getArgs(arguments);
		this.setOptions(args.options);
		var popts = this.options.pointyOptions;
		var d = popts.direction;
		if (!d) {
			var map = {
				left: 9,
				right: 3,
				up: 12,
				down: 6
			};
			d = map[this.options.point];
			if (!d) d = this.options.point;
			popts.direction = d;
		}
		if (!popts.width) popts.width = this.options.width;
		this.pointy = new StickyWin.UI.Pointy(args.caption, args.body, popts);
		this.options.content = null;
		
		this.setOptions(args.options, this.getPositionSettings());
		this.parent(this.options);
		this.win.empty().adopt(this.pointy);
		this.attachHandlers(this.win);
		this.position();
	},
	getArgs: function(){
		return StickyWin.UI.getArgs.apply(this, arguments);
	},
	getPositionSettings: function(){
		var s = this.pointy.options.divotSize;
		var d = this.options.point;
		switch(d) {
			case "left": case 8: case 9: case 10:
				return {
					edge: {
						x: 'left', 
						y: d==10?'top':d==8?'bottom':'center'
					},
					position: {x: 'right', y: 'center'},
					offset: {
						x: s
					}
				};
			case "right": case 2:  case 3: case 4:
				return {
					edge: {
						x: 'right', 
						y: d==2?'top':d==4?'bottom':'center'
					},
					position: {x: 'left', y: 'center'},
					offset: {x: -s}
				};
			case "up": case 11: case 12: case 1:
				return {
					edge: { 
						x: d==11?'left':d==1?'right':'center', 
						y: 'top' 
					},
					position: {	x: 'center', y: 'bottom' },
					offset: {
						y: s,
						x: d==11?-s:d==1?s:0
					}
				};
			case "down": case 5: case 6: case 7:
				return {
					edge: {
						x: d==7?'left':d==5?'right':'center', 
						y: 'bottom'
					},
					position: {x: 'center', y: 'top'},
					offset: {y: -s}
				};
		};
	},
	setContent: function() {
		var args = this.getArgs(arguments);
		this.pointy.setContent(args.caption, args.body);
		[this.pointy.h1, this.pointy.body].each(this.attachHandlers, this);
		if (this.visible) this.position();
		return this;
	},
	showWin: function(){
		this.parent.apply(this, arguments);
		this.pointy.positionPointer();
	},
	position: function(){
		this.parent.apply(this, arguments);
		this.pointy.positionPointer();
	},
	attachHandlers: function(content) {
		if (!content) return;
		content.getElements('.'+this.options.closeClassName).addEvent('click', function(){ this.hide(); }.bind(this));
		content.getElements('.'+this.options.pinClassName).addEvent('click', function(){ this.togglepin(); }.bind(this));
	}
});

Tips.Pointy = new Class({
	Extends: Tips,
	options: {
		onShow: $empty,
		onHide: $empty,
		pointyTipOptions: {
			point: 11,
			width: 150,
			pointyOptions: {
				closeButton: false
			}
		}
	},
	initialize: function(){
		var params = Array.link(arguments, {options: Object.type, elements: $defined});
		this.setOptions(params.options);
		this.tip = new StickyWin.PointyTip($extend(this.options.pointyTipOptions, {
			showNow: false
		}));
		if (this.options.className) $(this.tip).addClass(this.options.className);
		if (params.elements) this.attach(params.elements);
	},
	elementEnter: function(event, element){

		var title = element.retrieve('tip:title');
		var text = element.retrieve('tip:text');
		
		this.tip.setContent(title, text);

		this.timer = $clear(this.timer);
		this.timer = this.show.delay(this.options.showDelay, this);

		this.position(element);
	},

	elementLeave: function(event){
		$clear(this.timer);
		this.timer = this.hide.delay(this.options.hideDelay, this);
	},

	elementMove: function(event){
		return; //always fixed
	},

	position: function(element){
		this.tip.setOptions({
			relativeTo: element
		});
		this.tip.position();
	},

	show: function(){
		this.fireEvent('show', this.tip);
		this.tip.show();
	},

	hide: function(){
		this.fireEvent('hide', this.tip);
		this.tip.hide();
	}

});

/*
Script: Waiter.js

Adds a semi-transparent overlay over a dom element with a spinnin ajax icon.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var Waiter = new Class({
	Implements: [Options, Events, Chain],
	options: {
		baseHref: 'http://www.cnet.com/html/rb/assets/global/waiter/',
		containerProps: {
			styles: {
				position: 'absolute',
				'text-align': 'center'
			},
			'class':'waiterContainer'
		},
		containerPosition: {},
		msg: false,
		msgProps: {
			styles: {
				'text-align': 'center',
				fontWeight: 'bold'
			},
			'class':'waiterMsg'
		},
		img: {
			src: 'waiter.gif',
			styles: {
				width: 24,
				height: 24
			},
			'class':'waiterImg'
		},
		layer:{
			styles: {
				width: 0,
				height: 0,
				position: 'absolute',
				zIndex: 999,
				display: 'none',
				opacity: 0.9,
				background: '#fff'
			},
			'class': 'waitingDiv'
		},
		useIframeShim: true,
		fxOptions: {},
		injectWhere: null
//	iframeShimOptions: {},
//	onShow: $empty
//	onHide: $empty
	},
	initialize: function(target, options){
		this.target = $(target)||$(document.body);
		this.setOptions(options);
		this.waiterContainer = new Element('div', this.options.containerProps);
		if (this.options.msg) {
			this.msgContainer = new Element('div', this.options.msgProps);
			this.waiterContainer.adopt(this.msgContainer);
			if (!$(this.options.msg)) this.msg = new Element('p').appendText(this.options.msg);
			else this.msg = $(this.options.msg);
			this.msgContainer.adopt(this.msg);
		}
		if (this.options.img) this.waiterImg = $(this.options.img.id) || new Element('img').inject(this.waiterContainer);
		this.waiterOverlay = $(this.options.layer.id) || new Element('div').adopt(this.waiterContainer);
		this.waiterOverlay.set(this.options.layer);
		this.place(target);
		try {
			if (this.options.useIframeShim) this.shim = new IframeShim(this.waiterOverlay, this.options.iframeShimOptions);
		} catch(e) {
			dbug.log("Waiter attempting to use IframeShim but failed; did you include IframeShim? Error: ", e);
			this.options.useIframeShim = false;
		}
		this.waiterFx = this.waiterFx || new Fx.Elements($$(this.waiterContainer, this.waiterOverlay), this.options.fxOptions);
	},
	place: function(target, where){
		var where = where || this.options.injectWhere || target == document.body ? 'inside' : 'after';
		this.waiterOverlay.inject(target, where);
	},
	toggle: function(element, show) {
		//the element or the default
		element = $(element) || $(this.active) || $(this.target);
		this.place(element);
		if (!$(element)) return this;
		if (this.active && element != this.active) return this.stop(this.start.bind(this, element));
		//if it's not active or show is explicit
		//or show is not explicitly set to false
		//start the effect
		if((!this.active || show) && show !== false) this.start(element);
		//else if it's active and show isn't explicitly set to true
		//stop the effect
		else if(this.active && !show) this.stop();
		return this;
	},
	reset: function(){
		this.waiterFx.cancel().set({
			0: { opacity:[0]},
			1: { opacity:[0]}
		});
	},
	start: function(element){
		this.reset();
		element = $(element) || $(this.target);
		this.place(element);
		if (this.options.img) {
			this.waiterImg.set($merge(this.options.img, {
				src: this.options.baseHref + this.options.img.src
			}));
		}
		
		var start = function() {
			var dim = element.getComputedSize();
			this.active = element;
			this.waiterOverlay.setStyles({
				width: this.options.layer.width||dim.totalWidth,
				height: this.options.layer.height||dim.totalHeight,
				display: 'block'
			}).setPosition({
				relativeTo: element,
				position: 'upperLeft'
			});
			this.waiterContainer.setPosition($merge({
				relativeTo: this.waiterOverlay
			}, this.options.containerPosition));
			if (this.options.useIframeShim) this.shim.show();
			this.waiterFx.start({
				0: { opacity:[1] },
				1: { opacity:[this.options.layer.styles.opacity]}
			}).chain(function(){
				if (this.active == element) this.fireEvent('onShow', element);
				this.callChain();
			}.bind(this));
		}.bind(this);

		if (this.active && this.active != element) this.stop(start);
		else start();
		
		return this;
	},
	stop: function(callback){
		if (!this.active) {
			if ($type(callback) == "function") callback.attempt();
			return this;
		}
		this.waiterFx.cancel();
		this.waiterFx.clearChain();
		//fade the waiter out
		this.waiterFx.start({
			0: { opacity:[0]},
			1: { opacity:[0]}
		}).chain(function(){
			this.active = null;
			this.waiterOverlay.hide();
			if (this.options.useIframeShim) this.shim.hide();
			this.fireEvent('onHide', this.active);
			this.callChain();
			this.clearChain();
			if ($type(callback) == "function") callback.attempt();
		}.bind(this));
		return this;
	}
});

if (typeof Request != "undefined" && Request.HTML) {
	Request.HTML = Class.refactor(Request.HTML, {
		options: {
			useWaiter: false,
			waiterOptions: {},
			waiterTarget: false
		},
		initialize: function(options){
			this._send = this.send;
			this.send = function(options){
				if(this.waiter) this.waiter.start().chain(this._send.bind(this, options));
				else this._send(options);
				return this;
			};
			this.parent(options);
			if (this.options.useWaiter && ($(this.options.update) || $(this.options.waiterTarget))) {
				this.waiter = new Waiter(this.options.waiterTarget || this.options.update, this.options.waiterOptions);
				['onComplete', 'onException', 'onCancel'].each(function(event){
					this.addEvent(event, this.waiter.stop.bind(this.waiter));
				}, this);
			}
		}
	});
}

Element.Properties.waiter = {

	set: function(options){
		var waiter = this.retrieve('waiter');
		return this.eliminate('wait').store('waiter:options');
	},

	get: function(options){
		if (options || !this.retrieve('waiter')){
			if (options || !this.retrieve('waiter:options')) this.set('waiter', options);
			this.store('waiter', new Waiter(this, this.retrieve('waiter:options')));
		}
		return this.retrieve('waiter');
	}

};

Element.implement({

	wait: function(options){
		this.get('waiter', options).start();
		return this;
	},
	
	release: function(){
		var opt = Array.link(arguments, {options: Object.type, callback: Function.type});
		this.get('waiter', opt.options).stop(opt.callback);
		return this;
	}

});

/*
Script: Collapsable.js
	Enables a dom element to, when clicked, hide or show (it toggles) another dom element. Kind of an Accordion for one item.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var Collapsable = new Class({
	Extends: Fx.Reveal,
	initialize: function(clicker, section, options) {
		this.clicker = $(clicker);
		this.section = $(section);
		this.parent(this.section, options);
		this.addEvents();
	},
	addEvents: function(){
		this.clicker.addEvent('click', this.toggle.bind(this));
	}
});

/*
Script: HoverGroup.js
	Manages mousing in and out of multiple objects (think drop-down menus).

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var HoverGroup = new Class({
	Implements: [Options, Events],
	Binds: ['enter', 'leave', 'remain'],
	options: {
		//onEnter: $empty,
		//onLeave: $empty,
		elements: [],
		delay: 300,
		start: ['mouseenter'],
		remain: [],
		end: ['mouseleave']
	},
	initialize: function(options) {
		this.setOptions(options);
		this.attachTo(this.options.elements);
		this.addEvents({
			leave: function(){
				this.active = false;
			},
			enter: function(){
				this.active = true;
			}
		});
	},
	elements: [],
	attachTo: function(elements, detach){
		var actions = {};
		elements = $G(elements);
		this.options.start.each(function(start) {
			actions[start] = this.enter;
		}, this);
		this.options.end.each(function(end) {
			actions[end] = this.leave;
		}, this);
		this.options.remain.each(function(remain){
			actions[remain] = this.remain;
		}, this);
		if (detach) {
			elements.each(function(el) {
				el.removeEvents(actions);
				this.elements.erase(el);
			});
		} else {
			elements.addEvents(actions);
			this.elements.combine(elements);
		}
		return this;
	},
	detachFrom: function(elements){
		this.attachTo(elements, true);
	},
	enter: function(){
		this.isMoused = true;
		this.assert();
	},
	leave: function(){
		this.isMoused = false;
		this.assert();
	},
	remain: function(){
		if (this.active) this.enter();
	},
	assert: function(){
		$clear(this.assertion);
		this.assertion = (function(){
			if (!this.isMoused && this.active) this.fireEvent('leave');
			else if (this.isMoused && !this.active) this.fireEvent('enter');
		}).delay(this.options.delay, this);
	}
});

/*
Script: HtmlTable.js

Builds table elements with methods to add rows quickly.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var HtmlTable = new Class({
	Implements: [Options],
	options: {
		properties: {
			cellpadding: 0,
			cellspacing: 0,
			border: 0
		},
		rows: []
	},
	initialize: function(options) {
		this.setOptions(options);
		this.table = new Element('table').setProperties(this.options.properties);
		this.table.store('HtmlTable', this);
		this.tbody = new Element('tbody').inject(this.table);
		this.options.rows.each(this.push.bind(this));
		["adopt", "inject", "wraps", "grab", "replaces", "empty", "dispose"].each(function(method){
				this[method] = this.table[method].bind(this.table);
		}, this);
	},
	toElement: function(){
		return this.table;
	},
	push: function(row) {
		var tr = new Element('tr').inject(this.tbody);
		var tds = row.map(function (tdata) {
			tdata = tdata || '';
			var td = new Element('td').inject(tr);
			if(tdata.properties) td.setProperties(tdata.properties);
			function setContent(content){
				if($(content)) td.adopt($(content));
				else td.set('html', content);
			};
			if($defined(tdata.content)) setContent(tdata.content);
			else setContent(tdata);
			return td;
		}, this);
		return {tr: tr, tds: tds};
	}
});

/*
Script: MenuSlider.js
	Slides open a menu when the user mouses over a dom element. leaves it open while the mouse is over that element or the menu.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var MenuSlider = new Class({
	Implements: [Options, Events],
	Binds: ['slideIn', 'slideOut'],
	options: {
	/*	onIn: $empty,
		onInStart: $empty,
		onOut: $empty,
		hoverGroupOptions: {}, */
		fxOptions: {
			duration: 400,
			transition: 'expo:out',
			link: 'cancel'
		},
		useIframeShim: true
	},
	initialize: function(menu, subMenu, options) {
		this.menu = $(menu);
		this.subMenu = $(subMenu);
		this.setOptions(options);
		this.makeSlider();
		this.hoverGroup = new HoverGroup($merge(this.options.hoverGroupOptions, {
			elements: [this.menu, this.subMenu],
			onEnter: this.slideIn,
			onLeave: this.slideOut
		}));
	},
	makeSlider: function(){
		this.slider = new Fx.Slide(this.subMenu, this.options.fxOptions).hide();
		if (this.options.useIframeShim && window.IframeShim) this.shim = new IframeShim(this.subMenu);
	},
	slideIn: function(){
		this.fireEvent('onInStart');
		this.slider.slideIn().chain(function(){
			if (this.shim) this.shim.show();
			this.fireEvent('onIn');
		}.bind(this));
		return this;
	},
	slideOut: function(){
		this.hide();
		this.fireEvent('onOut');
		if (this.shim) this.shim.hide();
		return this;
	},
	hide: function(){
		$clear(this.hoverGroup.assertion);
		this.hoverGroup.active = false;
		this.slider.cancel();
		this.slider.hide();
		if (this.shim) this.shim.hide();
		return this;
	}
});

/*
Script: MooScroller.js

Recreates the standard scrollbar behavior for elements with overflow but using DOM elements so that the scroll bar elements are completely styleable by css.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var MooScroller = new Class({
	Implements: [Options, Events],
	options: {
		maxThumbSize: 10,
		mode: 'vertical',
		width: 0, //required only for mode: horizontal
		scrollSteps: 10,
		wheel: true,
		scrollLinks: {
			forward: 'scrollForward',
			back: 'scrollBack'
		},
		hideWhenNoOverflow: true
//		onScroll: $empty,
//		onPage: $empty
	},

	initialize: function(content, knob, options){
		this.setOptions(options);
		this.horz = (this.options.mode == "horizontal");

		this.content = $(content).setStyle('overflow', 'hidden');
		this.knob = $(knob);
		this.track = this.knob.getParent();
		this.setPositions();
		
		if(this.horz && this.options.width) {
			this.wrapper = new Element('div');
			this.content.getChildren().each(function(child){
				this.wrapper.adopt(child);
			}, this);
			this.wrapper.inject(this.content).setStyle('width', this.options.width);
		}
		

		this.bound = {
			'start': this.start.bind(this),
			'end': this.end.bind(this),
			'drag': this.drag.bind(this),
			'wheel': this.wheel.bind(this),
			'page': this.page.bind(this)
		};

		this.position = {};
		this.mouse = {};
		this.update();
		this.attach();
		
		var clearScroll = function (){
			$clear(this.scrolling);
		}.bind(this);
		['forward','back'].each(function(direction) {
			var lnk = $(this.options.scrollLinks[direction]);
			if(lnk) {
				lnk.addEvents({
					mousedown: function() {
						this.scrolling = this[direction].periodical(50, this);
					}.bind(this),
					mouseup: clearScroll.bind(this),
					click: clearScroll.bind(this)
				});
			}
		}, this);
		this.knob.addEvent('click', clearScroll.bind(this));
		window.addEvent('domready', function(){
			try {
				$(document.body).addEvent('mouseup', clearScroll.bind(this));
			}catch(e){}
		}.bind(this));
	},
	setPositions: function(){
		[this.track, this.knob].each(function(el){
			if (el.getStyle('position') == 'static') el.setStyle('position','relative');
		});

	},
	toElement: function(){
		return this.content;
	},
	update: function(){
		var plain = this.horz?'Width':'Height';
		this.contentSize = this.content['offset'+plain];
		this.contentScrollSize = this.content['scroll'+plain];
		this.trackSize = this.track['offset'+plain];

		this.contentRatio = this.contentSize / this.contentScrollSize;

		this.knobSize = (this.trackSize * this.contentRatio).limit(this.options.maxThumbSize, this.trackSize);

		if (this.options.hideWhenNoOverflow) {
			this.hidden = this.knobSize == this.trackSize;
			this.track.setStyle('opacity', this.hidden?0:1);
		}
		
		this.scrollRatio = this.contentScrollSize / this.trackSize;
		this.knob.setStyle(plain.toLowerCase(), this.knobSize);

		this.updateThumbFromContentScroll();
		this.updateContentFromThumbPosition();
	},

	updateContentFromThumbPosition: function(){
		this.content[this.horz?'scrollLeft':'scrollTop'] = this.position.now * this.scrollRatio;
	},

	updateThumbFromContentScroll: function(){
		this.position.now = (this.content[this.horz?'scrollLeft':'scrollTop'] / this.scrollRatio).limit(0, (this.trackSize - this.knobSize));
		this.knob.setStyle(this.horz?'left':'top', this.position.now);
	},

	attach: function(){
		this.knob.addEvent('mousedown', this.bound.start);
		if (this.options.scrollSteps) this.content.addEvent('mousewheel', this.bound.wheel);
		this.track.addEvent('mouseup', this.bound.page);
	},

	wheel: function(event){
		if (this.hidden) return;
		this.scroll(-(event.wheel * this.options.scrollSteps));
		this.updateThumbFromContentScroll();
		event.stop();
	},

	scroll: function(steps){
		steps = steps||this.options.scrollSteps;
		this.content[this.horz?'scrollLeft':'scrollTop'] += steps;
		this.updateThumbFromContentScroll();
		this.fireEvent('onScroll', steps);
	},
	forward: function(steps){
		this.scroll(steps);
	},
	back: function(steps){
		steps = steps||this.options.scrollSteps;
		this.scroll(-steps);
	},

	page: function(event){
		var axis = this.horz?'x':'y';
		var forward = (event.page[axis] > this.knob.getPosition()[axis]);
		this.scroll((forward?1:-1)*this.content['offset'+(this.horz?'Width':'Height')]);
		this.updateThumbFromContentScroll();
		this.fireEvent('onPage', forward);
		event.stop();
	},

	
	start: function(event){
		var axis = this.horz?'x':'y';
		this.mouse.start = event.page[axis];
		this.position.start = this.knob.getStyle(this.horz?'left':'top').toInt();
		document.addEvent('mousemove', this.bound.drag);
		document.addEvent('mouseup', this.bound.end);
		this.knob.addEvent('mouseup', this.bound.end);
		event.stop();
	},

	end: function(event){
		document.removeEvent('mousemove', this.bound.drag);
		document.removeEvent('mouseup', this.bound.end);
		this.knob.removeEvent('mouseup', this.bound.end);
		event.stop();
	},

	drag: function(event){
		var axis = this.horz?'x':'y';
		this.mouse.now = event.page[axis];
		this.position.now = (this.position.start + (this.mouse.now - this.mouse.start)).limit(0, (this.trackSize - this.knobSize));
		this.updateContentFromThumbPosition();
		this.updateThumbFromContentScroll();
		event.stop();
	}

});


/*
Script: MultipleOpenAccordion.js

Creates a Mootools Fx.Accordion that allows the user to open more than one element.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var MultipleOpenAccordion = new Class({
	Implements: [Options, Events, Chain],
	options: {
		togglers: [],
		elements: [],
		openAll: true,
		firstElementsOpen: [0],
		fixedHeight: false,
		fixedWidth: false,
		height: true,
		opacity: true,
		width: false
//	onActive: $empty,
//	onBackground: $empty
	},
	togglers: [],
	elements: [],
	initialize: function(container, options){
		this.setOptions(options);
		this.container = $(container);
		elements = $$(options.elements);
		$$(options.togglers).each(function(toggler, idx){
			this.addSection(toggler, elements[idx], idx);
		}, this);
		if (this.togglers.length) {
			if (this.options.openAll) this.showAll();
			else this.toggleSections(this.options.firstElementsOpen, false, true);
		}
		this.openSections = this.showSections.bind(this);
		this.closeSections = this.hideSections.bind(this);
	},
	addSection: function(toggler, element, pos){
		toggler = $(toggler);
		element = $(element);
		var test = this.togglers.contains(toggler);
		var len = this.togglers.length;
		this.togglers.include(toggler);
		this.elements.include(element);
		if (len && (!test || pos)){
			pos = $pick(pos - 1, len - 1);
			toggler.inject(this.elements[pos], 'after');
			element.inject(toggler, 'after');
		} else if (this.container && !test){
			toggler.inject(this.container);
			element.inject(this.container);
		}
		var idx = this.togglers.indexOf(toggler);
		toggler.addEvent('click', this.toggleSection.bind(this, idx));
		var mode;
		if (this.options.height && this.options.width) mode = "both";
		else mode = (this.options.height)?"vertical":"horizontal";
		element.store('reveal', new Fx.Reveal(element, {
			transitionOpacity: this.options.opacity,
			mode: mode,
			heightOverride: this.options.fixedHeight,
			widthOverride: this.options.fixedWidth
		}));
		return this;
	},
	onComplete: function(idx, callChain){
		this.fireEvent(this.elements[idx].isVisible()?'onActive':'onBackground', [this.togglers[idx], this.elements[idx]]);
		this.callChain();
		return this;
	},
	showSection: function(idx, useFx){
		this.toggleSection(idx, useFx, true);
	},
	hideSection: function(idx, useFx){
		this.toggleSection(idx, useFx, false);
	},
	toggleSection: function(idx, useFx, show, callChain){
		var method = show?'reveal':$defined(show)?'dissolve':'toggle';
		callChain = $pick(callChain, true);
		var el = this.elements[idx];
		if($pick(useFx, true)) {
			el.retrieve('reveal')[method]().chain(
				this.onComplete.bind(this, [idx, callChain])
			);
		} else {
				if (method == "toggle") el.togglek();
				else el[method == "reveal"?'show':'hide']();
				this.onComplete(idx, callChain);
		}
		return this;
	},
	toggleAll: function(useFx, show){
		var method = show?'reveal':$chk(show)?'disolve':'toggle';
		var last = this.elements.getLast();
		this.elements.each(function(el, idx){
			this.toggleSection(idx, useFx, show, el == last);
		}, this);
		return this;
	},
	toggleSections: function(sections, useFx, show) {
		last = sections.getLast();
		this.elements.each(function(el,idx){
			this.toggleSection(idx, useFx, sections.contains(idx)?show:!show, idx == last);
		}, this);
		return this;
	},
	showSections: function(sections, useFx){
		sections.each(function(i){
			this.showSection(i, useFx);
		}, this);
	},
	hideSections: function(sections, useFx){
		sections.each(function(i){
			this.hideSection(i, useFx);
		}, this);
	},
	showAll: function(useFx){
		return this.toggleAll(useFx, true);
	},
	hideAll: function(useFx){
		return this.toggleAll(useFx, false);
	}
});


/*
Script: SimpleCarousel.js

Builds a carousel object that manages the basic functions of a generic carousel (a carousel	here being a collection of "slides" that play from one to the next, with a collection of "buttons" that reference each slide).

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var SimpleCarousel = new Class({
	Implements: [Options, Events],
	options: {
//		onRotate: $empty,
//		onStop: $empty,
//		onAutoPlay: $empty,
//		onShowSlide: $empty,
		slideInterval: 4000,
		transitionDuration: 700,
		startIndex: 0,
		buttonOnClass: "selected",
		buttonOffClass: "off",
		rotateAction: "none",
		rotateActionDuration: 100,
		autoplay: true
	},
	initialize: function(container, slides, buttons, options){
		this.container = $(container);
		var instance = this.container.retrieve('SimpleCarouselInstance');
		if (instance) return instance;
		this.container.store('SimpleCarouselInstance', this);
		this.setOptions(options);
		this.container.addClass('hasCarousel');
		this.slides = $$(slides);
		this.buttons = $$(buttons);
		this.createFx();
		this.showSlide(this.options.startIndex);
		if(this.options.autoplay) this.autoplay();
		if(this.options.rotateAction != 'none') this.setupAction(this.options.rotateAction);
		return this;
	},
	toElement: function(){
		return this.container;
	},
	setupAction: function(action) {
		this.buttons.each(function(el, idx){
			$(el).addEvent(action, function() {
				this.slideFx.setOptions(this.slideFx.options, {duration: this.options.rotateActionDuration});
				if(this.currentSlide != idx) this.showSlide(idx);
				this.stop();
			}.bind(this));
		}, this);
	},
	createFx: function(){
		if (!this.slideFx) this.slideFx = new Fx.Elements(this.slides, {duration: this.options.transitionDuration});
		this.slides.each(function(slide){
			slide.setStyle('opacity',0);
		});
	},
	showSlide: function(slideIndex){
		var action = {};
		this.slides.each(function(slide, index){
			if(index == slideIndex && index != this.currentSlide){ //show
				$(this.buttons[index]).swapClass(this.options.buttonOffClass, this.options.buttonOnClass);
				action[index.toString()] = {
					opacity: 1
				};
			} else {
				$(this.buttons[index]).swapClass(this.options.buttonOnClass, this.options.buttonOffClass);
				action[index.toString()] = {
					opacity:0
				};
			}
		}, this);
		this.fireEvent('onShowSlide', slideIndex);
		this.currentSlide = slideIndex;
		this.slideFx.start(action);
		return this;
	},
	autoplay: function(){
		this.slideshowInt = this.rotate.periodical(this.options.slideInterval, this);
		this.fireEvent('onAutoPlay');
		return this;
	},
	stop: function(){
		$clear(this.slideshowInt);
		this.fireEvent('onStop');
		return this;
	},
	rotate: function(){
		current = this.currentSlide;
		next = (current+1 >= this.slides.length) ? 0 : current+1;
		this.showSlide(next);
		this.fireEvent('onRotate', next);
		return this;
	}
});

/*
Script: SimpleSlideShow.js

Makes a very, very simple slideshow gallery with a collection of dom elements and previous and next buttons.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var SimpleSlideShow = new Class({
	Implements: [Events, Options, Chain],
	options: {
		startIndex: 0,
		slides: [],
		currentSlideClass: 'currentSlide',
		currentIndexContainer: false,
		maxContainer: false,
		nextLink: false,
		prevLink: false,
		wrap: true,
		disabledLinkClass: 'disabled',
//	onNext: $empty,
//	onPrev: $empty,
//	onSlideClick: $empty,
//	onSlideDisplay: $empty,
		crossFadeOptions: {}
	},
	initialize: function(options){
		this.setOptions(options);
		var slides = this.options.slides;
		this.makeSlides(slides);
		this.setCounters();
		this.setUpNav();
		this.now = this.options.startIndex;
		if (this.slides.length > 0) this.show(this.now);
	},
	slides:[],
	setCounters: function(){
		if ($(this.options.currentIndexContainer))$(this.options.currentIndexContainer).set('html', this.now+1);
		if ($(this.options.maxContainer))$(this.options.maxContainer).set('html', this.slides.length);
	},
	makeSlides: function(slides){
		//hide them all
		slides.each(function(slide, index){
			if (index != this.now) slide.setStyle('display', 'none');
			else slide.setStyle('display', 'block');
			this.makeSlide(slide);
		}, this);
	},
	makeSlide: function(slide){
		slide.addEvent('click', function(){ this.fireEvent('onSlideClick'); }.bind(this));
		this.slides.include(slide);
	},
	setUpNav: function(){	
		if ($(this.options.nextLink)) {
			$(this.options.nextLink).addEvent('click', function(){
				this.forward();
			}.bind(this));
		}
		if ($(this.options.prevLink)) {
			$(this.options.prevLink).addEvent('click', function(){
				this.back();
			}.bind(this));
		}
	},
	disableLinks: function(now){
		if (this.options.wrap) return;
		now = $pick(now, this.now);
		var prev = $(this.options.prevLink);
		var next = $(this.options.nextLink);
		var dlc = this.options.disabledLinkClass;
		if (now > 0) {
			if (prev) prev.removeClass(dlc);
			if (now === this.slides.length-1 && next) next.addClass(dlc);
			else if (next) next.removeClass(dlc)
		}	else { //now is zero
			if (this.slides.length > 0 && next) next.removeClass(dlc);
			if (prev) prev.addClass(dlc);
		}
	},
	forward: function(){
		if ($type(this.now) && this.now < this.slides.length-1) this.show(this.now+1);
		else if ($type(this.now) && this.options.wrap) this.show(0);
		else if (!$type(this.now)) this.show(this.options.startIndex);
		this.fireEvent('next');
		return this;
	},
	back: function(){
		if (this.now > 0) {
			this.show(this.now-1);
			this.fireEvent('onPrev');
		} else if (this.options.wrap && this.slides.length > 1) {
			this.show(this.slides.length-1);
			this.fireEvent('prev');
		}
		return this;
	},
	show: function(index){
		if (this.showing) return this.chain(this.show.bind(this, index));
		var now = this.now;
		var s = this.slides[index]; //saving bytes
		function fadeIn(s, resetOpacity){
			s.setStyle('display','block');
			if (s.fxOpacityOk()) {
				if (resetOpacity) s.setStyle('opacity', 0);
				s.set('tween', this.options.crossFadeOptions).get('tween').start('opacity', 1).chain(function(){
					this.showing = false;
					this.disableLinks();
					this.callChain();
					this.fireEvent('onSlideDisplay', index);
				}.bind(this));
			}
		};
		if (s) {
			if ($type(this.now) && this.now != index){
				if (s.fxOpacityOk()) {
					var fx = this.slides[this.now].get('tween');
					fx.setOptions(this.options.crossFadeOptions);
					this.showing = true;
					fx.start('opacity', 0).chain(function(){
						this.slides[now].setStyle('display','none');
						s.addClass(this.options.currentSlideClass);
						fadeIn.run([s, true], this);
						this.fireEvent('onSlideDisplay', index);
					}.bind(this));
				} else {
					this.slides[this.now].setStyle('display','none');
					fadeIn.run(s, this);
				}
			} else fadeIn.run(s, this);
			this.now = index;
			this.setCounters();
		}
	},
	slideClick: function(){
		this.fireEvent('onSlideClick', [this.slides[this.now], this.now]);
	}
});

SimpleSlideShow.Carousel = new Class({
	Extends: SimpleSlideShow,
	Implements: [ToElement],
	Binds: ['makeSlide'],
	options: {
		sliderWidth: 999999
	},
	initialize: function(container, options){
		this.setOptions(options);
		this.container = $(container);
		this.element = new Element('div').wraps(this.container).setStyles({
			width: this.container.getSize().x,
			overflow: 'hidden'
		});
		this.container.setStyles({
			width: this.options.sliderWidth,
			position: 'relative'
		});
		this.parent(options);
	},
	makeSlides: function(slides) {
		this.slides = [];
		slides.each(this.makeSlide);
	},
	makeSlide: function(slide) {
		if (slide.retrieve('slideSetup')) return;
		slide.store('slideSetup', true);
		slide.show();
		var s = new Element('div', {
			styles: {
				'float': 'left',
				width: $(this).getSize().x
			}
		}).wraps(slide);
		this.parent(s);
		this.slides.erase(slide);
		this.setCounters();
		s.show();
		s.inject(this.container);
	},
	show: function(index) {
		if (!this.container) return;
		this.fx = this.fx || new Fx.Tween(this.container, {
			property: 'left'
		});
		if (this.showing) return this.chain(this.show.bind(this, index));
		var now = this.now;
		var s = this.slides[index]; //saving bytes
		if (s) {
			if (this.now != index) {
				this.fx.start(-s.getPosition(this.container).x).chain(function(){
					s.addClass(this.options.currentSlideClass);
					this.showing = false;
					this.disableLinks();
					this.callChain();
					this.fireEvent('onSlideDisplay', index);
				}.bind(this));
			}
			this.now = index;
			this.setCounters();
		}
	}
});

var SimpleImageSlideShow;
(function(){
	var extender = function(extend, passContainer) {
		return {
			Extends: extend,
			Implements: ToElement,
			options: {
				imgUrls: [],
				imgClass: 'screenshot',
				container: false
			},
			initialize: function(){
				var args = Array.link(arguments, {options: Object.type, container: $defined});
				this.container = $(args.container) || (args.options?$(args.options.container):false); //legacy
				if (passContainer) this.parent(this.container, args.options);
				else this.parent(args.options);
				this.options.imgUrls.each(function(url){
					this.addImg(url);
				}, this);
				this.show(this.options.startIndex);
			},
			addImg: function(url){
				if (this.container) {
					var img = new Element('img', {
						'src': url,
						'id': this.options.imgClass+this.slides.length
					}).addClass(this.options.imgClass).setStyle(
						'display', 'none').inject(this.container).addEvent(
						'click', this.slideClick.bind(this));
					this.slides.push(img);
					this.makeSlide(img);
					this.setCounters();
				}
				return this;
			}
		};
	};
	SimpleImageSlideShow = new Class(extender(SimpleSlideShow));
	SimpleImageSlideShow.Carousel = new Class(extender(SimpleSlideShow.Carousel, true));
})();


/*
Script: TabSwapper.js

Handles the scripting for a common UI layout; the tabbed box.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var TabSwapper = new Class({
	Implements: [Options, Events],
	options: {
		selectedClass: 'tabSelected',
		mouseoverClass: 'tabOver',
		deselectedClass: '',
		rearrangeDOM: true,
		initPanel: 0, 
		smooth: false, 
		smoothSize: false,
		maxSize: null,
		effectOptions: {
			duration: 500
		},
		cookieName: null, 
		cookieDays: 999
//	onActive: $empty,
//	onActiveAfterFx: $empty,
//	onBackground: $empty
	},
	tabs: [],
	sections: [],
	clickers: [],
	sectionFx: [],
	initialize: function(options){
		this.setOptions(options);
		var prev = this.setup();
		if (prev) return prev;
		if(this.options.cookieName && this.recall()) this.show(this.recall().toInt());
		else this.show(this.options.initPanel);
	},
	setup: function(){
		var opt = this.options;
		sections = $$(opt.sections);
		tabs = $$(opt.tabs);
		if (tabs[0] && tabs[0].retrieve('tabSwapper')) return tabs[0].retrieve('tabSwapper');
		clickers = $$(opt.clickers);
		tabs.each(function(tab, index){
			this.addTab(tab, sections[index], clickers[index], index);
		}, this);
	},
	addTab: function(tab, section, clicker, index){
		tab = $(tab); clicker = $(clicker); section = $(section);
		//if the tab is already in the interface, just move it
		if(this.tabs.indexOf(tab) >= 0 && tab.retrieve('tabbered') 
			 && this.tabs.indexOf(tab) != index && this.options.rearrangeDOM) {
			this.moveTab(this.tabs.indexOf(tab), index);
			return this;
		}
		//if the index isn't specified, put the tab at the end
		if(!$defined(index)) index = this.tabs.length;
		//if this isn't the first item, and there's a tab
		//already in the interface at the index 1 less than this
		//insert this after that one
		if(index > 0 && this.tabs[index-1] && this.options.rearrangeDOM) {
			tab.inject(this.tabs[index-1], 'after');
			section.inject(this.tabs[index-1].retrieve('section'), 'after');
		}
		this.tabs.splice(index, 0, tab);
		clicker = clicker || tab;

		tab.addEvents({
			mouseout: function(){
				tab.removeClass(this.options.mouseoverClass);
			}.bind(this),
			mouseover: function(){
				tab.addClass(this.options.mouseoverClass);
			}.bind(this)
		});

		clicker.addEvent('click', function(e){
			e.preventDefault();
			this.show(index);
		}.bind(this));

		tab.store('tabbered', true);
		tab.store('section', section);
		tab.store('clicker', clicker);
		this.hideSection(index);
		return this;
	},
	removeTab: function(index){
		var now = this.tabs[this.now];
		if(this.now == index){
			if(index > 0) this.show(index - 1);
			else if (index < this.tabs.length) this.show(index + 1);
		}
		this.now = this.tabs.indexOf(now);
		return this;
	},
	moveTab: function(from, to){
		var tab = this.tabs[from];
		var clicker = tab.retrieve('clicker');
		var section = tab.retrieve('section');
		
		var toTab = this.tabs[to];
		var toClicker = toTab.retrieve('clicker');
		var toSection = toTab.retrieve('section');
		
		this.tabs.erase(tab).splice(to, 0, tab);

		tab.inject(toTab, 'before');
		clicker.inject(toClicker, 'before');
		section.inject(toSection, 'before');
		return this;
	},
	show: function(i){
		if (!$chk(this.now)) {
			this.tabs.each(function(tab, idx){
				if (i != idx) 
					this.hideSection(idx)
			}, this);
		}
		this.showSection(i).save(i);
		return this;
	},
	save: function(index){
		if(this.options.cookieName) 
			Cookie.write(this.options.cookieName, index, {duration:this.options.cookieDays});
		return this;
	},
	recall: function(){
		return (this.options.cookieName)?$pick(Cookie.read(this.options.cookieName), false): false;
	},
	hideSection: function(idx) {
		var tab = this.tabs[idx];
		if (!tab) return this;
		var sect = tab.retrieve('section');
		if (!sect) return this;
		if (sect.getStyle('display') != 'none') {
			this.lastHeight = sect.getSize().y;
			sect.setStyle('display', 'none');
			tab.swapClass(this.options.selectedClass, this.options.deselectedClass);
			this.fireEvent('onBackground', [idx, sect, tab]);
		}
		return this;
	},
	showSection: function(idx) {
		var tab = this.tabs[idx];
		if (!tab) return this;
		var sect = tab.retrieve('section');
		if (!sect) return this;
		var smoothOk = this.options.smooth && (!Browser.Engine.trident4 
										|| (Browser.Engine.trident4 && sect.fxOpacityOk()));
		if(this.now != idx) {
			if (!tab.retrieve('tabFx')) 
				tab.store('tabFx', new Fx.Morph(sect, this.options.effectOptions));
			var start = {
				display:'block',
				overflow: 'hidden'
			};
			if (smoothOk) start.opacity = 0;
			var effect = false;
			if(smoothOk) {
				effect = {opacity: 1};
			} else if (sect.getStyle('opacity').toInt() < 1) {
				sect.setStyle('opacity', 1);
				if (!this.options.smoothSize) 
					this.fireEvent('onActiveAfterFx', [idx, sect, tab]);
			}
			if (this.options.smoothSize) {
				var size = sect.getDimensions().height;
				if ($chk(this.options.maxSize) && this.options.maxSize < size) 
					size = this.options.maxSize;
				if (!effect) effect = {};
				effect.height = size;
			}
			if ($chk(this.now)) this.hideSection(this.now);
			if (this.options.smoothSize && this.lastHeight) start.height = this.lastHeight;
			sect.setStyles(start);
			if (effect) {
				tab.retrieve('tabFx').start(effect).chain(function(){
					this.fireEvent('onActiveAfterFx', [idx, sect, tab]);
					sect.setStyle("height", "auto");
				}.bind(this));
			}
			this.now = idx;
			this.fireEvent('onActive', [idx, sect, tab]);
		}
		tab.swapClass(this.options.deselectedClass, this.options.selectedClass);
		return this;
	}
});


/*
Script: Clipboard.js
	Provides access to the OS clipboard so that data can be copied to it (using a flash plugin).

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var Clipboard = {
	swfLocation: 'http://www.cnet.com/html/rb/assets/global/clipboard/_clipboard.swf',
	copyFromElement: function(element) {
		element = $(element);
		if(!element) return null;
		if (Browser.Engine.trident) {
			try {
				window.addEvent('domready', function() {
					var range = element.createTextRange();
					if(range) range.execCommand('Copy');
				});
			}catch(e){
				dbug.log('cannot copy to clipboard: %s', o)
			}
		} else {
			var text = (element.getSelectedText)?element.getSelectedText():element.get('value');
			if (text) Clipboard.copy(text);
		}
		return element;
	},
	copy: function(text) {
		if(Browser.Engine.trident){
			window.addEvent('domready', function() {
				var cb = new Element('textarea', {styles: {display: 'none'}}).inject(document.body);
				cb.set('value', text).select();
				Clipboard.copyFromElement(cb);
				cb.dispose();
			});
		} else {
			var swf = ($('flashcopier'))?$('flashcopier'):new Element('div', {
				id: 'flashcopier'
			}).inject(document.body);
			swf.empty();
			swf.set('html', '<embed src="'+this.swfLocation+'" FlashVars="clipboard='+escape(text)+'" width="0" height="0" type="application/x-shockwave-flash"></embed>');
		}
	}
};


/*
Script: Confirmer.js
	Fades a message in and out for the user to tell them that some event (like an ajax save) has occurred.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var Confirmer = new Class({
	Implements: [Options, Events],
	options: {
		reposition: true, //for elements already in the DOM
		//if position = false, just fade
		positionOptions: {
			relativeTo: false,
			position: 'upperRight', //see <Element.setPosition>
			offset: {x:-225,y:0},
			zIndex: 9999
		},
		msg: 'your changes have been saved', //string or dom element
		msgContainerSelector: '.body',
		delay: 250,
		pause: 1000,
		effectOptions:{
			duration: 500
		},
		prompterStyle:{
			padding: '2px 6px',
			border: '1px solid #9f0000',
			backgroundColor: '#f9d0d0',
			fontWeight: 'bold',
			color: '#000',
			width: 210
		}
//	onComplete: $empty
	},
	initialize: function(options){
			this.setOptions(options);
			this.options.positionOptions.relativeTo = $(this.options.positionOptions.relativeTo) || document.body;
			this.prompter = ($(this.options.msg))?$(this.options.msg):this.makePrompter(this.options.msg);
			if(this.options.reposition){
				this.prompter.setStyles({
					position: 'absolute',
					display: 'none',
					zIndex: this.options.positionOptions.zIndex
				});
				if(this.prompter.fxOpacityOk()) this.prompter.setStyle('opacity',0);
			} else if(this.prompter.fxOpacityOk()) this.prompter.setStyle('opacity',0);
			else this.prompter.setStyle('visibility','hidden');
			if (!this.prompter.getParent()){
				window.addEvent('domready', function(){
					this.prompter.inject(document.body);
				}.bind(this));
			}
		try {
			this.msgHolder = this.prompter.getElement(this.options.msgContainerSelector);
			if(!this.msgHolder) this.msgHolder = this.prompter;
		} catch(e){dbug.log(e)}
	},
	makePrompter: function(msg){
		return new Element('div').setStyles(this.options.prompterStyle).appendText(msg);
	},
	prompt: function(options){
		if(!this.paused)this.stop();
		var msg = (options)?options.msg:false;
		options = $merge(this.options, {saveAsDefault: false}, options||{});
		if ($(options.msg) && msg) this.msgHolder.empty().adopt(options.msg);
		else if (!$(options.msg) && options.msg) this.msgHolder.empty().appendText(options.msg);
		if(!this.paused) {
			if(options.reposition) this.position(options.positionOptions);
			(function(){
				this.timer = this.fade(options.pause);
			}).delay(options.delay, this);
		}
		if(options.saveAsDefault) this.setOptions(options);
		return this;
	},
	fade: function(pause){
		this.paused = true;
		pause = $pick(pause, this.options.pause);
		if(!this.fx && this.prompter.fxOpacityOk())
			this.fx = new Fx.Tween(this.prompter, $merge({property: 'opacity'}, this.options.effectOptions));
		if(this.options.reposition) this.prompter.setStyle('display','block');
		if(this.prompter.fxOpacityOk()){
			this.prompter.setStyle('visibility','visible');
			this.fx.start(0,1).chain(function(){
				this.timer = (function(){
					this.fx.start(0).chain(function(){
						if(this.options.reposition) this.prompter.hide();
						this.paused = false;
					}.bind(this));
				}).delay(pause, this);
			}.bind(this));
		} else {
			this.prompter.setStyle('visibility','visible');
			this.timer = (function(){
				this.prompter.setStyle('visibility','hidden');
				this.fireEvent('onComplete');
				this.paused = false;
			}).delay(pause+this.options.effectOptions.duration, this);
		}
		return this;
	},
	stop: function(){	
		this.paused = false;
		$clear(this.timer);
		if(this.fx) this.fx.set(0);
		if(this.options.reposition) this.prompter.hide();
		return this;
	},
	position: function(positionOptions){
		this.prompter.setPosition($merge(this.options.positionOptions, positionOptions));
		return this;
	}
});


/*
Script: DatePicker.js
	Allows the user to enter a date in many popuplar date formats or choose from a calendar.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var DatePicker;
(function(){
	var DPglobal = function() {
		if (DatePicker.pickers) return;
		DatePicker.pickers = [];
		DatePicker.hideAll = function(){
			DatePicker.pickers.each(function(picker){
				picker.hide();
			});
		};
	};
 	DatePicker = new Class({
		Implements: [Options, Events, StyleWriter],
		options: {
			format: "%x",
			defaultCss: 'div.calendarHolder {height:177px;position: absolute;top: -21px !important;top: -27px;left: -3px;width: 100%;}'+
				'div.calendarHolder table.cal {margin-right: 15px !important;margin-right: 8px;width: 205px;}'+
				'div.calendarHolder td {text-align:center;}'+
				'div.calendarHolder tr.dayRow td {padding: 2px;width: 22px;cursor: pointer;}'+
				'div.calendarHolder table.datePicker * {font-size:11px;line-height:16px;}'+
				'div.calendarHolder table.datePicker {margin: 0;padding:0 5px;float: left;}'+
				'div.calendarHolder table.datePicker table.cal td {cursor:pointer;}'+
				'div.calendarHolder tr.dateNav {font-weight: bold;height:22px;margin-top:8px;}'+
				'div.calendarHolder tr.dayNames {height: 23px;}'+
				'div.calendarHolder tr.dayNames td {color:#666;font-weight:700;border-bottom:1px solid #ddd;}'+
				'div.calendarHolder table.datePicker tr.dayRow td:hover {background:#ccc;}'+
				'div.calendarHolder table.datePicker tr.dayRow td {margin: 1px;}'+
				'div.calendarHolder td.today {color:#bb0904;}'+
				'div.calendarHolder td.otherMonthDate {border:1px solid #fff;color:#ccc;background:#f3f3f3 !important;margin: 0px !important;}'+
				'div.calendarHolder td.selectedDate {border: 1px solid #20397b;background:#dcddef;margin: 0px !important;}'+
				'div.calendarHolder a.leftScroll, div.calendarHolder a.rightScroll {cursor: pointer; color: #000}'+
				'div.datePickerSW div.body {height: 160px !important;height: 149px;}'+
				'div.datePickerSW .clearfix:after {content: ".";display: block;height: 0;clear: both;visibility: hidden;}'+
				'div.datePickerSW .clearfix {display: inline-table;}'+
				'* html div.datePickerSW .clearfix {height: 1%;}'+
				'div.datePickerSW .clearfix {display: block;}',
			calendarId: false,
			stickyWinOptions: {
				draggable: true,
				dragOptions: {},
				position: "bottomLeft",
				offset: {x:10, y:10},
				fadeDuration: 400
			},
			stickyWinUiOptions: {},
			updateOnBlur: true,
			additionalShowLinks: [],
			showOnInputFocus: true,
			useDefaultCss: true,
			hideCalendarOnPick: true,
			weekStartOffset: 0,
			showMoreThanOne: true
/*		onPick: $empty,
			onShow: $empty,
			onHide: $empty */
		},
	
		initialize: function(input, options){
			DPglobal(); //make sure controller is setup
			if ($(input)) this.inputs = $H({start: $(input)});
	    	this.today = new Date();
			var StickyWinToUse = (typeof StickyWin.Fx == "undefined")?StickyWin:StickyWin.Fx;
			this.setOptions({
				stickyWinToUse: StickyWinToUse
			}, options);
			if(this.options.useDefaultCss) this.createStyle(this.options.defaultCss, 'datePickerStyle');
			if (!this.inputs) return;
			this.whens = this.whens || ['start'];
			if(!this.calendarId) this.calendarId = "popupCalendar" + new Date().getTime();
			this.setUpObservers();
			this.getCalendar();
			this.formValidatorInterface();
			DatePicker.pickers.push(this);
		},
		formValidatorInterface: function(){
			this.inputs.each(function(input){
				var props;
				if(input.get('validatorProps')){
					try {
						props = JSON.decode(input.get('validatorProps'));
					}catch(e){}
				}
				if (props && props.dateFormat) {
					dbug.log('using date format specified in validatorProps property of element to play nice with FormValidator');
					this.setOptions({ format: props.dateFormat });
				} else {
					if (!props) props = {};
					props.dateFormat = this.options.format;
					input.set('validatorProps', JSON.encode(props));
				}
			}, this);
		},
		calWidth: 280,
		inputDates: {},
		selectedDates: {},
		setUpObservers: function(){
			this.inputs.each(function(input) {
				if (this.options.showOnInputFocus) input.addEvent('focus', this.show.bind(this));
				input.addEvent('blur', function(e){
					if (e) {
						this.selectedDates = this.getDates(null, true);
						this.fillCalendar(this.selectedDates.start);
						if (this.options.updateOnBlur) this.updateInput();
					}
				}.bind(this));
			}, this);
			this.options.additionalShowLinks.each(function(lnk){
				$(lnk).addEvent('click', this.show.bind(this))
			}, this);
		},
		getDates: function(dates, getFromInputs){
			var d = {};
			if (!getFromInputs) dates = dates||this.selectedDates;
			var getFromInput = function(when){
				var input = this.inputs.get(when);
				if (input) d[when] = this.validDate(input.get('value'));
			}.bind(this);
			this.whens.each(function(when) {
				switch($type(dates)){
					case "object":
						if (dates) d[when] = dates[when]?dates[when]:dates;
						if (!d[when] && !d[when].format) getFromInput(when);
						break;
					default:
						getFromInput(when);
						break;
				}
				if (!d[when]) d[when] = this.selectedDates[when]||new Date();
			}, this);
			return d;
		},
		updateInput: function(){
			var d = {};
			$each(this.getDates(), function(value, key){
				var input = this.inputs.get(key);
				if (!input) return;
				input.set('value', (value)?this.formatDate(value)||"":"");
			}, this);
			return this;
		},
		validDate: function(val) {
			if (!$chk(val)) return null;
			var date = Date.parse(val.trim());
			return isNaN(date)?null:date;
		},
		formatDate: function (date) {
			return date.format(this.options.format);
		},
		getCalendar: function() {
			if(!this.calendar) {
				var cal = new Element("table", {
					'id': this.options.calendarId || '',
					'border':'0',
					'cellpadding':'0',
					'cellspacing':'0',
					'class':'datePicker'
				});
				var tbody = new Element('tbody').inject(cal);
				var rows = [];
				(8).times(function(i){
					var row = new Element('tr').inject(tbody);
					(7).times(function(i){
						var td = new Element('td').inject(row).set('html', '&nbsp;');
					});
				});
				var rows = tbody.getElements('tr');
				rows[0].addClass('dateNav');
				rows[1].addClass('dayNames');
				(6).times(function(i){
					rows[i+2].addClass('dayRow');
				});
				this.rows = rows;
				var dayCells = rows[1].getElements('td');
				dayCells.each(function(cell, i){
					cell.firstChild.data = Date.$days[(i + this.options.weekStartOffset) % 7].substring(0,3);
				}, this);
				[6,5,4,3].each(function(i){ rows[0].getElements('td')[i].dispose() });
				this.prevLnk = rows[0].getElement('td').setStyle('text-align', 'right');
				this.prevLnk.adopt(new Element('a').set('html', "&lt;").addClass('rightScroll'));
				this.month = rows[0].getElements('td')[1];
				this.month.set('colspan', 5);
				this.nextLnk = rows[0].getElements('td')[2].setStyle('text-align', 'left');
				this.nextLnk.adopt(new Element('a').set('html', '&gt;').addClass('leftScroll'));
				cal.addEvent('click', this.clickCalendar.bind(this));
				this.calendar = cal;
				this.container = new Element('div').adopt(cal).addClass('calendarHolder');
				this.content = StickyWin.ui('', this.container, $merge(this.options.stickyWinUiOptions, {
					cornerHandle: this.options.stickyWinOptions.draggable,
					width: this.calWidth
				}));
				var opts = $merge(this.options.stickyWinOptions, {
					content: this.content,
					className: 'datePickerSW',
					allowMultipleByClass: true,
					showNow: false,
					relativeTo: this.inputs.get('start')
				});
				this.stickyWin = new this.options.stickyWinToUse(opts);
				this.stickyWin.addEvent('onDisplay', this.positionClose.bind(this));
				this.container.setStyle('z-index', this.stickyWin.win.getStyle('z-index').toInt()+1);
			}
			return this.calendar;
		},
		positionClose: function(){
			if (this.closePositioned) return;
			var closer = this.content.getElement('div.closeButton');
			if (closer) {
				closer.inject(this.container, 'after').setStyle('z-index', this.stickyWin.win.getStyle('z-index').toInt()+2);
				(function(){
					this.content.setStyle('width', this.calendar.getSize().x + (this.options.time ? 240 : 40));
					closer.setPosition({relativeTo: this.stickyWin.win.getElement('.top'), position: 'upperRight', edge: 'upperRight'});
				}).delay(3, this);
			}
			this.closePositioned = true;
		},
		hide: function(){
			this.stickyWin.hide();
			this.fireEvent('onHide');
			return this;
		},
		hideOthers: function(){
			DatePicker.pickers.each(function(picker){
				if (picker != this) picker.hide();
			});
			return this;
		},
		show: function(){
			this.selectedDates = {};
			var dates = this.getDates(null, true);
			this.whens.each(function(when){
				this.inputDates[when] = dates[when]?dates[when].clone():dates.start?dates.start.clone():this.today;
		    this.selectedDates[when] = !this.inputDates[when] || isNaN(this.inputDates[when]) 
						? this.today 
						: this.inputDates[when].clone();
				this.getCalendar(when);
			}, this);
			this.fillCalendar(this.selectedDates.start);
			if (!this.options.showMoreThanOne) this.hideOthers();
			this.stickyWin.show();
			this.fireEvent('onShow');		
			return this;
		},
		handleScroll: function(e){
			if (e.target.hasClass('rightScroll')||e.target.hasClass('leftScroll')) {
				var newRef = e.target.hasClass('rightScroll')
					?this.rows[2].getElement('td').refDate - Date.$units.day()
					:this.rows[7].getElements('td')[6].refDate + Date.$units.day();
				this.fillCalendar(new Date(newRef));
				return true;
			}
			return false;
		},
		setSelectedDates: function(e, newDate){
			this.selectedDates.start = newDate;
		},
		onPick: function(){
			this.updateSelectors();
			this.inputs.each(function(input) {
				input.fireEvent("change");
				input.fireEvent("blur");
			});
			this.fireEvent('onPick');
			if(this.options.hideCalendarOnPick) this.hide();
		},
		clickCalendar: function(e) {
			if(this.options.format == "%d/%m/%Y") Date.$culture = "GB";
			if (this.handleScroll(e)) return;
			if (!e.target.firstChild || !e.target.firstChild.data) return;
			var val = e.target.firstChild.data;
			if (e.target.refDate) {
				var newDate = new Date(e.target.refDate);
				this.setSelectedDates(e, newDate);
				this.updateInput();
				this.onPick();
			}
		},
		fillCalendar: function (date) {
			if ($type(date) == "string") date = new Date(date);
			var startDate = (date)?new Date(date.getTime()):new Date();
			var hours = startDate.get('hours');
			startDate.setDate(1);
			startDate.setTime((startDate.getTime() - (Date.$units.day() * (startDate.getDay()))) + 
			                  (Date.$units.day() * this.options.weekStartOffset));
			var monthyr = new Element('span', {
				html: Date.$months[date.getMonth()] + " " + date.getFullYear()
			});
			$(this.rows[0].getElements('td')[1]).empty().adopt(monthyr);
			var atDate = startDate.clone();
			this.rows.each(function(row, i){
				if(i < 2) return;
				row.getElements('td').each(function(td){
					atDate.set('hours', hours);
					td.firstChild.data = atDate.getDate();
					td.refDate = atDate.getTime();
					atDate.setTime(atDate.getTime() + Date.$units.day());
				}, this);
			}, this);
			this.updateSelectors();
		},
		updateSelectors: function(){
			var atDate;
			var month = new Date(this.rows[5].getElement('td').refDate).getMonth();
			this.rows.each(function(row, i){
				if(i < 2) return;
				row.getElements('td').each(function(td){
					td.className = '';
					atDate = new Date(td.refDate);
					if(atDate.format("%x") == this.today.format("%x")) td.addClass('today');
					this.whens.each(function(when){
						var date = this.selectedDates[when];
						if(date && atDate.format("%x") == date.format("%x")) {
							td.addClass('selectedDate');
							this.fireEvent('selectedDateMatch', [td, when]);
						}
					}, this);
					this.fireEvent('rowDateEvaluated', [atDate, td]);
					if(atDate.getMonth() != month) td.addClass('otherMonthDate');
					atDate.setTime(atDate.getTime() + Date.$units.day());
				}, this);
			}, this);
		}
	});
})();

/*
Script: DatePicker.Extras.js
	Extends DatePicker to allow for range selection and time entry.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
DatePicker.refactor({
	options:{
		extraCSS: 'a.finish {position: relative;height: 13px !important;top: -31px !important;left: 85px !important;top: -34px;left: 77px;height: 16px;display:block;float: left;padding: 1px 12px 3px !important;}'+
			'div.calendarHolder div.time {border: #999 1px solid;width: 55px;position: relative;left: 3px;height: 17px;}'+
			'div.calendarHolder td.timeTD {width: 140px;} div.calendarHolder td.label{width:35px; text-align:right}'+
			'div.calendarHolder div.time select {font-size: 10px !important; font-size: 15px;padding: 0px;left:60px;position:absolute;top:-1px !important; width: auto !important;}'+
			'div.calendarHolder div.time input {width: 16px !important;width: 12px;padding: 2px;height: 13px;border: none !important;border: 1px solid #fff;}'+
			'div.calendarHolder div.timeSub {clear:both;position: relative;width: 65px;}'+
			'div.calendarHolder div.timeSub span {text-align: center;color: #999;margin: 5px;}'+
			'div.calendarHolder span.seperator {position:relative;top:-3px;}'+
			'div.calendarHolder table.stamp {position:relative;top: 35px !important;top: 50px;left: 0px;}'+
			'div.calendarHolder table.stamp a {left:123px;position:relative;top:9px;}'+
			'div.calendarHolder table.stamp td {border: none !important;}'+
			'div.calendarHolder td.selected_end {border-width: 1px 1px 1px 0px !important;margin: 0px 0px 0px 1px !important;}'+
			'div.calendarHolder td.selected_start {border-width: 1px 0px 1px 1px !important;margin: 0px 1px 0px 0px !important;}'+
			'div.calendarHolder table.datePicker td.range {background: #dcddef;border: solid #20397b;border-width: 1px 0px;margin: 0px 1px !important;}',
		range: false,
		time: false
	},
	initialize: function(inputs, options){
		if (options && (options.range || options.time)) {
			options = $merge({
				hideCalendarOnPick: false
			}, options);
		}
		if (options && options.time && !options.format) {
			options.format = "%x %X";
		}
		this.setOptions(options);
		this.whens = (this.options.range)?['start', 'end']:['start'];
		if ($type(inputs) == 'object') {
			this.inputs = $H(inputs);
		} else if ($type($(inputs)) == "element") {
			this.inputs = $H({'start': $(inputs)});
		} else if ($type(inputs) == "array"){
			inputs = $$(inputs);
			this.inputs = $H({});
			this.whens.each(function(when, i){
				this.inputs.set(when, inputs[i]);
			}, this);
		}
		if (this.options.time) this.calWidth = 460;
		this.parent(inputs, this.options);
		this.createStyle(this.options.extraCSS, 'datePickerPlusStyle');
		this.addEvent('rowDateEvaluated', function(atDate, td){
			if (this.options.range && this.selectedDates.start.diff(atDate, 'minute') > 0 
					&& this.selectedDates.end.diff(atDate, 'minute') < 0) td.addClass('range');
		}.bind(this));
		this.addEvent('selectedDateMatch', function(td, when){
			if(this.options.range) td.addClass('selected_'+when);
		}.bind(this));
	},
	updateInput: function(){
		this.parent();
		if (this.options.time) this.updateView();
	},
	updateView: function() {
		this.whens.each(function(when){
			var stamp = this.stamps[when];
			var date = this.getDates()[when];
			stamp.date.set('html', date?date.format("%b. %d, %Y"):"");
			if (stamp.hr) {
				stamp.hr.set('value', date?date.format("%I"):"");
				stamp.min.set('value', date?date.format("%M"):"");
			}
		}, this);
	},
	stamps: {},
	setupWideView: function(){
		var timeStampMap = {
			hr: '%I',
			'min': '%M'
		};
		timeSetMap = {
			hr: 'setHours',
			'min':'setMinutes'
		};
		var dates = this.getDates();
	
		if (!this.options.range && !this.options.time) return;
		this.stamps.table = new Element('table', {
			'class':'stamp'
		}).inject(this.container);
		this.stamps.tbody = new Element('tbody').inject(this.stamps.table);
		this.whens.each(function(when){
			this.stamps[when] = {};
			var s = this.stamps[when]; //saving some bytes
			s.container = new Element('tr').addClass(when+'_stamp').inject(this.stamps.tbody);
			s.label = new Element('td').inject(s.container).addClass('label');
			if(this.whens.length == 1) {
				s.label.set('html', 'date:');
			} else {
				s.label.set('html', when=="start"?"from:":"to:");
			}
			s.date = new Element('td').inject(s.container);
			if (this.options.time) {
				currentWhen = dates[when]||new Date();
				s.time = new Element('tr').inject(this.stamps.tbody);
				new Element('td').inject(s.time);
				s.timeTD = new Element('td').inject(s.time);
				s.timeInputs = new Element('div').addClass('time clearfix').inject(s.timeTD);
				s.timeSub = new Element('div', {'class':'timeSub'}).inject(s.timeTD);
				['hr','min'].each(function(t, i){
					s[t] = new Element('input', {
						type: 'text',
						'class': t,
						name: t,
						events: {
							focus: function(){
								this.select();
							},
							change: function(){
								this.selectedDates[when][timeSetMap[t]](s[t].get('value'));
								this.selectedDates[when].setAMPM(s.ampm.get('value'));
								this.updateInput();
							}.bind(this)
						}
					}).inject(s.timeInputs);
					s[t].set('value', currentWhen.format(timeStampMap[t]));
					if (i < 1) s.timeInputs.adopt(new Element('span', {'class':'seperator'}).set('html', ":"));
					new Element('span', {
						'class': t
					}).set('html', t).inject(s.timeSub);
				}, this);
				s.ampm = new Element('select').inject(s.timeInputs);
				['AM','PM'].each(function(ampm){
					var opt = new Element('option', {
						value: ampm,
						text: ampm.toLowerCase()
					}).set('html', ampm).inject(s.ampm);
					if (ampm == currentWhen.format("%p")) opt.selected = true;
				});
				s.ampm.addEvent('change', function(){
					var date = this.getDates()[when];
					var ampm = s.ampm.get('value');
					if (ampm != date.format("%p")) {
						date.setAMPM(ampm);
						this.updateInput();
					}
				}.bind(this));
			}
		}, this);
		new Element('tr').inject(this.stamps.tbody).adopt(new Element('td', {colspan: 2}).adopt(new Element('a', {
			'class':'closeSticky button',
			events: {
				click: function(){
					this.hide();
				}.bind(this)
			}
		}).set('html', 'Ok')));
	},
	show: function(){
		this.parent();
		if (this.options.time) {
			if (!this.stamps.table) this.setupWideView();
			this.updateView();
		}
	},
	startSet: false,
	onPick: function(){
		if((this.options.range && this.selectedDates.start && this.selectedDates.end) || !this.options.range) {
			this.parent();
		}
	},
	setSelectedDates: function(e, newDate) {
		if(this.options.range) {
			if (this.selectedDates.start && this.startSet) {
				if (this.selectedDates.start.getTime() > newDate.getTime()){
					this.selectedDates.end = new Date(this.selectedDates.start);
					this.selectedDates.start = newDate;
				} else {
					this.selectedDates.end = newDate;
				}
				this.startSet = false;
			} else {
				this.selectedDates.start = newDate;
				if (this.selectedDates.end && this.selectedDates.start.getTime() > this.selectedDates.end.getTime())
					this.selectedDates.end = new Date(newDate);
				this.startSet = true;
			}
		} else {
			this.parent(e, newDate);
		}
		if(this.options.time) {
			this.whens.each(function(when){
				var hr = this.stamps[when].hr.get('value').toInt();
				if (this.stamps[when].ampm.get('value') == "PM" && hr < 12) hr += 12;
				this.selectedDates[when].setHours(hr);
				this.selectedDates[when].setMinutes(this.stamps[when]['min'].get('value')||"0");
				this.selectedDates[when].setAMPM(this.stamps[when].ampm.get('value')||"AM");
			}, this);
		}
	}
});

/*
Script: FormValidator.js
	A css-class based form validation system.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var InputValidator = new Class({
	Implements: [Options],
	options: {
		errorMsg: 'Validation failed.',
		test: function(field){return true;}
	},
	initialize: function(className, options){
		this.setOptions(options);
		this.className = className;
	},
	test: function(field, props){
		if($(field)) return this.options.test($(field), props||this.getProps(field));
		else return false;
	},
	getError: function(field, props){
		var err = this.options.errorMsg;
		if($type(err) == "function") err = err($(field), props||this.getProps(field));
		return err;
	},
	getProps: function(field){
		if (!$(field)) return {};
		return field.get('validatorProps');
	}
});

Element.Properties.validatorProps = {

	set: function(props){
		return this.eliminate('validatorProps').store('validatorProps', props);
	},

	get: function(props){
		if (props) this.set(props);
		if (this.retrieve('validatorProps')) return this.retrieve('validatorProps');
		if(this.getProperty('validatorProps')){
			try {
				this.store('validatorProps', JSON.decode(this.getProperty('validatorProps')));
			}catch(e){ return {}}
		} else {
			var vals = this.get('class').split(' ').filter(function(cls) {
				//return cls.match(/[a-z].*\[\'.*\'\]/ig);
				return cls.test(':');
			});
			if (!vals.length) {
				this.store('validatorProps', {});
			} else {
				props = {};
				vals.each(function(cls){
					var split = cls.indexOf(':');
					props[cls.substring(0, split)] = JSON.decode(cls.substring(split+1, cls.length))
				});
				this.store('validatorProps', props);
			}
		}
		return this.retrieve('validatorProps');
	}

};

var FormValidator = new Class({
	Implements:[Options, Events],
	options: {
		fieldSelectors:"input, select, textarea",
		ignoreHidden: true,
		useTitles:false,
		evaluateOnSubmit:true,
		evaluateFieldsOnBlur: true,
		evaluateFieldsOnChange: true,
		serial: true,
		stopOnFailure: true,
		scrollToErrorsOnSubmit: true,
		warningPrefix: function(){
			return FormValidator.resources[FormValidator.language].warningPrefix || 'Warning: ';
		},
		errorPrefix: function(){
			return FormValidator.resources[FormValidator.language].errorPrefix || 'Error: ';
		}
//	onFormValidate: function(isValid, form){},
//	onElementValidate: function(isValid, field){}
	},
	initialize: function(form, options){
		this.setOptions(options);
		this.form = $(form);
		this.form.store('validator', this);
		this.warningPrefix = $lambda(this.options.warningPrefix)();
		this.errorPrefix = $lambda(this.options.errorPrefix)();

		if(this.options.evaluateOnSubmit) this.form.addEvent('submit', this.onSubmit.bind(this));
		if(this.options.evaluateFieldsOnBlur) this.watchFields();
	},
	toElement: function(){
		return this.form;
	},
	getFields: function(){
		return this.fields = this.form.getElements(this.options.fieldSelectors);
	},
	watchFields: function(){
		this.getFields().each(function(el){
				el.addEvent('blur', this.validateField.pass([el, false], this));
			if(this.options.evaluateFieldsOnChange)
				el.addEvent('change', this.validateField.pass([el, true], this));
		}, this);
	},
	onSubmit: function(event){
		if(!this.validate(event) && event) event.preventDefault();
		else this.reset();
	},
	reset: function() {
		this.getFields().each(this.resetField, this);
		return this;
	}, 
	validate: function(event) {
		var result = this.getFields().map(function(field) { 
			return this.validateField(field, true);
		}, this).every(function(v){ return v;});
		this.fireEvent('onFormValidate', [result, this.form, event]);
		if (this.options.stopOnFailure && !result && event) event.preventDefault();
		if (this.options.scrollToErrorsOnSubmit && !result) {
			var par = this.form.getParent();
			var isScrolled = function(p){
				return p.getScrollSize().y != p.getSize().y
			};
			var scrolls;
			while (par != document.body && !isScrolled(par)) {
				par = par.getParent();
			};
			var fx = par.retrieve('fvScroller');
			if (!fx && window.Fx) {
				fx = new Fx.Scroll(par, {
					transition: 'quad:out',
					offset: {
						y: -20
					}
				});
				par.store('fvScroller', fx);
			}
			var failed = this.form.getElement('.validation-failed');
			if (failed) {
				if (fx) fx.toElement(failed);
				else par.scrollTo(par.getScroll().x, failed.getPosition(par).y - 20);
			}
		}
		return result;
	},
	validateField: function(field, force){
		if (this.paused) return true;
		field = $(field);
		var passed = !field.hasClass('validation-failed');
		var failed, warned;
		if (this.options.serial && !force) {
			failed = this.form.getElement('.validation-failed');
			warned = this.form.getElement('.warning');
		}
		if(field && (!failed || force || field.hasClass('validation-failed') || (failed && !this.options.serial))){
			var validators = field.className.split(" ").some(function(cn){
				return this.getValidator(cn);
			}, this);
			var validatorsFailed = [];
			field.className.split(" ").each(function(className){
				if (!this.test(className,field)) validatorsFailed.include(className);
			}, this);
			passed = validatorsFailed.length === 0;
			if (validators && !field.hasClass('warnOnly')){
				if(passed) {
					field.addClass('validation-passed').removeClass('validation-failed');
					this.fireEvent('onElementPass', field);
				} else {
					field.addClass('validation-failed').removeClass('validation-passed');
					this.fireEvent('onElementFail', [field, failed]);
				}
			}
			if(!warned) {
				var warnings = field.className.split(" ").some(function(cn){
					if(cn.test('^warn-') || field.hasClass('warnOnly')) 
						return this.getValidator(cn.replace(/^warn-/,""));
					else return null;
				}, this);
				field.removeClass('warning');
				var warnResult = field.className.split(" ").map(function(cn){
					if(cn.test('^warn-') || field.hasClass('warnOnly')) 
						return this.test(cn.replace(/^warn-/,""), field, true);
					else return null;
				}, this);
			}
		}
		return passed;
	},
	getPropName: function(className){
		return 'advice'+className;
	},
	test: function(className, field, warn){
		field = $(field);
		if(field.hasClass('ignoreValidation')) return true;
		warn = $pick(warn, false);
		if(field.hasClass('warnOnly')) warn = true;
		var isValid = true;
		var validator = this.getValidator(className);
		if(validator && this.isVisible(field)) {
			isValid = validator.test(field);
			if(!isValid && validator.getError(field)){
				if(warn) field.addClass('warning');
				var advice = this.makeAdvice(className, field, validator.getError(field), warn);
				this.insertAdvice(advice, field);
				this.showAdvice(className, field);
			} else this.hideAdvice(className, field);
			this.fireEvent('onElementValidate', [isValid, field, className]);
		}

		if(warn) return true;
		return isValid;
	},
	getAllAdviceMessages: function(field, force) {
		var advice = [];
		if (field.hasClass('ignoreValidation') && !force) return advice;
		var validators = field.className.split(" ").some(function(cn){
			var warner = cn.test('^warn-') || field.hasClass('warnOnly');
			if (warner) cn = cn.replace(/^warn-/,"");
			var validator = this.getValidator(cn);
			if (!validator) return;
			advice.push({
				message: validator.getError(field),
				warnOnly: warner,
				passed: validator.test(),
				validator: validator
			});
		}, this);
		return advice;
	},
	showAdvice: function(className, field){
		var advice = this.getAdvice(className, field);
		if(advice && !field.retrieve(this.getPropName(className))
			 && (advice.getStyle('display') == "none" 
			 || advice.getStyle('visiblity') == "hidden" 
			 || advice.getStyle('opacity')==0)){
			field.store(this.getPropName(className), true);
			if(advice.reveal) advice.reveal();
			else advice.setStyle('display','block');
		}
	},
	hideAdvice: function(className, field){
		var advice = this.getAdvice(className, field);
		if(advice && field.retrieve(this.getPropName(className))) {
			field.store(this.getPropName(className), false);
			//if Fx.Reveal.js is present, transition the advice out
			if(advice.dissolve) advice.dissolve();
			else advice.setStyle('display','none');
		}
	},
	isVisible : function(field) {
		if (!this.options.ignoreHidden) return true;
		while(field != document.body) {
			if($(field).getStyle('display') == "none") return false;
			field = field.getParent();
		}
		return true;
	},
	getAdvice: function(className, field) {
		return field.retrieve('advice-'+className);
	},
	makeAdvice: function(className, field, error, warn){
		var errorMsg = (warn)?this.warningPrefix:this.errorPrefix;
				errorMsg += (this.options.useTitles) ? field.title || error:error;
		var advice = this.getAdvice(className, field);
		if(!advice){
			var cssClass = (warn)?'warning-advice':'validation-advice';
			advice = new Element('div', {
				text: errorMsg,
				styles: { display: 'none' },
				id: 'advice-'+className+'-'+this.getFieldId(field)
			}).addClass(cssClass);
		} else{
			advice.set('html', errorMsg);
		}
		field.store('advice-'+className, advice);
		return advice;
	},
	insertAdvice: function(advice, field){
		//Check for error position prop
		var props = field.get('validatorProps');
		//Build advice
		if (!props.msgPos || !$(props.msgPos)) {
			switch (field.type.toLowerCase()) {
				case 'radio':
					var p = field.getParent().adopt(advice);
					break;
				default: 
					advice.inject($(field), 'after');
			};
		} else {
			$(props.msgPos).grab(advice);
		}
	},
	getFieldId : function(field) {
		return field.id ? field.id : field.id = "input_"+field.name;
	},
	resetField: function(field) {
		field = $(field);
		if(field) {
			var cn = field.className.split(" ");
			cn.each(function(className) {
				if(className.test('^warn-')) className = className.replace(/^warn-/,"");
				var prop = this.getPropName(className);
				if(field.retrieve(prop)) this.hideAdvice(className, field);
				field.removeClass('validation-failed');
				field.removeClass('warning');
				field.removeClass('validation-passed');
			}, this);
		}
		return this;
	},
	stop: function(){
		this.paused = true;
		return this;
	},
	start: function(){
		this.paused = false;
		return this;
	},
	ignoreField: function(field, warn){
		field = $(field);
		if(field){
			this.enforceField(field);
			if(warn) field.addClass('warnOnly');
			else field.addClass('ignoreValidation');
		}
		return this;
	},
	enforceField: function(field){
		field = $(field);
		if(field) field.removeClass('warnOnly').removeClass('ignoreValidation');
		return this;
	}
});

FormValidator.resources = {
	enUS: {
		required:'This field is required.',
		minLength:'Please enter at least {minLength} characters (you entered {length} characters).',
		maxLength:'Please enter no more than {maxLength} characters (you entered {length} characters).',
		integer:'Please enter an integer in this field. Numbers with decimals (e.g. 1.25) are not permitted.',
		numeric:'Please enter only numeric values in this field (i.e. "1" or "1.1" or "-1" or "-1.1").',
		digits:'Please use numbers and punctuation only in this field (for example, a phone number with dashes or dots is permitted).',
		alpha:'Please use letters only (a-z) with in this field. No spaces or other characters are allowed.',
		alphanum:'Please use only letters (a-z) or numbers (0-9) only in this field. No spaces or other characters are allowed.',
		dateSuchAs:'Please enter a valid date such as {date}',
		dateInFormatMDY:'Please enter a valid date such as MM/DD/YYYY (i.e. "12/31/1999")',
		email:'Please enter a valid email address. For example "fred@domain.com".',
		url:'Please enter a valid URL such as http://www.google.com.',
		currencyDollar:'Please enter a valid $ amount. For example $100.00 .',
		oneRequired:'Please enter something for at least one of these inputs.',
		errorPrefix: 'Error: ',
		warningPrefix: 'Warning: '
	}
};
FormValidator.language = "enUS";
FormValidator.getMsg = function(key, language){
	return FormValidator.resources[language||FormValidator.language][key];
};

FormValidator.adders = {
	validators:{},
	add : function(className, options) {
		this.validators[className] = new InputValidator(className, options);
		//if this is a class
		//extend these validators into it
		if(!this.initialize){
			this.implement({
				validators: this.validators
			});
		}
	},
	addAllThese : function(validators) {
		$A(validators).each(function(validator) {
			this.add(validator[0], validator[1]);
		}, this);
	},
	getValidator: function(className){
		return this.validators[className.split(":")[0]];
	}
};
$extend(FormValidator, FormValidator.adders);
FormValidator.implement(FormValidator.adders);

FormValidator.add('IsEmpty', {
	errorMsg: false,
	test: function(element) { 
		if(element.type == "select-one"||element.type == "select")
			return !(element.selectedIndex >= 0 && element.options[element.selectedIndex].value != "");
		else
			return ((element.get('value') == null) || (element.get('value').length == 0));
	}
});

FormValidator.addAllThese([
	['required', {
		errorMsg: function(){
			return FormValidator.getMsg('required');
		},
		test: function(element) { 
			return !FormValidator.getValidator('IsEmpty').test(element); 
		}
	}],
	['minLength', {
		errorMsg: function(element, props){
			if($type(props.minLength))
				return FormValidator.getMsg('minLength').substitute({minLength:props.minLength,length:element.get('value').length });
			else return '';
		}, 
		test: function(element, props) {
			if($type(props.minLength)) return (element.get('value').length >= $pick(props.minLength, 0));
			else return true;
		}
	}],
	['maxLength', {
		errorMsg: function(element, props){
			//props is {maxLength:10}
			if($type(props.maxLength))
				return FormValidator.getMsg('maxLength').substitute({maxLength:props.maxLength,length:element.get('value').length });
			else return '';
		}, 
		test: function(element, props) {
			//if the value is <= than the maxLength value, element passes test
			return (element.get('value').length <= $pick(props.maxLength, 10000));
		}
	}],
	['validate-integer', {
		errorMsg: FormValidator.getMsg.pass('integer'),
		test: function(element) {
			return FormValidator.getValidator('IsEmpty').test(element) || /^-?[1-9]\d*$/.test(element.get('value'));
		}
	}],
	['validate-numeric', {
		errorMsg: FormValidator.getMsg.pass('numeric'), 
		test: function(element) {
			return FormValidator.getValidator('IsEmpty').test(element) || 
				/^-?(?:0$0(?=\d*\.)|[1-9]|0)\d*(\.\d+)?$/.test(element.get('value'));
		}
	}],
	['validate-digits', {
		errorMsg: FormValidator.getMsg.pass('digits'), 
		test: function(element) {
			return FormValidator.getValidator('IsEmpty').test(element) || (/^[\d() .:\-\+#]+$/.test(element.get('value')));
		}
	}],
	['validate-alpha', {
		errorMsg: FormValidator.getMsg.pass('alpha'), 
		test: function (element) {
			return FormValidator.getValidator('IsEmpty').test(element) ||  /^[a-zA-Z]+$/.test(element.get('value'))
		}
	}],
	['validate-alphanum', {
		errorMsg: FormValidator.getMsg.pass('alphanum'), 
		test: function(element) {
			return FormValidator.getValidator('IsEmpty').test(element) || !/\W/.test(element.get('value'))
		}
	}],
	['validate-date', {
		errorMsg: function(element, props) {
			if (Date.parse) {
				var format = props.dateFormat || "%x";
				return FormValidator.getMsg('dateSuchAs').substitute({date:new Date().format(format)});
			} else {
				return FormValidator.getMsg('dateInFormatMDY');
			}
		},
		test: function(element, props) {
			if(FormValidator.getValidator('IsEmpty').test(element)) return true;
			if (Date.parse) {
				var format = props.dateFormat || "%x";
				var d = Date.parse(element.get('value'));
				var formatted = d.format(format);
				if (formatted != "invalid date") element.set('value', formatted);
				return !isNaN(d);
			} else {
			var regex = /^(\d{2})\/(\d{2})\/(\d{4})$/;
			if(!regex.test(element.get('value'))) return false;
			var d = new Date(element.get('value').replace(regex, '$1/$2/$3'));
			return (parseInt(RegExp.$1, 10) == (1+d.getMonth())) && 
			(parseInt(RegExp.$2, 10) == d.getDate()) && 
			(parseInt(RegExp.$3, 10) == d.getFullYear() );
			}
		}
	}],
	['validate-email', {
		errorMsg: FormValidator.getMsg.pass('email'), 
		test: function (element) {
			return FormValidator.getValidator('IsEmpty').test(element) || /^[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,4}$/i.test(element.get('value'));
		}
	}],
	['validate-url', {
		errorMsg: FormValidator.getMsg.pass('url'), 
		test: function (element) {
			return FormValidator.getValidator('IsEmpty').test(element) || /^(https?|ftp|rmtp|mms):\/\/(([A-Z0-9][A-Z0-9_-]*)(\.[A-Z0-9][A-Z0-9_-]*)+)(:(\d+))?\/?/i.test(element.get('value'));
		}
	}],
	['validate-currency-dollar', {
		errorMsg: FormValidator.getMsg.pass('currencyDollar'), 
		test: function(element) {
			// [$]1[##][,###]+[.##]
			// [$]1###+[.##]
			// [$]0.##
			// [$].##
			return FormValidator.getValidator('IsEmpty').test(element) ||  /^\$?\-?([1-9]{1}[0-9]{0,2}(\,[0-9]{3})*(\.[0-9]{0,2})?|[1-9]{1}\d*(\.[0-9]{0,2})?|0(\.[0-9]{0,2})?|(\.[0-9]{1,2})?)$/.test(element.get('value'));
		}
	}],
	['validate-one-required', {
		errorMsg: FormValidator.getMsg.pass('oneRequired'), 
		test: function (element) {
			var p = element.parentNode;
			return p.getElements('input').some(function(el) {
				if (['checkbox', 'radio'].contains(el.get('type'))) return el.get('checked');
				return el.get('value');
			});
		}
	}]
]);

Element.Properties.validator = {

	set: function(options){
		var validator = this.retrieve('validator');
		if (validator) validator.setOptions(options);
		return this.store('validator:options');
	},

	get: function(options){
		if (options || !this.retrieve('validator')){
			if (options || !this.retrieve('validator:options')) this.set('validator', options);
			this.store('validator', new FormValidator(this, this.retrieve('validator:options')));
		}
		return this.retrieve('validator');
	}

};

Element.implement({
	validate: function(options){
		this.set('validator', options);
		return this.get('validator', options).validate();
	}
});

FormValidator.Tips = new Class({
	Extends: FormValidator,
	options: {
		pointyTipOptions: {
			point: "left"
		}
//		tipCaption: ''
	},
	showAdvice: function(className, field){
		var advice = this.getAdvice(field);
		if(advice && !advice.visible){
			advice.show();
			advice.position();
			advice.pointy.positionPointer();
		}
	},
	hideAdvice: function(className, field){
		var advice = this.getAdvice(field);
		if(advice && advice.visible) advice.show();
	},
	getAdvice: function(className, field) {
		var params = Array.link(arguments, {field: Element.type});
		return params.field.retrieve('PointyTip');
	},
	advices: [],
	makeAdvice: function(className, field, error, warn){
		if (!error && !warn) return;
		var advice = field.retrieve('PointyTip');
		if(!advice){
			var cssClass = warn?'warning-advice':'validation-advice';
			var msg = new Element('ul', {
				styles: {
					margin: 0,
					padding: 0,
					listStyle: 'none'
				}
			});
			var li = this.makeAdviceItem(className, field);
			if (li) msg.adopt(li);
			field.store('validationMsgs', msg);
			advice = new StickyWin.PointyTip(this.options.tipCaption, msg, $merge(this.options.pointyTipOptions, {
				showNow: false,
				relativeTo: field
			}));
			this.advices.push(advice);
			advice.msgs = {};
			field.store('PointyTip', advice);
			$(advice).addClass(cssClass).set('id', 'advice-'+className+'-'+this.getFieldId(field));			
		}
		field.store('advice-'+className, advice);
		this.appendAdvice(className, field, error, warn);
		advice.pointy.positionPointer();
		return advice;
	},
	validateField: function(field, force){
		var advice = this.getAdvice(field);
		var anyVis = this.advices.some(function(a){ return a.visible; });
		if (anyVis && this.options.serial) {
			if (advice && advice.visible) {
				var passed = this.parent(field, force);
				if (!field.hasClass('validation-failed')) advice.hide();
			}
			return passed;
		}
		var msgs = field.retrieve('validationMsgs');
		if (msgs) msgs.getChildren().hide();
		if (field.hasClass('validation-failed') || field.hasClass('warning')) if (advice) advice.show();
		if (this.options.serial) {
			var fields = this.form.getElements('.validation-failed, .warning');
			if (fields.length) {
				fields.each(function(f, i) {
					var adv = this.getAdvice(f);
					adv.hide();
				}, this);
			}
		}
		return this.parent(field, force);
	},
	makeAdviceItem: function(className, field, error, warn){
		if (!error && !warn) return;
		var advice = this.getAdvice(field);
		var errorMsg = this.makeAdviceMsg(field, error, warn);
		if (advice && advice.msgs[className]) return advice.msgs[className].set('html', errorMsg);
		return new Element('li', {
			html: errorMsg,
			display: 'none'
		});
	},
	makeAdviceMsg: function(field, error, warn){
		var errorMsg = (warn)?this.warningPrefix:this.errorPrefix;
			errorMsg += (this.options.useTitles) ? field.title || error:error;
		return errorMsg;
	},
	appendAdvice: function(className, field, error, warn) {
		var advice = this.getAdvice(field);
		if (advice.msgs[className]) return advice.msgs[className].set('html', this.makeAdviceMsg(field, error, warn)).show();
		var li = this.makeAdviceItem(className, field, error, warn);
		if (!li) return;
		li.inject(field.retrieve('validationMsgs'));
		li[li.reveal?'reveal':'show']();
		advice.msgs[className] = li;
	},
	insertAdvice: function(advice, field){
		//Check for error position prop
		var props = field.get('validatorProps');
		//Build advice
		if (!props.msgPos || !$(props.msgPos)) {
			switch (field.type.toLowerCase()) {
				case 'radio':
					var p = field.getParent().adopt(advice);
					break;
				default: 
					$(advice).inject($(field), 'after');
			};
		} else {
			$(props.msgPos).grab(advice);
		}
		advice.position();
	}
});

$extend(FormValidator.resources.enUS, {
	noSpace: 'There can be no spaces in this input.',
	reqChkByNode: 'No items are selected.',
	requiredChk: 'This field is required.',
	reqChkByName: 'Please select a {label}.',
	match: 'This field needs to match the {matchName} field',
	startDate: 'the start date',
	endDate: 'the end date',
	currendDate: 'the current date',
	afterDate: 'The date should be the same or after {label}.',
	beforeDate: 'The date should be the same or before {label}.',
	startMonth: 'Please select a start month',
	sameMonth: 'These two dates must be in the same month - you must change one or the other.'
});

FormValidator.addAllThese([
  ['validate-enforce-oncheck', {
    test: function(element, props) {
      if (element.checked) {
        (props.toEnforce || $(props.enforceChildrenOf).getElements('input, select, textarea')).map(function(item) {
          FV.enforceField(item);
        });
      }
      return true;
    }
  }],
  ['validate-ignore-oncheck', {
    test: function(element, props) {
      if (element.checked) {
        (props.toIgnore || $(props.ignoreChildrenOf).getElements('input, select, textarea')).each(function(item) {
          FV.ignoreField(item);
          FV.resetField(item);
        });
      }
      return true;
    }
  }],
  ['validate-nospace', {
    errorMsg: function(){
				return FormValidator.getMsg('noSpace');
			},
     test: function(element, props) {
					return !element.get('value').test(/\s/);
     }
  }],
  ['validate-toggle-oncheck', {
    test: function(element, props) {
	dbug.log(props);
      var parentForm = element.getParent('form').retrieve('validator');
      var eleArr = props.toToggle || $(props.toToggleChildrenOf).getElements('input, select, textarea');
	dbug.log(eleArr);
      if (!element.checked) {
        eleArr.each(function(item) {
          parentForm.ignoreField(item);
          parentForm.resetField(item);
        });
      } else {
				eleArr.each(function(item) {
					parentForm.enforceField(item);
				});
			}
      return true;
    }
  }],
  ['validate-reqchk-bynode', {
    errorMsg: function(){
			return FormValidator.getMsg('reqChkByNode');
		},
    test: function(element, props) {
      return ($(props.nodeId).getElements(props.selector || 'input[type=checkbox], input[type=radio]')).some(function(item){
        return item.checked;
      });
    }
  }],
  ['validate-required-check', {
    errorMsg: function(element, props) {
      return props.useTitle ? element.get('title') : FormValidator.getMsg('requiredChk');
    },
    test: function(element, props) {
      return !!element.checked;
    }
  }],
  ['validate-reqchk-byname', {
    errorMsg: function(element, props) {
      return FormValidator.getMsg('reqChkByName').substitute({label: props.label || element.get('type')});
    },
    test: function(element, props) {
      var grpName = props.groupName || element.get('name');
      var oneCheckedItem = $$(document.getElementsByName(grpName)).some(function(item, index){
        return item.checked;
      });
      var fv = element.getParent('form').retrieve('validator');
      if (oneCheckedItem && fv) fv.resetField(element);
      return oneCheckedItem;
    }
  }],
  ['validate-validate-match', {
    errorMsg: function(element, props) {
			return FormValidator.getMsg('match').substitute({matchName: props.matchName || $(props.matchInput).get('name')});
    }, 
    test: function(element, props){
      var eleVal = element.get('value');
      var matchVal = $(props.matchInput) && $(props.matchInput).get('value');
      return eleVal && matchVal ? eleVal == matchVal : true;
    }
  }],
  ['validate-after-date', {
    errorMsg: function(element, props) {
			return FormValidator.getMsg('afterDate').substitute({
				label: props.afterLabel || (props.afterElement ? FormValidator.getMsg('startDate') : FormValidator.getMsg('currentDate'))
			});
		},
    test: function(element, props) {
      var start = $(props.afterElement) ? Date.parse($(props.afterElement).get('value')) : new Date();
      var end = Date.parse(element.get('value'));
			return end && start ? end >= start : true;
    }
  }],
  ['validate-before-date', {
    errorMsg: function(element, props) {
			return FormValidator.getMsg('beforeDate').substitute({
				label: props.beforeLabel || (props.beforeElement ? FormValidator.getMsg('endDate') : FormValidator.getMsg('currentDate'))
			});
		},
    test: function(element, props) {
      var start = Date.parse(element.get('value'));
      var end = $(props.beforeElement) ? Date.parse($(props.beforeElement).get('value')) : new Date();
			return end && start ? end >= start : true;
    }
  }],
  ['validate-custom-required', {
    errorMsg: function(){
			return FormValidator.getMsg('required');
		},
    test: function(element, props) {
      return element.get('value') != props.emptyValue;
    }
  }],
  ['validate-same-month', {
    errorMsg: function(element, props) {
      var startMo = $(props.sameMonthAs) && $(props.sameMonthAs).get('value');
      var eleVal = element.get('value');
      if (eleVal != '') {
        if (!startMo) { return FormValidator.getMsg('startMonth');}
        else {
          return FormValidator.getMsg('sameMonth');
        }
      }
    },
    test: function(element, props) {
      var d1 = Date.parse(element.get('value'));
      var d2 = Date.parse($(props.sameMonthAs) && $(props.sameMonthAs).get('value'));
      return d1 && d2 ? d1.format("%B") == d2.format("%B") : true;
    }
  }]
]);


/*
Script: Fupdate.js
	Handles the basic functionality of submitting a form and updating a dom element with the result.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

var Fupdate = new Class({
	Implements: [Options, Events, Occlude, ToElement],
	options: {
		//onFailure: $empty,
		//onSuccess: #empty, //aliased to onComplete,
		//onSend: $empty
		requestOptions: {
			evalScripts: true,
			useWaiter: true
		},
		extraData: {},
		resetForm: true
	},
	property: 'fupdate',
	initialize: function(form, update, options) {
		this.element = $(form);
		if (this.occlude()) return this.occludes;
		this.update = $(update);
		this.setOptions(options);
		this.makeRequest();
		if (this.options.resetForm) {
			this.request.addEvent('success', function(){
				$try(function(){ $(this).reset(); }.bind(this));
				if (window.OverText) OverText.update();
			}.bind(this));
		}
		this.addFormEvent();
	},
	makeRequest: function(){
		this.request = new Request.HTML($merge({
				url: $(this).get('action'),
				update: this.update,
				emulation: false,
				waiterTarget: $(this)
		}, this.options.requestOptions)).addEvents({
			success: function(text, xml){
				['success', 'complete'].each(function(evt){
					this.fireEvent(evt, [this.update, text, xml]);
				}, this);				
			}.bind(this),
			failure: function(xhr){
				this.fireEvent('failure', xhr);
			}.bind(this)
		});
	},
	addFormEvent: function(){
		var fv = $(this).retrieve('validator');
		if (fv) {
			fv.addEvent('onFormValidate', function(valid, form, e) {
				if (valid || !fv.options.stopOnFailure) {
					e.stop();
					this.send();
				}
			}.bind(this));
		} else {
			$(this).addEvent('submit', function(e){
				e.stop();
				this.send();
			}.bind(this));
		}
	},
	send: function(){
		var formData = $(this).toQueryString().dedupeQs().parseQuery(false, false);
		var data = $H(this.options.extraData).combine(formData);
		this.fireEvent('send', [$(this), data]);
		this.request.send(unescape(data.toQueryString()));
	}
});

Element.Properties.fupdate = {

	set: function(){
		var opt = Array.link(arguments, {options: Object.type, update: Element.type, updateId: String.type});
		var update = opt.update || opt.updateId;
		var fupdate = this.retrieve('fupdate');
		if (update) {
			if (fupdate) fupdate.update = $(update);
			this.store('fupdate:update', update);
		}
		if (opt.options) {
			if (fupdate) fupdate.setOptions(opt.options);
			this.store('fupdate:options', opt.options)
		}
		return this;
	},

	get: function(){
		var opt = Array.link(arguments, {options: Object.type, update: Element.type, updateId: String.type});
		var update = opt.update || opt.updateId;
		if (opt.options || update || !this.retrieve('fupdate')){
			if (opt.options || !this.retrieve('fupdate:options')) this.set('fupdate', opt.options);
			if (update) this.set('fupdate', update);
			this.store('fupdate', new Fupdate(this, this.retrieve('fupdate:update'), this.retrieve('fupdate:options')));
		}
		return this.retrieve('fupdate');
	}

};

Element.implement({
	fupdate: function(update, options){
		this.get('fupdate', update, options).send();
		return this;
	}
});

String.implement({
	dedupeQs: function(){
		var result = $H({});
		this.split("&").each(function(pair){
			var n = pair.indexOf("=");
			result.include(unescape(pair.substring(0, n)), unescape(pair.substring(n+1, pair.length)));
		});
		return result.toQueryString();
	}
});

/*
Script: Fupdate.Append.js
	Handles the basic functionality of submitting a form and updating a dom element with the result. 
	The result is appended to the DOM element instead of replacing its contents.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
Fupdate.Append = new Class({
	Extends: Fupdate,
	options: {
		//onBeforeEffect: $empty,
		useReveal: true,
		revealOptions: {},
		inject: 'bottom'
	},
	makeRequest: function(){
		this.request = new Request.HTML($merge({
				url: $(this).get('action'),
				waiterTarget: $(this)
		}, this.options.requestOptions)).addEvents({
			success: function(tree, elements, html, javascript){
				var container = new Element('div').set('html', html).hide();
				container.inject(this.update, this.options.inject);
				if (this.options.useReveal) {
					this.fireEvent('beforeEffect', container);
					container.set('reveal', this.options.revealOptions).reveal().get('reveal').chain(function(){
						this.fireEvent('success', [container, this.update, tree, elements, html, javascript]);
					}.bind(this));
				} else {
					container.show();
					this.fireEvent('success', [container, this.update, tree, elements, html]);
				}
			}.bind(this),
			failure: function(xhr){
				this.fireEvent('failure', xhr);
			}.bind(this)
		});
	}
});

/*
Script: Fupdate.Prompt.js
	Prompts the user with the contents of a form (retrieved via ajax) and updates a DOM element with the result of the submission.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
(function(){
	
	var prompter = function(ext){
		return {
			Extends: ext,
			options: {
				//onUpdate: $empty,
				stickyWinToUse: StickyWin.Fx.Modal,
				stickyWinOptions: {},
				useUi: true,
				stickyWinUiOptions: {
					width: 500
				},
				useWaiter: true
			},
			initialize: function(form, update, options){
				this.setOptions(options);
				this.update = $(update);
				this.makeStickyWin(form);
				this.swin.addEvent('close', function(){
					if(this.request && this.request.waiter) this.request.waiter.stop();
				});
				this.addEvent('success', this.hide.bind(this));
			},
			makeStickyWin: function(form){
				if ($(form)) form = $(form);
				this.swin = new this.options.stickyWinToUse({
					content: this.options.useUi?StickyWin.ui('Update Info', form, this.options.stickyWinUiOptions):form,
					showNow: false
				});
				this.element = this.swin.win.getElement('form');
				this.initAfterUpdate();
			},
			hide: function(){
				this.swin.hide();
				return this;
			},
			prompt: function(){
				this.swin.show();
				return this;
			},
			initAfterUpdate: function(){
				this.setOptions({
					requestOptions: {
						useWaiter: this.options.useWaiter,
						waiterTarget: $(this),
						waiterOptions: {
							layer: {
								styles: {
									zIndex: 10001
								}
							}
						}
					}
				});
				this.makeRequest();
				this.addFormEvent();
				$(this).store('fupdate', this);
			}
		};
	};
	
	Fupdate.Prompt = new Class(prompter(Fupdate));
	if(Fupdate.Append) Fupdate.Append.Prompt = new Class(prompter(Fupdate.Append));
	
	
	var ajaxPrompter = function(ext) {
		return {
			Extends: ext,
			options: {
				stickyWinToUse: StickyWin.Fx.Modal.Ajax
			},
			makeStickyWin: function(formUrl){
				if (this.swin) return this.swin;
				this.swin = new this.options.stickyWinToUse($merge({
					showNow: false,
					requestOptions: this.options.requestOptions,
					onHide: function(){
						this.win.empty();
					},
					url: formUrl,
					handleResponse: function(response) {
						var responseScript = "";
						this.swin.Request.response.text.stripScripts(function(script){	responseScript += script; });
						var content = StickyWin.ui('Update Info', response, this.options.stickyWinUiOptions);
						this.swin.setContent(content);
						this.element = this.swin.win.getElement('form');
						this.initAfterUpdate();
						this.swin.show();
						if (this.options.requestOptions.evalScripts) $exec(responseScript);
					}.bind(this)
				}, this.options.stickyWinOptions));
				return this.swin;
			},
			prompt: function(){
				this.makeStickyWin().update();
				return this;
			}
		};
	};

	Fupdate.AjaxPrompt = new Class(ajaxPrompter(Fupdate.Prompt));
	if (Fupdate.Append) Fupdate.Append.AjaxPrompt = new Class(ajaxPrompter(Fupdate.Append.Prompt));
})();

/*
Script: InputFocus.js
	Adds a focused css class to inputs when they have focus.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var InputFocus = new Class({
	Implements: [Options, Occlude, ToElement],
	Binds: ['focus', 'blur'],
	options: {
		focusedClass: 'focused'
	},
	initialize: function(input, options) {
		this.element = $(input);
		if (this.occlude('focuser')) return this.occluded;
		this.setOptions(options);
		this.element.addEvents({
			focus: this.focus,
			blur: this.blur
		});
	},
	focus: function(){
		$(this).addClass(this.options.focusedClass);
	},
	blur: function(){
		$(this).removeClass(this.options.focusedClass);
	}
});

/*
Script: OverText.js
	Shows text over an input that disappears when the user clicks into it. The text remains hidden if the user adds a value.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var OverText = new Class({
	Implements: [Options, Events],
	options: {
//	textOverride: null,
		positionOptions: {
			position:"upperLeft",
			edge:"upperLeft",
			offset: {
				x: 4,
				y: 2
			}
		},
		poll: false,
		pollInterval: 250
//	onTextHide: $empty,
//	onTextShow: $empty
	},
	overTxtEls: [],
	initialize: function(inputs, options) {
		this.setOptions(options);
		$G(inputs).each(this.addElement, this);
		OverText.instances.push(this);
		if (this.options.poll) this.poll();
	},
	addElement: function(el){
		if (el.retrieve('OverText')) return;
		var val = this.options.textOverride || el.get('alt') || el.get('title');
		if (!val) return;
		this.overTxtEls.push(el);
		var txt = new Element('div', {
		  'class': 'overTxtDiv',
			styles: {
				lineHeight: 'normal',
				position: 'absolute'
			},
		  html: val,
		  events: {
		    click: this.hideTxt.pass([el, true], this)
		  }
		}).inject(el, 'after');
		el.addEvents({
			focus: this.hideTxt.pass([el, true], this),
			blur: this.testOverTxt.pass(el, this),
			change: this.testOverTxt.pass(el, this)
		}).store('OverTextDiv', txt).store('OverText', this);
		window.addEvent('resize', this.repositionAll.bind(this));
		this.testOverTxt(el);
		this.repositionOverTxt(el);
	},
	startPolling: function(){
		this.pollingPaused = false;
		return this.poll();
	},
	poll: function(stop) {
		//start immediately
		//pause on focus
		//resumeon blur
		if (this.poller && !stop) return this;
		var test = function(){
			if (this.pollingPaused == true) return;
			this.overTxtEls.each(function(el){
				if (el.retrieve('ot_paused')) return;
				this.testOverTxt(el);
			}, this);
		}.bind(this);
		if (stop) $clear(this.poller);
		else this.poller = test.periodical(this.options.pollInterval, this);
		return this;
	},
	stopPolling: function(){
		this.pollingPaused = true;
		return this.poll(true);
	},
	hideTxt: function(el, focus){
		var txt = el.retrieve('OverTextDiv');
		if (txt && txt.isVisible() && !el.get('disabled')) {
			txt.hide(); 
			try {
				if (focus) el.fireEvent('focus').focus();
			} catch(e){} //IE barfs if you call focus on hidden elements
			this.fireEvent('onTextHide', [txt, el]);
			el.store('ot_paused', true);
		}
		return this;
	},
	showTxt: function(el){
		var txt = el.retrieve('OverTextDiv');
		if (txt && !txt.isVisible()) {
			txt.show();
			this.fireEvent('onTextShow', [txt, el]);
			el.store('ot_paused', false);
		}
		return this;
	},
	testOverTxt: function(el){
		if (el.get('value')) this.hideTxt(el);
		else this.showTxt(el);	
	},
	repositionAll: function(){
		this.overTxtEls.each(this.repositionOverTxt.bind(this));
		return this;
	},
	repositionOverTxt: function (el){
		if (!el) return;
		try {
			var txt = el.retrieve('OverTextDiv');
			if (!txt || !el.getParent()) return;
			this.testOverTxt(el);
			txt.setPosition($merge(this.options.positionOptions, {relativeTo: el}));
			if (el.offsetHeight) this.testOverTxt(el);
			else this.hideTxt(el);
		} catch(e){
			dbug.log('overTxt error: ', e);
		}
		return this;
	}
});
OverText.instances = [];
OverText.update = function(){
	return OverText.instances.map(function(ot){
		return ot.repositionAll();
	});
};

/*
Script: ProductPicker.js
	Allows the user to pick a product from a data source.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var Picklet = new Class({
	Implements: [Options, Events],
/*	options: {
		onShow: $empty
	}, */
	inputElements : {},
	initialize: function(className, options){
		this.setOptions(options);
		this.className = className;
		this.getQuery = this.options.getQuery;
	}
});
var ProductPicker = new Class({
	Implements: [Options, Events, StyleWriter],
	options: {
//	onShow: $empty,
//	onPick: $empty,
		title: 'Product picker',
		showOnFocus: true,
		additionalShowLinks: [],
		stickyWinToUse: StickyWin.Fx,
		stickyWinOptions: {
			fadeDuration: 200,
			draggable: true,
			width: 450
		},
		moveIntoView: true,
		baseHref: 'http://www.cnet.com/html/rb/assets/global/Picker',
		css: "div.productPickerProductDiv div.results { overflow: 'auto'; width: 100%; margin-top: 4px }"+
			"div.productPickerProductDiv select { margin: 4px 0px 4px 0px}"+
			"div.pickerPreview div.sliderContent img {border: 1px solid #000}"+
			"div.pickerPreview div.sliderContent a {color: #0d63a0}" + 
			"div.productPickerProductDiv * {color: #000}" +
			".tool-tip { color: #fff; width: 172px; z-index: 13000; }" +
			".tool-title { font: Verdana, Arial, Helvetica, sans-serif; font-weight: bold; font-size: 11px; margin: 0; padding: 8px 8px 4px; background: url(%tipsArt%/bubble.png) top left !important; background: url(%tipsArt%/bubble.gif) top left; }" +
			".tool-text {font-size: 11px; margin: 0px; padding: 4px 8px 8px; background: url(%tipsArt%/bubble.png) bottom right !important; background: url(%tipsArt%/bubble.gif) bottom right; }"
	},
	initialize: function(input, picklets, options){
		this.setOptions(options);
		this.writeCss();
		this.input = $(input);
		if (!this.input) return;
		this.picklets = picklets;
		this.setUpObservers();
	},
	writeCss: function(){
		var art = this.options.baseHref;
		var tipsArt = art.replace("Picker", "tips");
		this.createStyle(this.options.css.replace("%tipsArt%", tipsArt, "g"), 'pickerStyles');
	},
	getPickletList: function(){
		if(this.picklets.length>1) {
			var selector = new Element('select').setStyle('width', 399);
			this.picklets.each(function(picklet, index){
				var opt = new Element('option').set('value',index);
				opt.text = picklet.options.descriptiveName;
				selector.adopt(opt);
			}, this);
			selector.addEvent('change', function(){
				this.showForm(this.picklets[selector.getSelected()[0].get('value')]);
				this.focusInput(true);
			}.bind(this));
			return selector;
		} else return false;
	},
	buildPicker: function(picklet){
		var contents = new Element('div');
		this.formBody = new Element('div');
		this.pickletList = this.getPickletList();
		if(this.pickletList) contents.adopt(this.pickletList);
		contents.adopt(this.formBody);
		var body = StickyWin.ui(this.options.title, contents, {
			width: this.options.stickyWinOptions.width,
			closeTxt: 'close'
		}).addClass('productPickerProductDiv');
		this.showForm();
		return body;
	},
	showForm: function(picklet){
		this.form = this.makeSearchForm(picklet || this.picklets[0]);
		this.formBody.empty().adopt(this.form);
		(picklet || this.picklets[0]).fireEvent('onShow');
		this.results = new Element('div').addClass('results');
		this.formBody.adopt(this.results);
		this.sliderFx = null;
		this.fireEvent("onShow");
	},
	makeSlider: function(){
		var png = (Browser.Engine.trident)?'gif':'png';
		this.slider = new Element('div', {
			styles: {
				background:'url('+this.options.baseHref+'/slider.'+png+') top right no-repeat',
				display: 'none',
				height:250,
				left:this.options.stickyWinOptions.width - 11,
				position:'absolute',
				top:25,
				width:0,
				overflow: 'hidden'
			}
		}).addClass('pickerPreview').inject(this.swin.win).addEvents({
			mouseover: function(){
				this.previewHover = true;
			}.bind(this),
			mouseout: function(){
				this.previewHover = false;
				(function(){if (!this.previewHover) this.hidePreview()}).delay(400, this);
			}.bind(this)
		});
		this.sliderContent = new Element('div', {
			styles: {
				width: 130,
				height: 200,
				padding: 10,
				margin: '10px 10px 0px 0px',
				overflow: 'auto',
				cssFloat: 'right'
			}
		}).inject(this.slider).addClass('sliderContent');
	},
	makeSearchForm: function(picklet){
		this.currentPicklet = picklet;
		var formTable = new Element('table', {
			styles: {
				width: "100%",
				cellpadding: '0',
				cellspacing: '0'
			}
		});
		var tBody = new Element('tbody').inject(formTable);
		var form = new Element('form').addEvent('submit', function(e){
			this.getResults(e.target, picklet);
		}.bind(this)).adopt(formTable).set('action','javascript:void(0);');
		$each(picklet.options.inputs, function(val, name){
			var ins = this.getSearchInputTr(val, name);
			tBody.adopt(ins.holder);
			picklet.inputElements[name] = ins.input;
		}, this);
		return form;
	},
	getSearchInputTr: function(val, name){
		try{
			var style = ($type(val.style))?val.style:{};
			//create the input object
			//this is I.E. hackery, because IE does not let you set the name of a DOM element.
			//thanks MSFT.
			var input = (Browser.Engine.trident)?new Element('<' + val.tagName + ' name="' + name + '" />'):
					new Element(val.tagName, {name: name});
			input.setStyles(style);
			if(val.type)input.set('type', val.type);
			if(val.tip && Tips){
				input.set('title', val.tip);
				new Tips([input], {
					onShow: function(tip){
						this.shown = true;
						(function(){
              if(!this.shown) return;
              $(tip).setStyles({ display:'block', opacity: 0 });
              new Fx.Tween(tip, {property: 'opacity', duration: 300 }).start(0,.9);
            }).delay(500, this);
          },
          onHide: function(tip){
            tip.setStyle('visibility', 'hidden');
            this.shown = false;
          }
        });
      }
      if(val.tagName == "select"){
        val.value.each(function(option, index){
          var opt = new Element('option',{ value: option });
          opt.text = (val.optionNames && val.optionNames[index])?$pick(val.optionNames[index], option):option;
          input.adopt(opt);
        });
      } else {
				input.set('value', $pick(val.value,""));
			}
      var holder = new Element('tr');
			var colspan=0;
			if (val.instructions) holder.adopt(new Element('td').set('html', val.instructions));
			else colspan=2;
			var inputTD = new Element('td').adopt(input);
			if (colspan) inputTD.set('colspan', colspan);
			holder.adopt(inputTD);
			return { holder : holder, input : input};
		}catch(e){dbug.log(e); return false;}
	},
	getResults: function(form, picklet){
		if(form.get('tag') != "form") form = $$('form').filter(function(fm){ return fm.hasChild(form) })[0];
		if(!form) {
			dbug.log('error computing form');
			return null;
		}
		var query = picklet.getQuery(unescape(form.toQueryString()).parseQuery());
		query.addEvent('onComplete', this.showResults.bind(this));
		query.request();
		return this;
	},
	showResults: function(data){
		var empty = false;
		if(this.results.innerHTML=='') {
			empty = true;
			this.results.setStyles({
				height: 0,
				border: '1px solid #666',
				padding: 0,
				overflow: 'auto',
				opacity: 0
			});
		} else this.results.empty();
		this.items = this.currentPicklet.options.resultsList(data);
		if(this.items && this.items.length > 0) {
			this.items.each(function(item, index){
				var name = this.currentPicklet.options.listItemName(item);
				var value = this.currentPicklet.options.listItemValue(item);
				this.results.adopt(this.makeProductListEntry(name, value, index));
			}, this);
		} else {
			this.results.set('html', "Sorry, there don't seem to be any items for that search");
		}
		this.results.morph({ height: 200, opacity: 1 });
		this.listStyles();
		this.getOnScreen.delay(500, this);
	},
	getOnScreen: function(){
		if(document.compatMode == "BackCompat") return;
		var s = this.swin.win.getCoordinates();
		if(s.top < window.getScroll().y) {
			this.swin.win.tween('top', window.getScroll().y+50);
			return;
		}
		if(s.top+s.height > window.getScroll().y+window.getSize().y && window.getSize().y>s.height) {
			this.swin.win.tween('top', window.getScroll().y+window.getSize().y-s.height-100);
			return;
		}
		try{this.swin.shim.show.delay(500, this.swin.shim);}catch(e){}
		return;
	},
	listStyles: function(){
		var defaultStyle = {
			cursor: 'pointer',
			borderBottom: '1px solid #ddd',
			padding: '2px 8px 2px 8px',
			backgroundColor:'#fff',
			color: '#000',
			fontWeight: 'normal'
		};
		var hoverStyle = {
			backgroundColor:'#fcfbd1',
			color: '#d56a00'
		};
		var selectedStyle = $extend(defaultStyle, {
			color: '#D00000',
			fontWeight: 'bold',
			backgroundColor: '#eee'
		});
		this.results.getElements('div.productPickerProductDiv').each(function(p){
			var useStyle = (this.input.value.toInt() == p.get('val').toInt())?selectedStyle:defaultStyle;
			p.setStyles(useStyle);
			if(!Browser.Engine.trident) {//ie doesn't like these mouseover behaviors...
				p.addEvent('mouseover', function(){ p.setStyles(hoverStyle); }.bind(this));
				p.addEvent('mouseout', function(){ p.setStyles(useStyle); });
			}
		}, this);
	},
	makeProductListEntry: function(name, value, index){
		var pDiv = new Element("div").addClass('productPickerProductDiv').adopt(
				new Element("div").set('html', name)
			).set('val', value);
		pDiv.addEvent('mouseenter', function(e){
			this.preview = true;
			this.sliderContent.empty();
			var content = this.getPreview(index);
			if($type(content)=="string") this.sliderContent.set('html', content);
			else if($(content)) this.sliderContent.adopt(content);
			this.showPreview.delay(200, this);
		}.bind(this));
		pDiv.addEvent('mouseleave', function(e){
			this.preview = false;
			(function(){if(!this.previewHover) this.hidePreview();}).delay(400, this);
		}.bind(this));
		pDiv.addEvent('click', function(){
			this.currentPicklet.options.updateInput(this.input, this.items[index]);
			this.fireEvent('onPick', [this.input, this.items[index], this]);
			this.hide();
			this.listStyles.delay(200, this);
		}.bind(this));
		return pDiv;
	},
	makeStickyWin: function(){
		if(document.compatMode == "BackCompat") this.options.stickyWinOptions.relativeTo = this.input;
		this.swin = new this.options.stickyWinToUse($merge(this.options.stickyWinOptions, {
			draggable: true,
			content: this.buildPicker()
		}));
	},
	focusInput: function(force){
		if ((!this.focused || $pick(force,false)) && this.form.getElement('input')) {
			this.focused = true;
			try { this.form.getElement('input').focus(); } catch(e){}
		}
	},
	show: function(){
		if (!this.swin) this.makeStickyWin();
		if (!this.slider) this.makeSlider();
		if (!this.swin.visible) this.swin.show();
		this.focusInput();
		return this;
	},
	hide: function(){
		$$('.tool-tip').hide();
		this.swin.hide();
		this.focused = false;
		return this;
	},
	setUpObservers: function(){
		try {
			if(this.options.showOnFocus) this.input.addEvent('focus', this.show.bind(this));
			if(this.options.additionalShowLinks.length>0) {
				this.options.additionalShowLinks.each(function(lnk){
					$(lnk).addEvent('click', this.show.bind(this));
				}, this);
			}
		}catch(e){dbug.log(e);}
	},
	showPreview: function(index){
		width = this.currentPicklet.options.previewWidth || 150;
		this.sliderContent.setStyle('width', (width-30));
		if(!this.sliderFx) this.sliderFx = new Fx.Elements([this.slider, this.swin.win]);
		this.sliderFx.clearChain();
		this.sliderFx.setOptions({
			duration: 1000, 
			transition: 'elastic:out'
		});
		if(this.preview && this.slider.getStyle('width').toInt() < width-5) {
			this.slider.show('block');
			this.sliderFx.start({
				'0':{//the slider
					'width':width
				},
				'1':{//the popup window (for ie)
					'width':width+this.options.stickyWinUiOptions.width
				}
			});
		}
	},
	hidePreview: function(){
		if(!this.preview) {
			this.sliderFx.setOptions({
				duration: 250, 
				transition: 'back:in'
			});
			this.sliderFx.clearChain();
			this.sliderFx.start({
				'0':{//the slider
					'width':[this.slider.getStyle('width').toInt(),0]
				},
				'1':{//the popup window (for ie)
					'width':[this.swin.win.getStyle('width').toInt(), this.options.stickyWinUiOptions]
				}
			}).chain(function(){
				this.slider.hide();
			}.bind(this));
		}
	},
	getPreview: function(index){
		return this.currentPicklet.options.previewHtml(this.items[index]);
	}
});


$extend(ProductPicker, {
	picklets: [],
	add: function(picklet){
		if(! picklet.className) {
			dbug.log('error: cannot add Picklet %o; missing className: %s', picklet, picklet.className);
			return;
		}
		this.picklets[picklet.className] = picklet;
	},
	addAllThese: function(picklets){
		picklets.each(function(picklet){
			this.add(picklet);
		}, this);
	},
	getPicklet: function(className){
		return ProductPicker.picklets[className]||false;
	}
});

var FormPickers = new Class({
	Implements: [Options],
	options: {
		inputs: 'input',
		additionalShowLinkClass: 'openPicker',
		pickletOptions: {}
	},
	initialize: function(form, options){
		this.setOptions(options);
		this.form = $(form);
		this.inputs = this.form.getElements(this.options.inputs);
		this.setUpInputs();
	},
	setUpInputs: function(inputs){
		inputs = $pick(inputs, this.inputs);
		inputs.each(this.addPickers.bind(this));
	},
	addPickers: function(input){
		var picklets = [];
		input.className.split(" ").each(function(clss){
			if(ProductPicker.getPicklet(clss)) picklets.push(ProductPicker.getPicklet(clss));
		}, this);
		if(input.getNext() && input.getNext().hasClass(this.options.additionalShowLinkClass))
			this.options.pickletOptions.additionalShowLinks = [input.getNext()];
		if(picklets.length>0)	new ProductPicker(input, picklets, this.options.pickletOptions);
	}
});

/*
Script: SimpleEditor.js
	A simple html editor for wrapping text with links and whatnot.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var SimpleEditor = new Class({
	Implements: [ToElement, Occlude],
	property: 'SimpleEditor',
	initialize: function(input, buttons, commands){
		this.element = $(input);
		if (this.occlude()) return this.occludes;
		this.commands = new Hash($extend(SimpleEditor.commands, commands||{}));
		this.buttons = $$(buttons);
		this.buttons.each(function(button){
			button.addEvent('click', function() {
				this.exec(button.get('rel'));
			}.bind(this));
		}.bind(this));
		$(this).addEvent('keydown', function(e){
			if (e.control||e.meta) {
				var key = this.shortCutToKey(e.key, e.shift);
				if(key) {
					e.stop();
					this.exec(key);
				}
			}
		}.bind(this));
	},
	shortCutToKey: function(shortcut, shift){
		var returnKey = false;
		this.commands.each(function(value, key){
			var char = (value.shortcut ? value.shortcut.toLowerCase() : value.shortcut);
			if(value.shortcut == shortcut || (shift && char == shortcut)) returnKey = key;
		});
		return returnKey;
	},
	addCommand: function(key, command, shortcut){
		this.commands.set(key, {
			command: command,
			shortcut: shortcut
		});
	},
	addCommands: function(commands){
		this.commands.extend(commands);
	},
	exec: function(key){
		var currentScrollPos; 
		if ($(this).scrollTop || $(this).scrollLeft) {
			currentScrollPos = {
				scrollTop: $(this).getScroll().y,
				scrollLeft: $(this).getScroll().x
			};
		}
		if(this.commands.has(key)) this.commands.get(key).command($(this));
		if(currentScrollPos) {
			$(this).set('scrollTop', currentScrollPos.getScroll().y);
			$(this).set('scrollLeft', currentScrollPos.getScroll().x);
		}
	}
});
$extend(SimpleEditor, {
	commands: {},
	addCommand: function(key, command, shortcut){
		SimpleEditor.commands[key] = {
			command: command,
			shortcut: shortcut
		}
	},
	addCommands: function(commands){
		$extend(SimpleEditor.commands, commands);
	}
});
SimpleEditor.addCommands({
	bold: {
		shortcut: 'b',
		command: function(input){
			input.insertAroundCursor({before:'<b>',after:'</b>'});
		}
	},
	underline: {
		shortcut: 'u',
		command: function(input){
			input.insertAroundCursor({before:'<u>',after:'</u>'});
		}
	},
	anchor: {
		shortcut: 'l',
		command: function(input){
			if(window.TagMaker){
				if(!this.linkBuilder) this.linkBuilder = new TagMaker.anchor();
				this.linkBuilder.prompt(input);
			} else {
				var href = window.prompt(SimpleEditor.getMsg('linkURL'));
				var opts = {before: '<a href="'+href+'">', after:'</a>'};
				if (!input.getSelectedText()) opts.defaultMiddle = window.prompt(SimpleEditor.getMsg('linkText'));
				input.insertAroundCursor(opts);
			}
		}
	},
	copy: {
		shortcut: false,
		command: function(input){
			if(Clipboard) Clipboard.copyFromElement(input);
			else simpleErrorPopup(SimpleEditor.getMsg('woops'), SimpleEditor.getMsg('nopeCtrlC'));
			input.focus();
		}
	},
	cut: {
		shortcut: false,
		command: function(input){
			if(Clipboard) {
				Clipboard.copyFromElement(input);
				input.insertAtCursor('');
			} else simpleErrorPopup(SimpleEditor.getMsg('woops'), SimpleEditor.getMsg('nopeCtrlX'));
		}
	},
	hr: {
		shortcut: '-',
		command: function(input){
			input.insertAtCursor('\n<hr/>\n');
		}
	},
	img: {
		shortcut: 'g',
		command: function(input){
			if(window.TagMaker) {
				if(!this.anchorBuilder) this.anchorBuilder = new TagMaker.image();
				this.anchorBuilder.prompt(input);
			} else {
				var href = window.prompt(SimpleEditor.getMsg('imgURL'));
				var alt = window.prompt(SimpleEditor.getMsg('imgAlt'));
				input.insertAtCursor('<img src="'+href+'" alt="'+alt.replace(/"/g,'')+'" />');
			}
		}
	},
	stripTags: {
		shortcut: '\\',
		command: function(input){
			input.insertAtCursor(input.getSelectedText().stripTags());
		}
	},
	sup: {
		shortcut: false,
		command: function(input){
			input.insertAroundCursor({before:'<sup>', after: '</sup>'});
		}
	},
	sub: {
		shortcut: false,
		command: function(input){
			input.insertAroundCursor({before:'<sub>', after: '</sub>'});
		}
	},
	blockquote: {
		shortcut: false,
		command: function(input){
			input.insertAroundCursor({before:'<blockquote>', after: '</blockquote>'});
		}
	},
	paragraph: {
		shortcut: 'enter',
		command: function(input){
			input.insertAroundCursor({before:'\n<p>\n', after: '\n</p>\n'});
		}
	},
	strike: {
		shortcut: 'k',
		command: function(input){
			input.insertAroundCursor({before:'<strike>',after:'</strike>'});
		}
	},
	italics: {
		shortcut: 'i',
		command: function(input){
			input.insertAroundCursor({before:'<i>',after:'</i>'});
		}
	},
	bullets: {
		shortcut: '8',
		command: function(input){
			input.insertAroundCursor({before:'<ul>\n	<li>',after:'</li>\n</ul>'});
		}
	},
	numberList: {
		shortcut: '=',
		command: function(input){
			input.insertAroundCursor({before:'<ol>\n	<li>',after:'</li>\n</ol>'});
		}
	},
	clean: {
		shortcut: false,
		command: function(input){
			input.tidy();
		}
	},
	preview: {
		shortcut: false,
		command: function(input){
			try {
				if(!this.container){
					this.container = new Element('div', {
						styles: {
							border: '1px solid black',
							padding: 8,
							height: 300,
							overflow: 'auto'
						}
					});
					this.preview = new StickyWin.Modal({
						content: StickyWin.ui("preview", this.container, {
							width: 600,
							buttons: [{
								text: 'close',
								onClick: function(){
									this.container.empty();
								}.bind(this)
							}]
						}),
						showNow: false
					});
				}
				this.container.set('html', input.get('value'));
				this.preview.show();
			} catch(e){dbug.log('you need StickyWin.Modal and StickyWin.ui')}
		}
	}
});

SimpleEditor.resources = {
	enUS: {
		woops:'Woops',
		nopeCtrlC:'Sorry, this function doesn\'t work here; use ctrl+c.',
		nopeCtrlX:'Sorry, this function doesn\'t work here; use ctrl+x.',
		linkURL:'The URL for the link',
		linkText:'The link text',
		imgURL:'The URL to the image',
		imgAlt:'The title (alt) for the image'
	}
};
SimpleEditor.language = "enUS";
SimpleEditor.getMsg = function(key, language){
	return SimpleEditor.resources[language||SimpleEditor.language][key];
};

/*
Script: TagMaker.js
	Prompts the user to fill in the gaps to create an html tag output.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
var TagMaker = new Class({
	Implements: [Options, Events, StyleWriter],
	options: {
		name: "Tag Builder",
		output: '',
		picklets: {},
		help: {},
		example: {},
		'class': {},
		selectLists: {},
		width: 400,
		maxHeight: 500,
		clearOnPrompt: true,
		baseHref: "http://www.cnet.com/html/rb/assets/global/tips", 
		css: "table.trinket {	width: 98%;	margin: 0px auto;	font-size: 10px; }"+
					"table.trinket td {	vertical-align: top;	padding: 4px;}"+
					"table.trinket td a.button {	position: relative;	top: -2px;}"+
					"table.trinket td.example {	font-size: 9px;	color: #666;	text-align: right;	border-bottom: 1px solid #ddd;"+
						"padding-bottom: 6px;}"+
					"table.trinket div.validation-advice {	background-color: #a36565;	font-weight: bold;	color: #fff;	padding: 4px;"+
						"margin-top: 3px;}"+
					"table.trinket input.text {width: 100%;}"+
					".tagMakerTipElement { 	cursor: help; }"+
					".tagMaker .tip {	color: #fff;	width: 172px;	z-index: 13000; }"+
					".tagMaker .tip-title {	font-weight: bold;	font-size: 11px;	margin: 0;	padding: 8px 8px 4px;"+
							"background: url(%baseHref%/bubble.png) top left;}"+
					".tagMaker .tip-text { font-size: 11px; 	padding: 4px 8px 8px; "+
							"background: url(%baseHref%/bubble.png) bottom right; }"+
					".tagMaker { z-index:10001 }"
//	onPrompt: $empty,
//	onChoose: $empty
	},
	initialize: function(options){
		this.setOptions(options);
		this.buttons = [
			{
				text: 'Copy',
				onClick: this.copyToClipboard.bind(this),
				properties: {
					'class': 'closeSticky tip',
					title: 'Copy::Copy the html to your OS clipboard (like hitting Ctrl+C)'
				}
			},
			{
				text: 'Paste',
				onClick: function(){
					if(this.validator.validate()) this.insert();
				}.bind(this),
				properties: {
					'class': 'tip',
					title: 'Paste::Insert the html into the field you are editing'
				}
			},
			{
				text: 'Close',
				properties: {
					'class': 'closeSticky tip',
					title: 'Close::Close this popup'
				}
			}
		];
		this.createStyle(this.options.css.replace("%baseHref%", this.options.baseHref, "g"), 'defaultTagBuilderStyle');
	},
	prompt: function(target){
		this.target = $(target);
		var content = this.getContent();
		if (this.options.clearOnPrompt) this.clear();
		if(content) {
				var relativeTo = (document.compatMode == "BackCompat" && this.target)?this.target:document.body;
				if(!this.win) {
					this.win = new StickyWin.Fx({
						content: content,
						draggable: true,
						relativeTo: relativeTo,
						onClose: function(){
							$$('.tagMaker-tip').hide();
						}
					});
				}
				if(!this.win.visible) this.win.show();
		}
		var innerText = this.getInnerTextInput();
		this.range = target.getSelectedRange();
		if(innerText) innerText.set('value', target.getTextInRange(this.range.start, this.range.end)||"");
		return this.fireEvent('onPrompt');
	},
	clear: function(){
		this.body.getElements('input').each(function(input){
			input.erase('value');
		});
	},
	getKeys: function(text) {
		return text.split('%').filter(function(inputKey, index){
				return index%2;
		});
	},
	getInnerTextInput: function(){
		return this.body.getElement('input[name=Inner-Text]');
	},
	getContent: function(){
		var opt = this.options; //save some bytes
		if(!this.form) { //if the body hasn't been created, create it
			this.form = new Element('form');
				var table = new HtmlTable({properties: {'class':'trinket'}});
				this.getKeys(opt.output).each(function(inputKey) {
					if(this.options.selectLists[inputKey]){
						var input = new Element('select').setProperties({
							name: inputKey.replace(' ', '-', 'g')
						}).addEvent('change', this.createOutput.bind(this));
						this.options.selectLists[inputKey].each(function(opt){
							var option = new Element('option').inject(input);
							if(opt.selected) option.set('selected', true);
							option.set('value', opt.value);
							option.set('text', opt.key);
						}, this);
						table.push([inputKey, input]);
					} else {
						var input = new Element('input', {
							type: 'text',
							name: inputKey.replace(/ /g, '-'),
							title: inputKey+'::'+opt.help[inputKey],
							'class': 'text tip ' + ((opt['class'])?opt['class'][inputKey]||'':''),
							events: {
								keyup: this.createOutput.bind(this),
								focus: function(){this.select()},
								change: this.createOutput.bind(this)
							}
						});
						if(opt.picklets[inputKey]) {
							var a = new Element('a').addClass('button').set('html', 'choose');
							var div = new Element('div').adopt(input.setStyle('width',160)).adopt(a);
							var picklets = ($type(opt.picklets[inputKey]) == "array")?opt.picklets[inputKey]:[opt.picklets[inputKey]];
							new ProductPicker(input, picklets, {
								showOnFocus: false, 
								additionalShowLinks: [a],
								onPick: function(input, data, picker){
									try {
										var ltInput = this.getInnerTextInput();
										if(ltInput && !ltInput.get('value')) {
											try {
												ltInput.set('value', picker.currentPicklet.options.listItemName(data));
											}catch (e){dbug.log('set value error: ', e);}
										}
										var val = input.value;
										if(inputKey == "Full Path" && val.test(/^http:/))
												input.set('value', val.substring(val.indexOf('/', 7), val.length));
										this.createOutput();
									} catch(e){dbug.log(e)}
								}.bind(this)
							});
							table.push([inputKey, div]);
						} else table.push([inputKey, input]);
					}
					//[{content: <content>, properties: {colspan: 2, rowspan: 3, 'class': "cssClass", style: "border: 1px solid blue"}]
					if(this.options.example[inputKey]) 
						table.push([{content: 'eg. '+this.options.example[inputKey], properties: {colspan: 2, 'class': 'example'}}]);
				}, this);
				this.resultInput = new Element('input', {
						type: 'text',
						title: 'HTML::This is the resulting tag html.',
						'class': 'text result tip'
					}).addEvent('focus', function(){this.select()});
				table.push(['HTML', this.resultInput]).tr;

			this.form = table.table;
			this.body = new Element('div', {
				styles: {
					overflow:'auto',
					maxHeight: this.options.maxHeight
				}
			}).adopt(this.form);
			this.validator = new FormValidator(this.form);
			this.validator.insertAdvice = function(advice, field){
				var p = $(field.parentNode);
				if(p) p.adopt(advice);
			};
		}

		if(!this.content) {
			this.content = StickyWin.ui(this.options.name, this.body, {
				buttons: this.buttons,
				width: this.options.width.toInt()
			});
			new Tips(this.content.getElements('.tip'), {
				showDelay: 700,
				maxTitleChars: 50, 
				maxOpacity: .9,
				className: 'tagMaker'
			});
		}
		return this.content;

	},
	createOutput: function(){
		var inputs = this.form.getElements('input, select');
		var html = this.options.output;
		inputs.each(function(input) {
			if(!input.hasClass('result')) {
				html = html.replace(new RegExp('%'+input.get('name').replace('-', ' ', 'g').toLowerCase()+'%', 'ig'),
					(input.get('tag')=='select'?input.getSelected()[0]:input).get('value'));
				html = html.replace(/\s\w+\=""/g, "");
			}
		});
		return this.resultInput.value = html;
	},
	copyToClipboard: function(){
		var inputs = this.form.getElements('input');
		var result = inputs[inputs.length-1];
		result.select();
		Clipboard.copyFromElement(result);
		$$('.tagMaker-tip').hide();
		this.win.hide();
		this.fireEvent('onChoose');
	},
	insert: function(){
		if(!this.target) {
			simpleErrorPopup('Cannot Paste','This tag builder was not launched with a target input specified; you\'ll have to copy the tag yourself. Sorry!');
			return;
		}
		var value = (this.target)?this.target.value:this.target;
		var output = this.body.getElement("input.result");
		
		var currentScrollPos; 
		if (this.target.scrollTop || this.target.scrollLeft) {
			currentScrollPos = {
				scrollTop: this.target.getScroll().y,
				scrollLeft: this.target.getScroll().x
			};
		}
		this.target.value = value.substring(0, this.range.start) + output.value + value.substring((this.range.end-this.range.start) + this.range.start, value.length);
		if(currentScrollPos) {
			this.target.scrollTop = currentScrollPos.getScroll().y;
			this.target.scrollLeft = currentScrollPos.getScroll().x;
		}

		this.target.selectRange(this.range.start, output.value.length + this.range.start);
		this.fireEvent('onChoose');
		$$('.tagMaker-tip').hide();
		this.win.hide();
		return;
	}
});


TagMaker.image = new Class({
	Extends: TagMaker,
	options: {
		name: "Image Builder",
		output: '<img src="%Full Url%" width="%Width%" height="%Height%" alt="%Alt Text%" style="%Alignment%"/>',
		help: {
			'Full Url':'Enter the external URL (http://...) to the image',
			'Width':'Enter the width in pixels.',
			'Height':'Enter the height in pixels.',
			'Alt Text':'Enter the alternate text for the image.',
			'Alignment':'Choose how to float the image.'
		},
		example: {
			'Full Url':'http://i.i.com.com/cnwk.1d/i/hdft/redball.gif'
		},
		'class': {
			'Full Url':'validate-url required',
			'Width':'validate-digits',
			'Height':'validate-digits',
			'Alt Text':''
		},
		selectLists: {
			Alignment: [
				{
					key: 'left',
					value: 'float: left'
				},
				{
					key: 'right',
					value: 'float: right'
				},
				{
					key: 'none',
					value: 'float: none',
					selected: true
				},
				{
					key: 'center',
					value: 'margin-left: auto; margin-right: auto;'
				}
			]		
		}
	}
});

TagMaker.anchor = new Class({
	Extends: TagMaker,
	options: {
		name: "Anchor Builder",
		output: '<a href="%Full Url%">%Inner Text%</a>',
		help: {
			'Full Url':'Enter the external URL (http://...)',
			'Inner Text':'Enter the text for the link body'
		},
		example: {
			'Full Url':'http://www.microsoft.com',
			'Inner Text':'Microsoft'
		},
		'class': {
			'Full Url':'validate-url'
		}
	}
});

/**
 * Autocompleter
 *
 * @version		1.1.1
 *
 * @todo: Caching, no-result handling!
 *
 *
 * @license		MIT-style license
 * @author		Harald Kirschner <mail [at] digitarald.de>
 * @copyright	Author
 */
var Autocompleter = {};

var OverlayFix = IframeShim;

Autocompleter.Base = new Class({
	
	Implements: [Options, Events],
	
	options: {
		minLength: 1,
		markQuery: true,
		width: 'inherit',
		maxChoices: 10,
//		injectChoice: null,
//		customChoices: null,
		className: 'autocompleter-choices',
		zIndex: 42,
		delay: 400,
		observerOptions: {},
		fxOptions: {},
//		onSelection: $empty,
//		onShow: $empty,
//		onHide: $empty,
//		onBlur: $empty,
//		onFocus: $empty,

		autoSubmit: false,
		overflow: false,
		overflowMargin: 25,
		selectFirst: false,
		filter: null,
		filterCase: false,
		filterSubset: false,
		forceSelect: false,
		selectMode: true,
		choicesMatch: null,

		multiple: false,
		separator: ', ',
		separatorSplit: /\s*[,;]\s*/,
		autoTrim: true,
		allowDupes: false,

		cache: true,
		relative: false
	},

	initialize: function(element, options) {
		this.element = $(element);
		this.setOptions(options);
		this.build();
		this.observer = new Observer(this.element, this.prefetch.bind(this), $merge({
			'delay': this.options.delay
		}, this.options.observerOptions));
		this.queryValue = null;
		if (this.options.filter) this.filter = this.options.filter.bind(this);
		var mode = this.options.selectMode;
		this.typeAhead = (mode == 'type-ahead');
		this.selectMode = (mode === true) ? 'selection' : mode;
		this.cached = [];
	},

	/**
	 * build - Initialize DOM
	 *
	 * Builds the html structure for choices and appends the events to the element.
	 * Override this function to modify the html generation.
	 */
	build: function() {
		if ($(this.options.customChoices)) {
			this.choices = this.options.customChoices;
		} else {
			this.choices = new Element('ul', {
				'class': this.options.className,
				'styles': {
					'zIndex': this.options.zIndex
				}
			}).inject(document.body);
			this.relative = false;
			if (this.options.relative || this.element.getOffsetParent() != document.body) {
				this.choices.inject(this.element, 'after');
				this.relative = this.element.getOffsetParent();
			}
			this.fix = new OverlayFix(this.choices);
		}
		if (!this.options.separator.test(this.options.separatorSplit)) {
			this.options.separatorSplit = this.options.separator;
		}
		this.fx = (!this.options.fxOptions) ? null : new Fx.Tween(this.choices, $merge({
			'property': 'opacity',
			'link': 'cancel',
			'duration': 200
		}, this.options.fxOptions)).addEvent('onStart', Chain.prototype.clearChain).set(0);
		this.element.setProperty('autocomplete', 'off')
			.addEvent((Browser.Engine.trident || Browser.Engine.webkit) ? 'keydown' : 'keypress', this.onCommand.bind(this))
			.addEvent('click', this.onCommand.bind(this, [false]))
			.addEvent('focus', this.toggleFocus.create({bind: this, arguments: true, delay: 100}));
			//.addEvent('blur', this.toggleFocus.create({bind: this, arguments: false, delay: 100}));
		document.addEvent('click', function(e){
			if (e.target != this.choices) this.toggleFocus(false);
		}.bind(this));
	},

	destroy: function() {
		if (this.fix) this.fix.dispose();
		this.choices = this.selected = this.choices.destroy();
	},

	toggleFocus: function(state) {
		this.focussed = state;
		if (!state) this.hideChoices(true);
		this.fireEvent((state) ? 'onFocus' : 'onBlur', [this.element]);
	},

	onCommand: function(e) {
		if (!e && this.focussed) return this.prefetch();
		if (e && e.key && !e.shift) {
			switch (e.key) {
				case 'enter':
					if (this.element.value != this.opted) return true;
					if (this.selected && this.visible) {
						this.choiceSelect(this.selected);
						return !!(this.options.autoSubmit);
					}
					break;
				case 'up': case 'down':
					if (!this.prefetch() && this.queryValue !== null) {
						var up = (e.key == 'up');
						this.choiceOver((this.selected || this.choices)[
							(this.selected) ? ((up) ? 'getPrevious' : 'getNext') : ((up) ? 'getLast' : 'getFirst')
						](this.options.choicesMatch), true);
					}
					return false;
				case 'esc': case 'tab':
					this.hideChoices(true);
					break;
			}
		}
		return true;
	},

	setSelection: function(finish) {
		var input = this.selected.inputValue, value = input;
		var start = this.queryValue.length, end = input.length;
		if (input.substr(0, start).toLowerCase() != this.queryValue.toLowerCase()) start = 0;
		if (this.options.multiple) {
			var split = this.options.separatorSplit;
			value = this.element.value;
			start += this.queryIndex;
			end += this.queryIndex;
			var old = value.substr(this.queryIndex).split(split, 1)[0];
			value = value.substr(0, this.queryIndex) + input + value.substr(this.queryIndex + old.length);
			if (finish) {
				var space = /[^\s,]+/;
				var tokens = value.split(this.options.separatorSplit).filter(space.test, space);
				if (!this.options.allowDupes) tokens = [].combine(tokens);
				var sep = this.options.separator;
				value = tokens.join(sep) + sep;
				end = value.length;
			}
		}
		this.observer.setValue(value);
		this.opted = value;
		if (finish || this.selectMode == 'pick') start = end;
		this.element.selectRange(start, end);
		this.fireEvent('onSelection', [this.element, this.selected, value, input]);
	},

	showChoices: function() {
		var match = this.options.choicesMatch, first = this.choices.getFirst(match);
		this.selected = this.selectedValue = null;
		if (this.fix) {
			var pos = this.element.getCoordinates(this.relative), width = this.options.width || 'auto';
			this.choices.setStyles({
				'left': pos.left,
				'top': pos.bottom,
				'width': (width === true || width == 'inherit') ? pos.width : width
			});
		}
		if (!first) return;
		if (!this.visible) {
			this.visible = true;
			this.choices.setStyle('display', '');
			if (this.fx) this.fx.start(1);
			this.fireEvent('onShow', [this.element, this.choices]);
		}
		if (this.options.selectFirst || this.typeAhead || first.inputValue == this.queryValue) this.choiceOver(first, this.typeAhead);
		var items = this.choices.getChildren(match), max = this.options.maxChoices;
		var styles = {'overflowY': 'hidden', 'height': ''};
		this.overflown = false;
		if (items.length > max) {
			var item = items[max - 1];
			styles.overflowY = 'scroll';
			styles.height = item.getCoordinates(this.choices).bottom;
			this.overflown = true;
		};
		this.choices.setStyles(styles);
		this.fix.show();
	},

	hideChoices: function(clear) {
		if (clear) {
			var value = this.element.value;
			if (this.options.forceSelect) value = this.opted;
			if (this.options.autoTrim) {
				value = value.split(this.options.separatorSplit).filter($arguments(0)).join(this.options.separator);
			}
			this.observer.setValue(value);
		}
		if (!this.visible) return;
		this.visible = false;
		this.observer.clear();
		var hide = function(){
			this.choices.setStyle('display', 'none');
			this.fix.hide();
		}.bind(this);
		if (this.fx) this.fx.start(0).chain(hide);
		else hide();
		this.fireEvent('onHide', [this.element, this.choices]);
	},

	prefetch: function() {
		var value = this.element.value, query = value;
		if (this.options.multiple) {
			var split = this.options.separatorSplit;
			var values = value.split(split);
			var index = this.element.getCaretPosition();
			var toIndex = value.substr(0, index).split(split);
			var last = toIndex.length - 1;
			index -= toIndex[last].length;
			query = values[last];
		}
		if (query.length < this.options.minLength) {
			this.hideChoices();
		} else {
			if (query === this.queryValue || (this.visible && query == this.selectedValue)) {
				if (this.visible) return false;
				this.showChoices();
			} else {
				this.queryValue = query;
				this.queryIndex = index;
				if (!this.fetchCached()) this.query();
			}
		}
		return true;
	},

	fetchCached: function() {
		return false;
		if (!this.options.cache
			|| !this.cached
			|| !this.cached.length
			|| this.cached.length >= this.options.maxChoices
			|| this.queryValue) return false;
		this.update(this.filter(this.cached));
		return true;
	},

	update: function(tokens) {
		this.choices.empty();
		this.cached = tokens;
		if (!tokens || !tokens.length) {
			this.hideChoices();
		} else {
			if (this.options.maxChoices < tokens.length && !this.options.overflow) tokens.length = this.options.maxChoices;
			tokens.each(this.options.injectChoice || function(token){
				var choice = new Element('li', {'html': this.markQueryValue(token)});
				choice.inputValue = token;
				this.addChoiceEvents(choice).inject(this.choices);
			}, this);
			this.showChoices();
		}
	},

	choiceOver: function(choice, selection) {
		if (!choice || choice == this.selected) return;
		if (this.selected) this.selected.removeClass('autocompleter-selected');
		this.selected = choice.addClass('autocompleter-selected');
		this.fireEvent('onSelect', [this.element, this.selected, selection]);
		if (!selection) return;
		this.selectedValue = this.selected.inputValue;
		if (this.overflown) {
			var coords = this.selected.getCoordinates(this.choices), margin = this.options.overflowMargin,
				top = this.choices.scrollTop, height = this.choices.offsetHeight, bottom = top + height;
			if (coords.top - margin < top && top) this.choices.scrollTop = Math.max(coords.top - margin, 0);
			else if (coords.bottom + margin > bottom) this.choices.scrollTop = Math.min(coords.bottom - height + margin, bottom);
		}
		if (this.selectMode) this.setSelection();
	},

	choiceSelect: function(choice) {
		if (choice) this.choiceOver(choice);
		this.setSelection(true);
		this.queryValue = false;
		this.hideChoices();
	},

	filter: function(tokens) {
		var regex = new RegExp(((this.options.filterSubset) ? '' : '^') + this.queryValue.escapeRegExp(), (this.options.filterCase) ? '' : 'i');
		return (tokens || this.tokens).filter(regex.test, regex);
	},

	/**
	 * markQueryValue
	 *
	 * Marks the queried word in the given string with <span class="autocompleter-queried">*</span>
	 * Call this i.e. from your custom parseChoices, same for addChoiceEvents
	 *
	 * @param		{String} Text
	 * @return		{String} Text
	 */
	markQueryValue: function(str) {
		return (!this.options.markQuery || !this.queryValue) ? str
			: str.replace(new RegExp('(' + ((this.options.filterSubset) ? '' : '^') + this.queryValue.escapeRegExp() + ')', (this.options.filterCase) ? '' : 'i'), '<span class="autocompleter-queried">$1</span>');
	},

	/**
	 * addChoiceEvents
	 *
	 * Appends the needed event handlers for a choice-entry to the given element.
	 *
	 * @param		{Element} Choice entry
	 * @return		{Element} Choice entry
	 */
	addChoiceEvents: function(el) {
		return el.addEvents({
			'mouseover': this.choiceOver.bind(this, [el]),
			'click': this.choiceSelect.bind(this, [el])
		});
	}
});


/**
 * Autocompleter.Local
 *
 * @version		1.1.1
 *
 * @todo: Caching, no-result handling!
 *
 *
 * @license		MIT-style license
 * @author		Harald Kirschner <mail [at] digitarald.de>
 * @copyright	Author
 */
Autocompleter.Local = new Class({

	Extends: Autocompleter.Base,

	options: {
		minLength: 0,
		delay: 200
	},

	initialize: function(element, tokens, options) {
		this.parent(element, options);
		this.tokens = tokens;
	},

	query: function() {
		this.update(this.filter());
	}

});


/**
 * Autocompleter.Remote
 *
 * @version		1.1.1
 *
 * @todo: Caching, no-result handling!
 *
 *
 * @license		MIT-style license
 * @author		Harald Kirschner <mail [at] digitarald.de>
 * @copyright	Author
 */

Autocompleter.Ajax = {};

Autocompleter.Ajax.Base = new Class({

	Extends: Autocompleter.Base,

	options: {
		postVar: 'value',
		postData: {},
		ajaxOptions: {},
		onRequest: $empty,
		onComplete: $empty
	},

	initialize: function(element, options) {
		this.parent(element, options);
		var indicator = $(this.options.indicator);
		if (indicator) {
			this.addEvents({
				'onRequest': indicator.show.bind(indicator),
				'onComplete': indicator.hide.bind(indicator)
			}, true);
		}
	},

	query: function(){
		var data = $unlink(this.options.postData);
		data[this.options.postVar] = this.queryValue;
		this.fireEvent('onRequest', [this.element, this.request, data, this.queryValue]);
		this.request.send({'data': data});
	},

	/**
	 * queryResponse - abstract
	 *
	 * Inherated classes have to extend this function and use this.parent(resp)
	 *
	 * @param		{String} Response
	 */
	queryResponse: function() {
		this.fireEvent('onComplete', [this.element, this.request, this.response]);
	}

});

Autocompleter.Ajax.Json = new Class({

	Extends: Autocompleter.Ajax.Base,

	initialize: function(el, url, options) {
		this.parent(el, options);
		this.request = new Request.JSON($merge({
			'url': url,
			'link': 'cancel'
		}, this.options.ajaxOptions)).addEvent('onComplete', this.queryResponse.bind(this));
	},

	queryResponse: function(response) {
		this.parent();
		this.update(response);
	}

});

Autocompleter.Ajax.Xhtml = new Class({

	Extends: Autocompleter.Ajax.Base,

	initialize: function(el, url, options) {
		this.parent(el, options);
		this.request = new Request.HTML($merge({
			'url': url,
			'link': 'cancel',
			'update': this.choices
		}, this.options.ajaxOptions)).addEvent('onComplete', this.queryResponse.bind(this));
	},

	queryResponse: function(tree, elements) {
		this.parent();
		if (!elements || !elements.length) {
			this.hideChoices();
		} else {
			this.choices.getChildren(this.options.choicesMatch).each(this.options.injectChoice || function(choice) {
				var value = choice.innerHTML;
				choice.inputValue = value;
				this.addChoiceEvents(choice.set('html', this.markQueryValue(value)));
			}, this);
			this.showChoices();
		}

	}

});


/*
Script: Autocompleter.JsonP.js
	Implements JsonP support for the Autocompleter class.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/

Autocompleter.JsonP = new Class({

	Extends: Autocompleter.Ajax.Json,

	options: {
		postVar: 'query',
		jsonpOptions: {},
//		onRequest: $empty,
//		onComplete: $empty,
//		filterResponse: $empty
		minLength: 1
	},

	initialize: function(el, url, options) {
		this.url = url;
		this.setOptions(options);
		this.parent(el, options);
	},

	query: function(){
		var data = $unlink(this.options.jsonpOptions.data||{});
		data[this.options.postVar] = this.queryValue;
		this.jsonp = new JsonP(this.url, $merge({data: data},	this.options.jsonpOptions));
		this.jsonp.addEvent('onComplete', this.queryResponse.bind(this));
		this.fireEvent('onRequest', [this.element, this.jsonp, data, this.queryValue]);
		this.jsonp.request();
	},
	
	queryResponse: function(response) {
		this.parent();
		var data = (this.options.filter)?this.options.filter.run([response], this):response;
		this.update(data);
	}

});

/**
 * Observer - Observe formelements for changes
 *
 * @version		1.0rc3
 *
 * @license		MIT-style license
 * @author		Harald Kirschner <mail [at] digitarald.de>
 * @copyright	Author
 */
var Observer = new Class({

	Implements: [Options, Events],

	options: {
		periodical: false,
		delay: 1000
	},

	initialize: function(el, onFired, options){
		this.setOptions(options);
		this.addEvent('onFired', onFired);
		this.element = $(el) || $$(el);
		/* Clientcide change */
		this.boundChange = this.changed.bind(this);
		this.resume();
	},

	changed: function() {
		var value = this.element.get('value');
		if ($equals(this.value, value)) return;
		this.clear();
		this.value = value;
		this.timeout = this.onFired.delay(this.options.delay, this);
	},

	setValue: function(value) {
		this.value = value;
		this.element.set('value', value);
		return this.clear();
	},

	onFired: function() {
		this.fireEvent('onFired', [this.value, this.element]);
	},

	clear: function() {
		$clear(this.timeout || null);
		return this;
	},
	/* Clientcide change */
	pause: function(){
		$clear(this.timeout);
		$clear(this.timer);
		this.element.removeEvent('keyup', this.boundChange);
		return this;
	},
	resume: function(){
		this.value = this.element.get('value');
		if (this.options.periodical) this.timer = this.changed.periodical(this.options.periodical, this);
		else this.element.addEvent('keyup', this.boundChange);
		return this;
	}

});

var $equals = function(obj1, obj2) {
	return (obj1 == obj2 || JSON.encode(obj1) == JSON.encode(obj2));
};

/*
Script: AutoCompleter.Clientcide.js
	Adds Clientcide css assets to autocompleter automatically.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
(function(){
	var AcClientcide = function(){
		return {
			options: {
				baseHref: 'http://www.cnet.com/html/rb/assets/global/autocompleter/'
			},
			initialize: function() {
				this.parent.apply(this,arguments);
				this.writeStyle();
			},
			writeStyle: function(){
				window.addEvent('domready', function(){
					if($('AutocompleterCss')) return;
					new Element('link', {
						rel: 'stylesheet', 
						media: 'screen', 
						type: 'text/css', 
						href: this.options.baseHref+'Autocompleter.css', 
						id: 'AutocompleterCss'
					}).inject(document.head);
				}.bind(this));
			}
		};
	};
	Autocompleter.Base.refactor(AcClientcide());
	if (Autocompleter.Ajax) {
		["Base", "Xhtml", "Json"].each(function(c){
			if(Autocompleter.Ajax[c]) Autocompleter.Ajax[c] = Autocompleter.Ajax[c].refactor(AcClientcide());
		});
	}
	if (Autocompleter.Local) Autocompleter.Local.refactor(AcClientcide());
	if (Autocompleter.JsonP) Autocompleter.JsonP.refactor(AcClientcide());
})();


/*
Script: Lightbox.js
	A lightbox clone for MooTools.

* Christophe Beyls (http://www.digitalia.be); MIT-style license.
* Inspired by the original Lightbox v2 by Lokesh Dhakar: http://www.huddletogether.com/projects/lightbox2/.
* Refactored by Aaron Newton 

*/
var Lightbox = new Class({
	Implements: [Options, Events, Modalizer],
	binds: ['click', 'keyboardListener', 'addHtmlElements'],
	options: {
//	anchors: null,
		resizeDuration: 400,
//	resizeTransition: false,	// default transition
		initialWidth: 250,
		initialHeight: 250,
		zIndex: 5000,
		animateCaption: true,
		showCounter: true,
		autoScanLinks: true,
		relString: 'lightbox',
		useDefaultCss: true,
		assetBaseUrl: 'http://www.cnet.com/html/rb/assets/global/slimbox/',
		overlayStyles: {
			opacity: 0.8
		}
//	onImageShow: $empty,
//	onDisplay: $empty,
//	onHide: $empty
	},

	initialize: function(){
		var args = Array.link(arguments, {options: Object.type, links: Array.type});
		this.setOptions(args.options);
		var anchors = args.links || this.options.anchors;
		if (this.options.autoScanLinks && !anchors) anchors = $$('a[rel^='+this.options.relString+']');
		if(!$$(anchors).length) return; //no links!
		this.addAnchors(anchors);
		if(this.options.useDefaultCss) this.addCss();
		window.addEvent('domready', this.addHtmlElements.bind(this));
	},
		
	anchors: [],
	
	addAnchors: function(anchors){
		$$(anchors).each(function(el){
			if(!el.retrieve('lightbox')) {
				el.store('lightbox', this);
				this.attach(el);
			}
		}.bind(this));
	},
	
	attach: function(el) {		
		el.addEvent('click', this.click.pass(el, this));
		this.anchors.include(el);
	},

	addHtmlElements: function(){
		this.container = new Element('div', {
			'class':'lbContainer'
		}).inject(document.body);
		this.setModalOptions({
			elementsToHide: ''
		});
		this.overlay = this.layer().addClass('lbOverlay');
		this.setModalStyle(this.options.overlayStyles);
		this.popup = new Element('div', {
			'class':'lbPopup'
		}).inject(this.container);
		this.overlay.inject(this.popup).setStyles(this.options.overlayStyles);
		this.center = new Element('div', {
			styles: {	
				width: this.options.initialWidth, 
				height: this.options.initialHeight, 
				marginLeft: (-(this.options.initialWidth/2)),
				display: 'none',
				zIndex:this.options.zIndex+1
			}
		}).inject(this.popup).addClass('lbCenter');
		this.image = new Element('div', {
			'class': 'lbImage'
		}).inject(this.center);
		
		this.prevLink = new Element('a', {
			'class': 'lbPrevLink', 
			href: 'javascript:void(0);', 
			styles: {'display': 'none'}
		}).inject(this.image);
		this.nextLink = this.prevLink.clone().removeClass('lbPrevLink').addClass('lbNextLink').inject(this.image);
		this.prevLink.addEvent('click', this.previous.bind(this));
		this.nextLink.addEvent('click', this.next.bind(this));

		this.bottomContainer = new Element('div', {
			'class': 'lbBottomContainer', 
			styles: {
				display: 'none', 
				zIndex:this.options.zIndex+1
		}}).inject(this.popup);
		this.bottom = new Element('div', {'class': 'lbBottom'}).inject(this.bottomContainer);
		new Element('a', {
			'class': 'lbCloseLink', 
			href: 'javascript:void(0);'
		}).inject(this.bottom).addEvent('click', this.close.bind(this));
		this.overlay.addEvent('click', this.close.bind(this));
		this.caption = new Element('div', {'class': 'lbCaption'}).inject(this.bottom);
		this.number = new Element('div', {'class': 'lbNumber'}).inject(this.bottom);
		new Element('div', {'styles': {'clear': 'both'}}).inject(this.bottom);
		var nextEffect = this.nextEffect.bind(this);
		this.fx = {
			overlay: new Fx.Tween(this.overlay, {property: 'opacity', duration: 500}).set(0),
			resize: new Fx.Morph(this.center, $extend({
				duration: this.options.resizeDuration, 
				onComplete: nextEffect}, 
				this.options.resizeTransition ? {transition: this.options.resizeTransition} : {})),
			image: new Fx.Tween(this.image, {property: 'opacity', duration: 500, onComplete: nextEffect}),
			bottom: new Fx.Tween(this.bottom, {property: 'margin-top', duration: 400, onComplete: nextEffect})
		};

		this.preloadPrev = new Element('img');
		this.preloadNext = new Element('img');
	},
	
	addCss: function(){
		window.addEvent('domready', function(){
			if($('LightboxCss')) return;
			new Element('link', {
				rel: 'stylesheet', 
				media: 'screen', 
				type: 'text/css', 
				href: this.options.assetBaseUrl + 'slimbox.css',
				id: 'LightboxCss'
			}).inject(document.head);
		}.bind(this));
	},

	click: function(el){
		link = $(el);
		var rel = link.get('rel')||this.options.relString;
		if (rel == this.options.relString) return this.show(link.get('href'), link.get('title'));

		var j, imageNum, images = [];
		this.anchors.each(function(el){
			if (el.get('rel') == link.get('rel')){
				for (j = 0; j < images.length; j++) if(images[j][0] == el.get('href')) break;
				if (j == images.length){
					images.push([el.get('href'), el.get('title')]);
					if (el.get('href') == link.get('href')) imageNum = j;
				}
			}
		}, this);
		return this.open(images, imageNum);
	},

	show: function(url, title){
		return this.open([[url, title]], 0);
	},

	open: function(images, imageNum){
		this.fireEvent('onDisplay');
		this.images = images;
		this.setup(true);
		this.top = (window.getScroll().y + (window.getSize().y / 15)).toInt();
		this.center.setStyles({
			top: this.top,
			display: ''
		});
		this.fx.overlay.start(this.options.overlayStyles.opacity);
		return this.changeImage(imageNum);
	},

	setup: function(open){
		var elements = $$('object, iframe');
		elements.extend($$(Browser.Engine.trident ? 'select' : 'embed'));
		elements.reverse().each(function(el){
			if (open) el.store('lbBackupStyle', el.getStyle('visibility') || 'visible');
			var vis = (open ? 'hidden' : el.retrieve('lbBackupStyle') || 'visible');
			el.setStyle('visibility', vis);
		});
		var fn = open ? 'addEvent' : 'removeEvent';
		document[fn]('keydown', this.keyboardListener);
		this.step = 0;
	},

	keyboardListener: function(event){
		switch (event.code){
			case 27: case 88: case 67: this.close(); break;
			case 37: case 80: this.previous(); break;	
			case 39: case 78: this.next();
		}
	},

	previous: function(){
		return this.changeImage(this.activeImage-1);
	},

	next: function(){
		return this.changeImage(this.activeImage+1);
	},

	changeImage: function(imageNum){
		this.fireEvent('onImageShow', [imageNum, this.images[imageNum]]);
		if (this.step || (imageNum < 0) || (imageNum >= this.images.length)) return false;
		this.step = 1;
		this.activeImage = imageNum;

		this.center.setStyle('backgroundColor', '');
		this.bottomContainer.setStyle('display', 'none');
		this.prevLink.setStyle('display', 'none');
		this.nextLink.setStyle('display', 'none');
		this.fx.image.set(0);
		this.center.addClass('lbLoading');
		this.preload = new Element('img', {
			events: {
				load: function(){
					this.nextEffect.delay(100, this)
				}.bind(this)
			}
		});
		this.preload.set('src', this.images[imageNum][0]);
		return false;
	},

	nextEffect: function(){
		switch (this.step++){
		case 1:
			this.image.setStyle('backgroundImage', 'url('+this.images[this.activeImage][0]+')');
			this.image.setStyle('width', this.preload.width);
			this.bottom.setStyle('width',this.preload.width);
			this.image.setStyle('height', this.preload.height);
			this.prevLink.setStyle('height', this.preload.height);
			this.nextLink.setStyle('height', this.preload.height);

			this.caption.set('html',this.images[this.activeImage][1] || '');
			this.number.set('html',(!this.options.showCounter || (this.images.length == 1)) ? '' : 'Image '+(this.activeImage+1)+' of '+this.images.length);

			if (this.activeImage) $(this.preloadPrev).set('src', this.images[this.activeImage-1][0]);
			if (this.activeImage != (this.images.length - 1)) 
				$(this.preloadNext).set('src',  this.images[this.activeImage+1][0]);
			if (this.center.clientHeight != this.image.offsetHeight){
				this.fx.resize.start({height: this.image.offsetHeight});
				break;
			}
			this.step++;
		case 2:
			if (this.center.clientWidth != this.image.offsetWidth){
				this.fx.resize.start({width: this.image.offsetWidth, marginLeft: -this.image.offsetWidth/2});
				break;
			}
			this.step++;
		case 3:
			this.bottomContainer.setStyles({
				top: (this.top + this.center.getSize().y), 
				height: 0, 
				marginLeft: this.center.getStyle('margin-left'), 
				display: ''
			});
			this.fx.image.start(1);
			break;
		case 4:
			this.center.style.backgroundColor = '#000';
			if (this.options.animateCaption){
				this.fx.bottom.set(-this.bottom.offsetHeight);
				this.bottomContainer.setStyle('height', '');
				this.fx.bottom.start(0);
				break;
			}
			this.bottomContainer.style.height = '';
		case 5:
			if (this.activeImage) this.prevLink.setStyle('display', '');
			if (this.activeImage != (this.images.length - 1)) this.nextLink.setStyle('display', '');
			this.step = 0;
		}
	},

	close: function(){
		this.fireEvent('onHide');
		if (this.step < 0) return;
		this.step = -1;
		if (this.preload) this.preload.destroy();
		for (var f in this.fx) this.fx[f].cancel();
		this.center.setStyle('display', 'none');
		this.bottomContainer.setStyle('display', 'none');
		this.fx.overlay.chain(this.setup.pass(false, this)).start(0);
		return;
	}
});
window.addEvent('domready', function(){if($(document.body).get('html').match(/rel=?.lightbox/i)) new Lightbox()});


/*
Script: FormValidator.French.js
	FormValidator messages in French. Thanks Miquel Hudin.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
FormValidator.resources.FR = {
	required:'Ce champ est obligatoire.',
	minLength:'S\'il vous vous plat crivez au moins {minLength} caractres (vous avez crit {length} caractres).',
	maxLength:'S\'il vous vous plat n\'crivez pas plus de {maxLength} caractres (vous avez crit {length} caractres).',
	integer:'S\'il vous vous plat crivez un nombre entier dans ce champ. Les nombres avec des dcimals  (p.ex. 1\'25) ne sont pas permis.',
	numeric:'S\'il vous vous plat crivez seulement des chiffres dans ce champ (p.ex. "1" ou "1\'1" ou "-1" or "-1\'1").',
	digits:'S\'il vous vous plat crivez des nombres et des signes de ponctuation seulement dans ce champ (par exemple, un numro de tlphone avec des traits d\'union est permis).',
	alpha:'S\'il vous vous plat crivez seulement des lettres (a-z) dans ce champ. Les espaces ou d\'autres caractres ne sont pas permis.',
	alphanum:'S\'il vous vous plat crivez seulement des lettres (a-z) ou des chiffres (0-9) dans ce champ. Les espaces ou d\'autres caractres ne sont pas permis.',
	dateSuchAs:'S\'il vous vous plat crivez une date correcte, comme {date}',
	dateInFormatMDY:'S\'il vous vous plat crivez une date correcte, comme JJ/MM/AAAA (p.ex. "31/11/1999")',
	email:'S\'il vous vous plat crivez une adresse de courrier lectronique. Par example "fred@domain.com".',
	url:'S\'il vous vous plat crivez une URL, comme http://www.google.com.',
	currencyDollar:'S\'il vous vous plat crivez une quantit correcte. Par example $100.00 .',
	oneRequired:'S\'il vous vous plat slectionnez au moins une de ces options.',
	errorPrefix: 'Erreur: ',
	warningPrefix: 'Attention: '
};

/*
Script: SimpleEditor.French.js
	SimpleEditor messages in French. Thanks Miquel Hudin.

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
SimpleEditor.resources.FR = {
	woops:'Oops',
	nopeCtrlC:'Désolé, cette fonction ne fonctionne pas ici. Utilisez ctrl+c.',
	nopeCtrlX:'Désolé, cette fonction ne fonctionne pas ici. Utilisez ctrl+x.',
	linkURL:'L\'URL pour le lien',
	linkText:'Texte du lien',
	imgURL:'L\'URL de l\'image',
	imgAlt:'Le titre (alt) de l\'image'
};


/*
Script: Element.Delegation.js
	Extends the Element native object to include the delegate method for more efficient event management.
	
	Event checking based on the work of Daniel Steigerwald.
	License: MIT-style license.
	Copyright: Copyright (c) 2008 Daniel Steigerwald, daniel.steigerwald.cz

License:
	http://www.clientcide.com/wiki/cnet-libraries#license
*/
(function(){
	
	var getType = function(type) {
		var custom = Element.Events.get(type);
		return custom ? custom.base : type;
	};

	var checkOverOut = function(el, e){
		if (el == e.target || el.hasChild(e.target)) {
			var related = e.relatedTarget;
			if (related == undefined) return true;
			if (related === false) return false;
			return ($type(el) != 'document' && related != el && related.prefix != 'xul' && !el.hasChild(related));
		}
	};
	
	var check = function(e, test) {
		var target = e.target;
		var isOverOut = /^(mouseover|mouseout)$/.test(e.type);
		var els = this.getElements(test);
		var match = els.indexOf(target);
		if (match >= 0) return els[match];
		for (var i = els.length; i--; ) {
			var el = els[i];
			if (el == target || el.hasChild(target)) {
				return (!isOverOut || checkOverOut(el, e)) ? el : false;
			}
		}
	};
	
	var splitType = function(type) {
		if (type.test(/^.*?\(.*?\)$/)) {
			return {
				event: type.match(/.*?(?=\()/),
				selector: type.replace(/^.*?\((.*)\)$/,"$1")
			}
		}
		return {event: type};
	};
	
	var oldAddEvent = Element.prototype.addEvent,
		oldAddEvents = Element.prototype.addEvents,
	 	oldRemoveEvent = Element.prototype.removeEvent,
	 	oldRemoveEvents = Element.prototype.removeEvents;
	Element.implement({
		//use as usual (this.addEvent('click', fn))
		//or for delegation
		//this.addEvent('click(div.foo)', fn)
		addEvent: function(type, fn){
			var splitted = splitType(type);
			//if the type has a selector
			if (splitted.selector) {
				//get the delegates and the monitors
				var monitors = this.retrieve('$moo:delegateMonitors', {});
				//if there's not a delegate for this type already
				if (!monitors[type]) {
					//create a monitor that will fire the event when it occurs
					var monitor = function(e) { 
						var el = check.call(this, e, splitted.selector);
						if (el) this.fireEvent(type, [e, el], 0, el);
					}.bind(this);
					monitors[type] = monitor;
					//add the monitor to the element
					//translates to this.addEvent('click', monitor);
					//monitor just looks for clicks that match the selector
					//and fires the event
					//we only create one monitor for a given selector and all it does
					//is calls fireEvent for that selector
					//i.e. this.fireEvent('click:div.foo')
					oldAddEvent.call(this, splitted.event, monitor);
				}
			}
			return oldAddEvent.apply(this, arguments);
		},
		removeEvent: function(type, fn){
			//if we're removing an event with a selector
			var splitted = splitType(type);
			if (splitted.selector) {
				var events = this.retrieve('events');
				//if there are no events with this type or
				//if a specific fn is being removed but isn't attached, return
				if (!events[type] || (fn && !events[type].contains(fn))) return this;
				//if a fn is defined, remove it from the events
				if (fn) events[type].erase(fn);
				//else empty all the events of this type
				else events[type].empty();
				//if there are none left, we remove the monitor, too
				if (events[type].length == 0) {
					//get the monitors we've created
					var monitors = this.retrieve('$moo:delegateMonitors', {});
					//remove the monitor
					oldRemoveEvent(splitted.event, monitors[type]);
					//delete the monitor from the monitors list
					delete monitors[type];
				}
				return this;
			}
			//otherwise do what you normally do and remove the event (no selector involved)
			return oldRemoveEvent.apply(this, arguments);
		},
		//fireEvent is changed to allow a bind argument so that we can bind
		//the method to the element in our monitor above
		fireEvent: function(type, args, delay, bind){
			var events = this.retrieve('events');
			if (!events || !events[type]) return this;
			events[type].keys.each(function(fn){
				//added the bind||this
				fn.create({'bind': bind||this, 'delay': delay, 'arguments': args})();
			}, this);
			return this;
		}
	});

})();