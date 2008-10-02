import logging
from midiObject import MidiNote

class myRingBuffer(object):
    def __init__(self, bufferSize=2048):
        self.bufferSize = bufferSize
        self.start = 0
        self.end = 0
        self.buffer = []
        for i in range (0, bufferSize):
            
            self.buffer.append(MidiNote(0,0,0,0))

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
    
    #Write Data in the ring buffer
    def put(self, newNote):
        #if there is enought place to insert new elements
        if ( self.avail_for_put > 0):
            #if the last midi time note is inferiror to the new note
            if ( self.buffer[self.end - 1].time <= newNote.time ):
                self.buffer[self.end] = newNote
                self.end = (self.end + 1) % self.bufferSize
            else:
                #else looking into the buffer to see where to insert it
                self.find_place(newNote)
        else:
            log.error("Buffer full")
      
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


    def find_place(self,note):
        """Find the correct place (time scheduled ) for the new note
        """

        g = -1
        #if its the first place
        if (note.time <= self.buffer[self.start].time):
            g = self.start
        else:
            #si end > start
            if self.end >= self.start :
                for i in range(self.end - 1, self.start-1, -1):
                
                    if ( self.buffer[i-1].time <= note.time and self.buffer[i].time >= note.time):
                        g = i                       
                        break
            else:
                #si end < start
                for i in range(self.end - 1, 0, -1):
                    print i 
                    if ( self.buffer[i-1].time <= note.time and self.buffer[i].time >= note.time):
                        g = i
                        break
            
                if g == -1 :
                    #print g
                    for i in range(self.bufferSize - 1, self.start-1, -1):
                        print i
                        if ( self.buffer[i-1].time <= note.time and self.buffer[i].time >= note.time):
                            g = i
                            break
            
            
        if ( g == -1):
            log.error("Can't find the correct place for the new note")
        else:
            
            #si end > start 
            #On decal les notes suivante de 1
            if ( self.end >= self.start ):
                for j in range(self.end, g, -1):
                    self.buffer[j] = self.buffer[j-1]                
            else:
                #end < start               
                for j in range(self.end, 0, -1):
                    self.buffer[j] = self.buffer[j-1]

                self.buffer[0] = self.buffer[self.bufferSize-1]

                for j in range(self.bufferSize-1, g, -1):
                    self.buffer[j] = self.buffer[j-1]                
                                
            #on insert la nouvelle note
            self.buffer[g] = note
            self.end = (self.end + 1 ) % self.bufferSize


if __name__ == "__main__":
    rb = myRingBuffer(10)
    rb.put(MidiNote(10,0,0,0))
    rb.put(MidiNote(12,0,0,0))
    rb.put(MidiNote(13,0,0,0))
    rb.put(MidiNote(14,0,0,0))
    rb.put(MidiNote(14,0,0,0))
    rb.put(MidiNote(14,0,0,0))
    rb.get()

    rb.put(MidiNote(15,0,0,0))
    rb.put(MidiNote(16,0,0,0))
    rb.put(MidiNote(18,0,0,0))
    rb.put(MidiNote(19,0,0,0))
    rb.put(MidiNote(20,0,0,0))
    rb.put(MidiNote(21,0,0,0))

    for i in range(0, rb.bufferSize):
        print rb.buffer[i].time

    rb.put(MidiNote(17,0,0,0))

    for i in range(0, rb.bufferSize):
        print rb.buffer[i].time

    print '----------------------------'
    for i in range(rb.start, rb.bufferSize-1):
        print rb.buffer[i].time

    for i in range(0, rb.end):
        print rb.buffer[i].time
