#ifndef __BASE_MESSAGE__
#define __BASE_MESSAGE__
/*
const char* str[] = 
{ 
	"undefined","err","ok","ack","open","close","start","stop",
	"pause","quit","info" 
};
*/

class BaseMessage
{
public:
 	enum type{ error =-2, undefined=-1,zero=0,ok='=',quit='Q',system='!',ping='.',string='s'};

	BaseMessage(unsigned short i):t_(static_cast<unsigned char>(i)){}
	BaseMessage(char i):t_(i){}
	BaseMessage(type t):t_(t){}
	BaseMessage():t_(0){}

	int get_int(){return static_cast<int>(t_);}
	char get_char(){return t_;}
	type get_type(){return static_cast<type>(t_);}
                           
	unsigned char t_;
}; 


#endif // __BASE_MESSAGE__
