CC=gcc
CXX=g++
CPPFLAGS=
LDLIBS= -lOpenCL

all: CPPFLAGS += -O2
all: nbs

debug: CPPFLAGS += -DDEBUG
debug: nbs

nbs:
	$(CXX) $(CPPFLAGS) -o nbs NBS.cpp Engine.cpp $(LDLIBS)

clean:
	rm nbs

