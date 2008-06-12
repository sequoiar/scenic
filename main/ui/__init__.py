
# Twisted imports
from twisted.python.modules import getModule


def find_ui():
    """
    Find all the different user interfaces available
    """
    all_ui = []
    sub_mods = getModule('ui').iterModules()
    for sub_mod in sub_mods:
        if sub_mod.isPackage():
            all_ui.append(sub_mod.name)
            print "Loading User interface: " + sub_mod.name
            sub_mod.load()
    return all_ui

