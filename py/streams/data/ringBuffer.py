import logging
from midiObject import MidiNote

class myRingBuffer(object):
    def __init__(self):
        self.start = 0
        self.end = 0
        self.buffer = []
    
    #get the total len of the ring    
    def len(self):
        return len(self.buffer)

    #Write Data in the ring buffer
    def put(self, newNote):
        if len(self.buffer) > 0 :
            if self.buffer[-1][1] <= newNote[1]:
                #checking if the note is in the right place
                self.buffer.append(newNote)
            else:
                #sinon on regarde ou il faut l inserer
                if newNote[1] <= self.buffer[0][1]:
                    self.buffer.insert(0,newNote)
                else:
                    place = [ind for ind, obj in enumerate(self.buffer) if obj[1] < newNote[1]][-1]
                    self.buffer.insert(place + 1, newNote)

        else:
            #simply adding the note to the buffer
            self.buffer.append(newNote)
      

    #getting data from the buffer
    def get(self):            
        copied = self.buffer
        self.buffer = []                
        return copied


    def get_data(self,time):
        data = []
        ind = []
        #attention part du principe que le buffer est trier en ordre croissant
        for i in range(len(self.buffer)):
            if self.buffer[i][1] <= time + 4:
                data.append(self.buffer[i])
                ind.append(i)
        for i in range(len(ind)):
            self.buffer.pop(0)
        
        return data

if __name__ == "__main__":
    r = myRingBuffer()
    r.put([['ob',1,1],10])
    r.put([['ob',1,1],14])
    r.put([['ob',1,1],12])
    print r.buffer
