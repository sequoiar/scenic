#include <iostream>

#ifndef _LEVELS_H_
#define _LEVELS_H_

class VuMeter {
    public:
    VuMeter() : rms_(0.0) {};
    void print() { std::cout << "normalized rms value: " << rms_ << std::endl; }
    void updateRms(double newRms) { rms_ = newRms; }

    private:
    double rms_;
};

#endif // _LEVELS_H_
