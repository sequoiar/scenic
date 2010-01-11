APP=dc-ctl

all: $(APP)

clean:
	rm -f *.o $(APP)

$(APP): $(APP).cpp
	$(CXX) $< -o $@ `pkg-config --cflags --libs libdc1394-2 ` -lboost_program_options
