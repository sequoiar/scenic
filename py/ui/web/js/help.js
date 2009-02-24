// import Nevow.Athena

Help = Nevow.Athena.Widget.subclass('Help');

Help.methods(
    function __init__(self, node) {
        Help.upcall(self, "__init__", node);
    },  

    function manual(self) {
        self.callRemote("rc_manual");
        return false;
    }
);
