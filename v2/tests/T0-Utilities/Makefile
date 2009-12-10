GTEST_HOME=..
PLATFORM=posix

CCFLAGS= -I$(GTEST_HOME)/include -I../../platform/$(PLATFORM)
LDFLAGS= -L$(GTEST_HOME)/lib -lgtest -lgtest_main

.SUFFIXES: .bin .o .cc

OBJS=../../util/Packet.o ../../util/DebugPacketProcessor.o

all: $(OBJS) T0.0.CircularBufferTest.bin T0.1.PacketTest.bin

../../util/Packet.o: ../../util/Packet.hh

../../util/DebugPacketProcessor.o: ../../util/DebugPacketProcessor.hh

.cc.o:
	g++ -c -o $*.o $(CCFLAGS) $<

.cc.bin: $(OBJS)
	g++ -c -o $*.o $(CCFLAGS) $<
	g++ -o $@ $(LDFLAGS) $*.o $(OBJS)

run: all
	LD_LIBRARY_PATH=$(GTEST_HOME)/lib ./T0.0.CircularBufferTest.bin
	LD_LIBRARY_PATH=$(GTEST_HOME)/lib ./T0.1.PacketTest.bin

clean:
	rm -f *.bin *.o
