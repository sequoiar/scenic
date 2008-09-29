all:
	-cd public && ./autogen.sh && make -j4;
	-cd inhouse/public.unused && ./autogen.sh && make -j4;
