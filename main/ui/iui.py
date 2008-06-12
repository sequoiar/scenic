
# Twisted imports
from zope.interface import Interface

class IUI(Interface):
    """
    A specific user interface (CLI, Web, GTK, etc.)
    """
    def test():
        """
        Returns something for test purpose
        """


