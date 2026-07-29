
CXX = g++

CXXFLAGS = -std=c++17 -O0 -Wall -g 
# CXXFLAGS = -std=c++17 -O2 -Wall -g -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-local-typedefs -Wpedantic -Wno-sign-compare -Wno-unused-function

HFLAGS = -I . 

all: musica

musica: mus_prog.o  mus_lib.o
	$(CXX) $(CXXFLAGS) -o musica mus_prog.o mus_lib.o 

mus_prog.o: mus_prog.cpp  mus_lib.hpp
	$(CXX) $(CXXFLAGS) -c mus_prog.cpp

mus_lib.o:  mus_lib.cpp mus_lib.hpp
	$(CXX) $(CXXFLAGS) -c mus_lib.cpp 

clean: 
	rm -f musica *.o 

