SHARED_DIR=shared/include
GENERATED_DIR=generated
CC=g++
CFLAGS= -std=c++0x -c -Wall  -I$(SHARED_DIR) -I$(THRIFT_DIR) -I$(GENERATED_DIR)/..
PIC= -fPIC
LDFLAGS= 
# CBITSFLAGS is set to -m32 to build linux32 fmus
SOURCES=$(wildcard *.cpp)
GENERATED_SOURCES=$(wildcard generated/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
GENERATED_OBJECTS=$(GENERATED_SOURCES:.cpp=.o)
EXECUTABLE=ril_fmi.so

all: $(SOURCES) $(GENERATED_SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) $(GENERATED_OBJECTS)
	$(CC) $(LDFLAGS)  $(CBITSFLAGS) $(OBJECTS) $(GENERATED_OBJECTS) -fvisibility=hidden -shared -Wl,-soname,$@ -o $@

.cpp.o:
	$(CC) $(CBITSFLAGS) $(CFLAGS) $(PIC) $< -o $@

clean:
	rm -rf *.so *.o *~

