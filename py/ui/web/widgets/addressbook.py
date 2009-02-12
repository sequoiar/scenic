from nevow.athena import expose

#App imports
from ui.web.web import Widget
from utils import log
from utils.i18n import to_utf

log = log.start('debug', 1, 0, 'web_adb')


class Addressbook(Widget):
    """
    """
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
