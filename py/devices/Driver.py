class Driver:
    """
        Base class for a driver for a type of Device.
        Drivers must inherit from this class and implement each method.
    """
    def start(self):
        """
        Starts the use of the device. 
        
        Called only once on system startup.
        There should be a "started" class variable for each driver in 
        order to make sure it is started only once.
        """
        pass
    
    def list(self):
        """Lists name of devices of the type that a driver supports.
        Returns a list of string.
        """
        return []
    
    def get(self,device_name=None):
        """
            Returns a device object.
            device_name must be a ASCII string.
            
            Returns None in case of device doesn't exist ?
            TODO: should it raise an exception instead
        """
        return None

