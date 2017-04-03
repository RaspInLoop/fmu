SHARED_DIR=shared/include
GENERATED_DIR=gen-cpp
CC=g++
CFLAGS= -std=c++0x -c -Wall -I$(GENERATED_DIR) -I$(SHARED_DIR) -I$(THRIFT_DIR)/include 
PIC= -fPIC
LDFLAGS= -L$(THRIFT_DIR)/lib/
# CBITSFLAGS is set to -m32 to build linux32 fmus

THRIFT_DEFINITION=ril.thrift
SOURCES=$(wildcard *.cpp) $(filter-out $(wildcard $(GENERATED_SOURCES)/*_server.skeleton.cpp), $(wildcard $(GENERATED_SOURCES)/*.cpp))
GENERATED_SOURCES=gen-cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=ril_fmi.so

all: $(GENERATED_SOURCES) $(SOURCES)  $(EXECUTABLE)

$(GENERATED_SOURCES): $(THRIFT_DEFINITION)
	$(THRIFT_DIR)/bin/thrift -r --gen cpp $?

%.o : %.cpp	
	$(CC) $(CBITSFLAGS) $(CFLAGS) $(PIC) $< -o $@

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS)  $(CBITSFLAGS) $(OBJECTS) -fvisibility=hidden -Wl,-Bstatic -lthrift -Wl,-Bdynamic  -shared -Wl,-soname,$@ -o $@

clean:
	rm -rf gen-cpp
	rm -rf *.so *.o *~
