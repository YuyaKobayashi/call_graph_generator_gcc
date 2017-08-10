CFLAGS= -Wall -g -O0 -finstrument-functions -rdynamic
CXXFLAGS= -Wall -g -O0 -fPIC --std=c++11


TARGET=main libcall_tracer.so func_hist2dot

.PHONY:all 
all: $(TARGET)

%: %.o

lib%.so: %.o
	$(CXX)  -shared -fPIC -ldl -o $@ $^

.PHONY:clean
clean:
	rm -fr $(TARGET)


