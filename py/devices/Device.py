# TODO : what is the software design exactly ? 

class Device:
    """
        Base class for any Device.
    """
    def __init__(self,**kwargs):
        """
        Child class should call the setAttributes() method with kwargs as an argument.
        """
        self.attributes = dict(kwargs)
        #self.setAttributes(kwargs)
    
    #def setAttributes(self,kwargs):
    #    """
    #    Set many attributes at once
    #    """
    #    pass
    
    def set(self,attr,val):
        """
        Sets one attribute
        """
        self.attributes[attr] = val
    
    def get(self,attr):
        """
        Gets one attribute
        """
        ret = None
        try:
            ret = self.attributes[attr]
        except KeyError:
            pass
        return ret
        
    def getAttributeNames(self):
        """
        Gets list of all attributes names
        """
        return self.attributes.keys()
    
    #TODO: set_all() ?

if __name__ == '__main__':
    # test
    class TestAudioDev(Device):
        def __init__(self,**kwargs):
            #self.setAttributes(kwargs)
            self.attributes = dict(kwargs)
            
        def printAll(self):
            # for debugging
            for k in self.attributes.keys():
                print "%s = %s" % (k, self.attributes[k])
    
    print "starting test"
    d = TestAudioDev(sampleRates=[44100,48000], brand='MOTU')
    d.printAll()
    print "DONE"
    
