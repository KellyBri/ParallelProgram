CC			:= gcc
CXX			:= g++
NVCC		:= nvcc

CCFLAGS		:= -O3 -march=native -Wall -std=gnu11
CXXFLAGS	:= -O3 -march=native -Wall -std=c++11
NVFLAGS		:= -arch=sm_61 -O3 -std=c++11 -Wno-deprecated-gpu-targets -use_fast_math
OMPFLAGS	:= -fopenmp

LIBS		:= -lpng

EXES		:= gray_sobel

all: $(EXES)

clean:
	rm -f $(EXES)
gray_sobel: gray_sobel.cu
	$(NVCC) $(NVFLAGS) $(LIBS) -o $@ $?
