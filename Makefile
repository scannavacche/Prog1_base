
CXX      = g++

# versione release
 CXXFLAGS = -std=c++17 -O2 -Wall 
 LDFLAGS  =

# versione debug
## CXXFLAGS = -std=c++17 -O0 -Wall -g -DDEBUG
## LDFLAGS  = -g

# versione silent, dopo espansione silenzia alcuni warning mirati (usati per FCN)
# CXXFLAGS = -std=c++17 -O2 -Wall -g -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-local-typedefs -Wpedantic -Wno-sign-compare -Wno-unused-function
# LDFLAGS  = -g

# LDLIBS   =
# HFLAGS = -I .

.PHONY: all clean

all: musica

musica: mus_prog.o  mus_csr.o mus_lib.o
	$(CXX) $(LDFLAGS) -o musica mus_prog.o mus_csr.o mus_lib.o 

mus_prog.o: mus_prog.cpp  mus_csr.hpp mus_lib.hpp
	$(CXX) $(CXXFLAGS) -c mus_prog.cpp

mus_csr.o:  mus_csr.cpp mus_csr.hpp mus_lib.hpp
	$(CXX) $(CXXFLAGS) -c mus_csr.cpp 

mus_lib.o:  mus_lib.cpp mus_lib.hpp
	$(CXX) $(CXXFLAGS) -c mus_lib.cpp 

clean: 
	rm -f musica *.o 

