// import Nevow.Athena

AddressBook.List = Nevow.Athena.Widget.subclass('AddressBook.List');

AddressBook.List.methods(
    function __init__(self, node) {
        AddressBook.List.upcall(self, "__init__", node);
        self.adbList = self.nodeByAttribute('name', 'adbList');
    },  

//    function doSay(self) {
//        self.callRemote("say", self.message.value);
//        self.message.value = ""; 
//        return false;
//    },
 
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
//        var newNode = document.createElement('option');
//        newNode.appendChild(document.createTextNode(contacts[0]));
//        self.adbList.appendChild(newNode);
    });
