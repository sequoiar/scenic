from nevow import rend, loaders, appserver, static, tags
from nevow.athena import LivePage, LiveElement, expose

#App imports
from utils import log
from utils.i18n import to_utf
from utils.common import find_callbacks

log = log.start('debug', 1, 0, 'web_adb')

def render(self, ctx, data):
    """
    Replace the tag with a new AddressBook element.
    """
    adb = AddressBook(self.api)
    adb.setFragmentParent(self)
    return adb



class AddressBook(LiveElement):
    """
    """
    docFactory = loaders.xmlfile('ui/web/addressbook.html')
#    jsClass = u"AddressBook.List"
    
    def __init__(self, api):
        LiveElement.__init__(self)
        self.callbacks = find_callbacks(self)
        self.api = api

    def _get_contacts(self, origin, data):
        adb = []
        contacts = data[0].items()
        contacts.sort()
        for name, contact in contacts:
            adb.append((contact.name, to_utf(contact.address), contact.port))
        log.info('receive update: %r' % self)
        print adb
        self.callRemote('updateList', adb)
        
    def get_list(self):
        self.api.get_contacts(self)
        return False
    expose(get_list)
