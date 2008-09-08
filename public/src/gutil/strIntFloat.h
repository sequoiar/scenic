
#ifndef __STR_INT_FLOAT_H__
#define __STR_INT_FLOAT_H__

#include <string>
#include <map>

class StrIntFloat;
typedef std::map<std::string, StrIntFloat> MapMsg;

class StrIntFloat
{
    char type_;
    std::string s_;
    int i_;
    float f_;

    public:
        StrIntFloat(std::string s)
            : type_('s'), s_(s), i_(0), f_(0.0){}
        StrIntFloat(int i)
            : type_('i'), s_(), i_(i), f_(0.0){}
        StrIntFloat(float f)
            : type_('f'), s_(), i_(0), f_(f){}

        StrIntFloat()
            : type_('n'), s_(), i_(0), f_(0.0){}

        char type() const { return type_;}
        bool get(std::string& s) const
        {
            if(type_ != 's')
                return false;
            s = s_;
            return true;
        }


        bool get(int& i) const
        {
            if(type_ != 'i')
                return false;
            i = i_;
            return true;
        }


        bool get(float& f) const
        {
            if(type_ != 'f')
                return false;
            f = f_;
            return true;
        }


        StrIntFloat(const StrIntFloat& sif_)
            : type_(sif_.type_), s_(sif_.s_), i_(sif_.i_), f_(sif_.f_){}
        StrIntFloat& operator=(const StrIntFloat& in)
        {
            if(this == &in)
                return *this;
            type_ = in.type_; s_ = in.s_; i_ = in.i_; f_ = in.f_;
            return *this;
        }
};

#endif
