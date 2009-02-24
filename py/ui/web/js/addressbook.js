// import Nevow.Athena

Addressbook = Nevow.Athena.Widget.subclass('Addressbook');

Addressbook.methods(
    function __init__(self, node) {
        Addressbook.upcall(self, "__init__", node);
        self.adbList = self.nodeByAttribute('name', 'adbList');
    },  

    function getList(self) {
        self.callRemote("rc_get_list");
        return false;
    },
 
    function updateList(self, contacts) {
		if ( self.adbList.hasChildNodes() ) {
		    while ( self.adbList.childNodes.length >= 1 ) {
		        self.adbList.removeChild( self.adbList.firstChild );       
		    } 
		}
		for (var i=0; i<contacts.length; i++) {
//			console.info(contacts[i]);
	        var newNode = document.createElement('option');
	        newNode.appendChild(document.createTextNode(contacts[i][0]));
	        self.adbList.appendChild(newNode);
		}
		return false;
});
