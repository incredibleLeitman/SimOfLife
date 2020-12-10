# TODO: check os for platform dependent clean
#ifeq ($(OS), Windows_NT)
#	#EXECUTABLE=a.exe
#else
#	#EXECUTABLE=a.out
#endif

CXX = g++
CXXFLAGS = -Wall -O3 -fopenmp
LIB = -L"/mnt/c/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.1/lib/x64"
INC = -I"/mnt/c/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.1/include"

SimOfLife: SimOfLife.cpp Timing.cpp
	$(CXX) $(CXXFLAGS) $(INC) $(LIB) -o SimOfLife SimOfLife.cpp Timing.cpp -lOpenCL

clean:
	#del SimOfLife.exe SimOfLife
	rm -f *.o SimOfLife