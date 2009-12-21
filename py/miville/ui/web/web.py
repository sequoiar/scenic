#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Miville
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Miville is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Miville.  If not, see <http://www.gnu.org/licenses/>.

"""
This modules is the base of the web client (web user interface)
of the application. It supports internationalisation (i18n).

More about the i18n process
---------------------------

Here we describe all the steps involved in producing translations for this module.
Maybe, when you read this, there will be script that simplifies the process.
But anyway it's good to know how it works.

So here the steps to produce a translation from scratch:

    We will use the context of creating a new Widget as an example.
    
    1. from nevow.i18n import _ and use this function everywhere you need
       something translated in your python code::
        
          from nevow.i18n import _
          
          def example(feature):
              return _('Sorry, the feature %s is not implemented.' % feature)

    2. In your XML template put the attribute n:render="i18" inside the
       element you need to translate::
       
           <h3 n:render="i18n">Nerds</h3>
           
    3. Extract the strings to translate from your python script with GNU xgettext.
       Put the name of your Widget as the name of the .poy file::
       
           $ xgettext -o pathToLocaleDirectory/your_widget.poy your_widget.py
    
    4. Extract the strings to translate from your XML template with
       nevow-xmlgettext. Put the name of your Widget as the name of the .poh file::
       
           $ nevow-xmlgettext your_widget.xml > pathToLocaleDirctory/your_widget.poh
           
    5. Merge the two newly created files (.poy and .poh) with msgcat.
       Put the .poy file first::
       
           $ cd pathToLocaleDirectory
           $ msgcat -o your_widget.pot your_widget.poy your_widget.poh
           
    6. Create one .po file for each language you want to support with msginit::
    
           $ msginit -l fr_CA -i your_widget.pot
           
       The resulting file your_widget.po should be created in the directory
       localeDirectory/LL_CC/LC_MESSAGES/. You can add the -o arguments to
       msginit to specified the right place.
        
    7. Translate all the strings in you your_widget.po file for each languages.
       You can do this with a tool like Poedit.
       
    8. Convert .po file to the binary format .mo with msgfmt::
    
           $ msgfmt localeDirectory/fr_CA/LC_MESSAGES/your_widget.po
           
    That's it! Now we need to add how to update the translations with
    the evolution of the code and template.
           
"""
import os.path as path
import os
from xml.etree import ElementTree
from twisted.python.modules import getModule
try:
    from nevow import loaders, appserver, static, tags, inevow
    from nevow.athena import LivePage, LiveFragment, expose as nevow_expose
    from nevow.i18n import render as i18nrender
    from nevow.i18n import I18NConfig
except ImportError:
    raise ImportError('If you want to use the Web interface, you need to install Nevow.')
from miville.utils import Observer, log
from miville.utils.i18n import to_utf
from miville.utils.common import find_callbacks
from miville.ui.web.pages.procspage import ProcsPage

log = log.start('debug', True, True, 'web')

# globals
widgets_mod = []
BASE_PATH = os.path.dirname(__file__)
#XXX template name must be specified as global
TEMPLATE_NAME = None
TEMPLATE_NAME = "blue" 

def import_widgets():
    """
    Imports all the widget modules found in the miville/ui/web/widgets 
    directory and add render_<widget> method to the Index class for each.
    """
    package = 'miville.ui.web.widgets'
    loaded = []
    # TODO: fix weird bug. All widgets are loaded 3 times
    for widget in getModule(package).iterModules():
        if widget.name not in loaded:
            log.debug('Will load widget %s %s %s' % (widget, widget.name, widget.filePath.path))
            log.debug("%s" % (loaded))
            loaded.append("%s" % (widget.name))
            if widget.isPackage():
                medias = getModule(widget.name).iterModules()
                for media in medias:
                    try:
                        mod = media.load()
                    except:
                        log.error('Unable to load the module %s' % media.name)
                        raise
                    else:
                        if hasattr(mod, 'render'):
                            widgets_mod.append(mod)
            else:
                try:
                    mod = widget.load()
                    log.debug('Loaded the module %s' % (widget.name.split(".")[-1]))
                except:
                    log.error('Unable to load the module %s' % widget.name)
                    raise
                else:
                    if hasattr(mod, camel_name(mod.__name__.replace(package, '')[1:])):
                        widgets_mod.append(mod)
    # for each widget add a render_<widget> method to the Index class
    for widget in widgets_mod:
        name = str(widget.__name__.replace(package, '')[1:].replace('.', '_'))
        setattr(Index, 'render_' + name, widget_factory(widget))

def widget_factory(mod):
    """
    Factory that produces a render_widget method to be added to the Index class
    for each widget.
    """
    # get the module name without the path
    mod_name = mod.__name__.split('.')[ - 1]
    # convert the module to the class name by removing the 
    # underscore and camelcasing it
    klass_name = camel_name(mod_name)
    # get the class from the widget module
    klass = getattr(mod, klass_name)
    def render_widget(self, ctx, data):
        """
        Generic method used to render all widgets from the Index class.
        """
        widget = klass(self.api, TEMPLATE_NAME)
        widget.setFragmentParent(self)
        return widget
    return render_widget

def camel_name(name):
    """
    Transforms a file name (small case with underscore) to a class name style
    (camel case without underscore). Put first character and each character
    after an underscore in upper case and remove the underscores.
    
    Example::
    
        class_name -> ClassName
    """
    return ''.join([part.title() for part in name.split('_')])

def small_name(name):
    """
    Transforms a class name to a file name (all lower case with underscore)
    
    Example::
    
        ClassName -> class_name
    """
    chars = []
    for i, char in enumerate(name):
        if i > 0 and char.isupper():
            chars.append('_')
        chars.append(char)
    return ''.join(chars).lower()

def create_css(href):
    """
    Create a css link Stan tag
    """ 
    return tags.link(rel="stylesheet", type="text/css", href=href)

def create_js(src):
    """
    Create a js script Stan tag
    """ 
    return tags.script(src=src, type="text/javascript")

class Index(LivePage, Observer):
    """
    Class representing the base of the root page (/index.xml).
    
    Its child paths are :
     * js/ => templates/default/js AND js/
     * css/ => templates/default/css
     * img/ => templates/default/img
    """
    addSlash = True
    # serve some static files
    child_jslib = static.File(path.join(BASE_PATH, 'js/'))
    # XXX: deprecated:
    #child_css = static.File(path.join(BASE_PATH, 'templates/default/css/'))
    #child_js = static.File(path.join(BASE_PATH, 'templates/default/js/'))
    #child_img = static.File(path.join(BASE_PATH, 'templates/default/img/'))
    # let's add all the templates as static files. This is much simpler.
    child_templates = static.File(path.join(BASE_PATH, 'templates/'))
    # render_i18n = i18nrender()
    def __init__(self, subject):
        Observer.__init__(self, subject)
        self.subject = subject
        self.api = subject.api
        log.info("Using template %s" % (TEMPLATE_NAME))
        self.lang = 'en'
        # load base XML file
        self.docFactory = loaders.xmlfile(path.join(BASE_PATH, 'index.xml'))
        # not used anymore ?
        #        if TEMPLATE_NAME:
        #            self.child_template = static.File(path.join('ui/web/templates', TEMPLATE_NAME))
        LivePage.__init__(self)

    def childFactory(self, ctx, name):
        log.info("childFactory %s %s" % (ctx, name) )
        #if name.lower().startswith("settings"):
        #    return SettingsPage()
        #if name.lower().startswith("gst"):
        #    return GstPage()
        if name.lower().startswith("procs"):
            return ProcsPage()

    def renderHTTP(self, ctx):
        """
        Overwrite renderHTTP to get GET arguments, cookies values
        and client IP address.
        """
        # We're only overriding renderHTTP to look for a 'lang' query parameter
        # without cluttering up the messages renderer, below.
        #
        # If 'lang' is present then we "force" the translation language. This
        # simulates how user preferences from the session might be used to
        # override the browser's language settings.
        lang = ctx.arg('lang')
        if lang is not None:
            ctx.remember([lang], inevow.ILanguages)
        #        request = inevow.IRequest(ctx)
        #        request.addCookie('temp', 'allo', expires=None, domain=None, path=None, max_age=None, comment=None, secure=None)
        #        cookie = request.getCookie('temp')
        #        print cookie
        # add the locale file for the index
        ctx.remember(I18NConfig(domain='index', localeDir='locale'), inevow.II18NConfig)                
        # get the client IP address to look if he is in the LAN
        # client_ip = request.getClientIP())
        #
        # Let the base class handle it, really.
        return LivePage.renderHTTP(self, ctx)

    def list_widgets(self):
        """
        This method return a list of all the Widgets that are present
        in the index template.

        Loads templates/XXX/xml/index.xml or templates/default/xml/index.xml
        where XXX is the name of the template if its index.xml file is present.
        """
        index_file = path.join(BASE_PATH, 'templates/default/xml/index.xml')
        # if we don't use default template check if there's a valid index.xml
        # in the choose template directory and set the path accordinally
        if TEMPLATE_NAME is not None:
            tmp_path = path.join(BASE_PATH, 'templates', TEMPLATE_NAME, 'xml/index.xml')
            if path.isfile(tmp_path):
                index_file = tmp_path
        # parse the XML, find the widget element and return the list
        # of widget's name
        tree = ElementTree.parse(index_file)
        divs = tree.getiterator('div')
        attribute = '{http://nevow.com/ns/nevow/0.1}render'
        widgets = [div.attrib[attribute]
                   for div in divs
                   if (div.attrib.has_key(attribute)
                       # remove i18n because it's not a widget
                       and div.attrib[attribute] != 'i18n')]
        return widgets
    
    def render_header(self, ctx, data):
        """
        Add javascript and css imports to the page header.
       
        Files linked to in header: 
         * templates/default/css/style.css
         * templates/default/css/XXX.css where XXX is the name of each widget.
         * templates/default/js/XXX.js where XXX is the name of each widget.

        If the template name is set to "blue", it loads :
         * templates/blue/css/XXX.css where XXX is the name of each widget.
         * templates/blue/js/XXX.js where XXX is the name of each widget.
        ...if they are found, instead of the "default" one.
        """
        # FIXME: I think we should show the full tree of files instead
        # of hacking paths one by one.
        log.info("Template: %s" % (TEMPLATE_NAME))
        templates_path = path.join(BASE_PATH, 'templates')
        widgets = self.list_widgets()
        widgets.insert(0, 'index')
        log.debug(widgets)
        links = []
        # add style.css files
        style_file = 'css/style.css'
        if path.isfile(path.join(templates_path, 'default', style_file)):
            links.append(create_css(path.join('templates', 'default', style_file)))
        if TEMPLATE_NAME is not None and \
            path.isfile(path.join(templates_path, TEMPLATE_NAME, style_file)):
            links.append(create_css(path.join('templates', TEMPLATE_NAME, style_file)))
        kinds = {'css':create_css, 'js':create_js}
        # add nevow js, display js and css for every widget
        for widget in widgets:
            widget_file = '%s.js' % widget
            # add the nevow js code for this widget
            widget_path = path.join(BASE_PATH, 'js', widget_file)
            if path.isfile(widget_path):
                # links.append(self.create_js(path.join('jslib', widget_file)))
                self.jsModules.mapping[camel_name(widget)] = widget_path
            else:
                log.debug("'%s' is not a valid file." % widget_path)
            for kind in kinds.keys():
                # kind directory plus name of the file. Ex.: css/index.css
                file_name = path.join(kind, '%s.%s' % (widget, kind))
                if TEMPLATE_NAME is not None and \
                path.isfile(path.join(templates_path, TEMPLATE_NAME, file_name)):
                    links.append(kinds[kind](path.join('templates', TEMPLATE_NAME, file_name)))
                    #FIXME: Not quite legible. 
                    # This appends to a list what calling either create_css() 
                    # or create_js() returns. The path looks like "template/XXX.css or template/XXX.js where XXX is the name of the widget.
                elif path.isfile(path.join(templates_path, 'default', file_name)):
                    links.append(kinds[kind](path.join('templates', 'default', file_name)))
        # prettier xml output
        for i in range(1, len(links) * 2, 2):
            links.insert(i, ['\n\t\t'])
        return links

    def render_body(self, ctx, data):
        """
        Load and render the body of the page from the selected template.
        """
        body_file = 'xml/index.xml'
        templates_path = path.join(BASE_PATH, 'templates')
        if TEMPLATE_NAME is not None and path.isfile(path.join(templates_path, TEMPLATE_NAME, body_file)):
            template = TEMPLATE_NAME
        else:
            template = 'default'
        body = loaders.xmlfile(path.join(templates_path, template, body_file))
        return body

    def child_ (self, ctx):
        """
        This create a new Index instance on browser refresh and multiple 
        connections.
        """
        return Index(self.subject)

    def update(self, origin, key, data):
        """
        Observer update method.
        Call proper callbacks in all the Widgets.
        """
        for children in self.liveFragmentChildren:
            if hasattr(children, 'callbacks') and key in children.callbacks:
                children.callbacks[key](origin, data)

# Remove the rewrite of the XML element id
# (now we need to avoid clash of id ourself)
for key, preprocessors in enumerate(LiveFragment.preprocessors):
    if preprocessors.__name__ == "rewriteAthenaIds":
        del LiveFragment.preprocessors[key]
        
class Widget(LiveFragment):
    """
    Base class for all the widget classes.
    *Note: we use the deprecated LiveFragment instead of LiveElement because
    i18n need the context, so it doesn't work with LiveElement.* 
    Every method of a Widget that starts with "cb_" catches a callback 
    for Miville API 
    Subject notification.
    """
    # add the translation renderer
    render_i18n = i18nrender()

    def __init__(self, api, template):
        """
        Get the name of the widget and find the template file (in template and,
        if not, in 'default'.)
        """
        self.docFactory = ['']  # not sure if it's a good idea ?
        class_name = self.__class__.__name__
        tmpl_name = 'xml/%s.xml' % small_name(class_name)
        tmpl_path = path.join(BASE_PATH, 'templates')
        if template and path.isfile(path.join(tmpl_path, template, tmpl_name)):
            self.docFactory = loaders.xmlfile(path.join(tmpl_path, template, tmpl_name))
        elif path.isfile(path.join(tmpl_path, 'default', tmpl_name)):
            self.docFactory = loaders.xmlfile(path.join(tmpl_path, 'default', tmpl_name))
        else:
            log.error("Didn't find any valid %s.xml template." % small_name(class_name))
        # add the JS class for this widget
        # (should be the same name has the python class) 
        self.jsClass = to_utf(class_name)
        LiveFragment.__init__(self)
        self.callbacks = find_callbacks(self, 'cb_')
        log.debug("Found API callbacks : %s" % (self.callbacks.keys()))
        self.api = api

    def rend(self, ctx, data):
        """
        We overwrite LiveFragment.rend to add the good i18n domain
        to the context of each Widget base on the module name of the widget. 
        """
        # TODO: get the good localeDir for each platform
        domain = self.__module__.split('.')[ - 1]
        ctx.remember(I18NConfig(domain=domain, localeDir='locale'),
                     inevow.II18NConfig)                
        # Let the base class handle it.
        return LiveFragment.rend(self, ctx, data)

    def callRemote(self, methodName, *args):
        """
        Overwrite callRemote to tranform all object of type string to utf.
        """
        args = tree_to_utf(*args)
        # call the base class callRemote
        LiveFragment.callRemote(self, methodName, *args)
        
def tree_to_utf(*args):
    """
    Convert every string to utf in mixed structure of int, float, string,
    unicode, list, tuple and dict to be ready for JSON.
    """
    clean_args = []
    for arg in args:
        if isinstance(arg, str):
            clean_args.append(to_utf(arg))
        elif isinstance(arg, (tuple, list)):
            clean_args.append(tree_to_utf(*arg))
        elif isinstance(arg, dict):
            clean_dict = {}
            for key, value in arg.items():
                clean_dict[to_utf(key)] = tree_to_utf(value)[0]
            clean_args.append(clean_dict)
        else:
            clean_args.append(arg)
    return clean_args

def expose(loc):
    """
    Function that automatically exposes 
    (that means become accessible from javascript)
    all methods beginning by "rc\_" (for "remote call") in the local scope
    of a Class.
    
    :param loc: Should be a dictionnary where values are method object
                (usually you pass locals())
     
    Usage::
        
        class Widget(LiveElement):
            def rc_method(self):
                pass
            expose(locals())
    """
    for key, value in loc.iteritems():
        if key.startswith('rc_'):
            nevow_expose(value)

def start(subject, port=8080, interfaces='', template_name="blue"):
    """
    This function is call when the core find and load all the UI packages.
    """
    global TEMPLATE_NAME
    TEMPLATE_NAME = template_name
    site = appserver.NevowSite(Index(subject))
    # reactor.listenTCP(port, site, 5, '127.0.0.1')
    # subject is the api...
    subject.api.listen_tcp(port, site, interfaces) #  subject.config.ui_network_interfaces)
    import_widgets()
