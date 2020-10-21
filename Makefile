CXX = g++
CXXFLAGS = -Wall -O3

SimOfLife: SimOfLife.o Timing.o
	$(CXX) $(CXXFLAGS) -o SimOfLife SimOfLife.o Timing.o

main.o: SimOfLife.cpp.cpp Timing.h
	$(CXX) $(CXXFLAGS) -c SimOfLife.cpp

Timing.o: Timing.h

clean :
	-rm *.o SimOfLife