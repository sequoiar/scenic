
/*
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 *
 * http://www.sat.qc.ca
 * All rights reserved.
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _SHARED_VIDEO_BUFFER_H_
#define _SHARED_VIDEO_BUFFER_H_

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

namespace videosize
{
    const int WIDTH = 640;
    const int HEIGHT = 480;
    const int MAX_WIDTH = 1920;
    const int MAX_HEIGHT = 1080;
}

class SharedVideoBuffer
{
    public:
        static double ASPECT_RATIO;

        SharedVideoBuffer(int width, int height);

        boost::interprocess::interprocess_mutex & getMutex();

        unsigned char* pixelsAddress();

        void pushBuffer(unsigned char *newBuffer, size_t size);

        void notifyConsumer();

        void notifyProducer();

        bool waitOnConsumer(boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> &lock);

        // wait for buffer to be pushed if it's currently empty
        bool waitOnProducer(boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> &lock);

        int getWidth();
        int getHeight();

    private:

        enum {
            MAX_BUFFER_SIZE = videosize::MAX_WIDTH * videosize::MAX_HEIGHT * sizeof(short),
            BUFFER_SIZE = videosize::WIDTH * videosize::HEIGHT * sizeof(short)
        };

        // pixels to write to/read from
        unsigned char pixels[MAX_BUFFER_SIZE];

        // resolution
        const int width_;
        const int height_;

        // mutex to protect access to the queue
        boost::interprocess::interprocess_mutex mutex_;

        // condition to wait when the queue is empty
        boost::interprocess::interprocess_condition conditionEmpty_;

        // condition to wait when the queue is full
        boost::interprocess::interprocess_condition conditionFull_;

        // is there a buffer ready to be consumed
        // in our shared memory?
        bool bufferIn_;
};

// use in configure.ac like so:
// AC_CHECK_LIB([shared_video], [shared_video_is_present],
// AC_MSG_NOTICE([found libshared_video]), AC_MSG_ERROR([didn't find libshared_video]), [-pthread])

extern "C" {
    void shared_video_is_present();
}

#endif // _SHARED_VIDEO_BUFFER_H_
