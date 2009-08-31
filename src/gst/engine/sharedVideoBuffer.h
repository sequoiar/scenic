#ifndef _SHARED_VIDEO_BUFFER_H_
#define _SHARED_VIDEO_BUFFER_H_

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

class SharedVideoBuffer
{
    public:
        static const int WIDTH = 640;
        static const int HEIGHT = 480;
        static const double ASPECT_RATIO;

        SharedVideoBuffer();

        boost::interprocess::interprocess_mutex & getMutex();

        unsigned char* pixelsAddress();
        
        bool isPushing() const;
        
        void pushBuffer(unsigned char *newBuffer, size_t size);

        void stopPushing();
       
        void startPushing();
        
        void notifyConsumer();

        void notifyProducer();

        void waitOnConsumer(boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> &lock);

        // wait for buffer to be pushed if it's currently empty
        void waitOnProducer(boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> &lock);

    private:

        enum { BUFFER_SIZE = WIDTH * HEIGHT * sizeof(short)};

        // pixels to write to/read from
        unsigned char pixels[BUFFER_SIZE];

        // mutex to protect access to the queue 
        boost::interprocess::interprocess_mutex mutex_;

        // condition to wait when the queue is empty
        boost::interprocess::interprocess_condition conditionEmpty_;

        // condition to wait when the queue is full 
        boost::interprocess::interprocess_condition conditionFull_;

        // is there a buffer ready to be consumed
        // in our shared memory? 
        bool bufferIn_;

        // has either process signalled that it wants to quit?
        bool doPush_;
};

#endif // _SHARED_VIDEO_BUFFER_H_
