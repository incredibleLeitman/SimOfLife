CXX = g++
CXXFLAGS = -Wall -O3 -fopenmp

SimOfLife: SimOfLife.cpp Timing.cpp

clean:
	del SimOfLife.exe SimOfLife
	#-rm -f *.o SimOfLife
