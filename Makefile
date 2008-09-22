all:
	-cd public && ./autogen.sh && make -j;
	-cd inhouse/public.unused && ./autogen.sh && make -j;
