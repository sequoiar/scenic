import logging

class myRingBuffer():
    def __init__(self, bufferSize=2048):
        self.bufferSize = bufferSize
        self.start = 0
        self.end = 0
        self.buffer = []
        for i in range (0, bufferSize):
            self.buffer.append(0)

    #reset counter to 0   
    def flush(self):
        self.start = 0
        self.end = 0
    
    #get the total len of the ring    
    def len(self):
        return ( self.end + self.bufferSize - self.start) % self.bufferSize

    #get how much space we can use in the buffer
    def avail_for_put(self):
        return (self.bufferSize - self.len())
    
    #Write Data in th ering buffer
    def put(self, newNote):
        if ( self.avail_for_put > 0):
            self.buffer[self.end] = newNote
            self.end = (self.end + 1) % self.bufferSize
        else:
            logging.error("Buffer full")
      
    #return nb of data available to get  
    def avail_for_get(self):
        return self.len()
     
   	#getting data from the buffer
    def get(self):
        copied = []       
        while(self.avail_for_get() > 0):
            copied.append(self.buffer[self.start])
            self.start = (self.start + 1 ) % self.bufferSize                
        
        return copied

