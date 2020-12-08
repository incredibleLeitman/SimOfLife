#ifeq ($(OS), Windows_NT)
#	#EXECUTABLE=a.exe
#else
#	#EXECUTABLE=a.out
#endif

CXX = g++
CXXFLAGS = -Wall -O3 -fopenmp #-lOpenCL
#TODO: https://stackoverflow.com/questions/7542808/how-to-compile-opencl-on-ubuntu
LIB = -L"/mnt/c/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.1/lib/x64"
INC = -I"/mnt/c/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.1/include"

default: main

main: SimOfLife.cpp Timing.cpp
	#$(CXX) $(CXXFLAGS) $(LIB) $(INC) -c SimOfLife.cpp Timing.cpp
	$(CXX) $(CXXFLAGS) $(LIB) $(INC) SimOfLife.cpp Timing.cpp

clean:
	#del SimOfLife.exe SimOfLife
	rm -f *.o SimOfLife