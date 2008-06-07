
# Twisted imports
from zope.interface import Interface, Attribute

class IUI(Interface):
    """
    A specific user interface (CLI, Web, GTK, etc.)
    """
    def test(var):
        """
        Returns the pressure this material can support without
        fracturing at the given temperature.

    @type temperature: C{float}
    @param temperature: Kelvins

    @rtype: C{float}
    @return: Pascals
        """

    dielectricConstant = Attribute("""
        @type dielectricConstant: C{complex}
        @ivar dielectricConstant: The relative permittivity, with the
        real part giving reflective surface properties and the
        imaginary part giving the radio absorption coefficient.
        """)