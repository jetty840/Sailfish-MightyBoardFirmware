TARGET=T1-UART-exerciser
PROJ_DIR=../../..
PLATFORM=posix
CXXFLAGS=-I$(PROJ_DIR)/platform/$(PLATFORM) -I$(PROJ_DIR)

OBJS=$(TARGET).o $(PROJ_DIR)/util/Packet.o

all: $(TARGET)

clean:
	rm -f $(OBJS)
	
# Compile: create object files from C++ source files.
.cc.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@ 

$(TARGET): $(OBJS)
	g++ $(CXXFLAGS) -o $(TARGET) $^

