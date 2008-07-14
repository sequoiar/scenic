#include "gstThread.h"
#include "osc/osc.h"
#include "gutil/optionArgs.h"

int main (int argc, char** argv)
{
	GstThread t;
	OscThread o;
	OptionArgs opts;

	opts.add(t.get_args());
	opts.add(o.get_args());

	if(!opts.parse(argc,argv))
		return 1;

	QueuePair queue = t.getQueue("");
	if(!t.run())
		return -1;

	BaseMessage in(BaseMessage::init);
	queue.push(in);
	while(1)
	{
		BaseMessage start(BaseMessage::start);
		queue.push(start);
		usleep(1000000);

		BaseMessage stop(BaseMessage::stop);
		queue.push(stop);
		usleep(1000000);

		BaseMessage f = queue.copy_timed_pop(1);

		if(f.get_type() == BaseMessage::quit) {
			break;
		}



	}

	std::cout << "Done!" << std::endl;

	return 0;
}


