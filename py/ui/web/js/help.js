/**
 * Help for Miville's web interface.
 *
 * @author Society for Arts and Technology
 */

// import Nevow.Athena

Help = Nevow.Athena.Widget.subclass('Help');

Help.methods(
    function __init__(self, node) {
        /**
         * Constructor for the help widget.
         *
         * @param self The instance of the Help Nevow Widget.
         * @param node Please document this.
         */
        Help.upcall(self, "__init__", node);
    },  

    function manual(self) {
        /**
         * The manual.
         *
         * @param self The instance of the Help Nevow Widget.
         */
        self.callRemote("rc_manual");
        return false;
    }
);
