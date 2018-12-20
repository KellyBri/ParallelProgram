NVFLAGS  := -std=c++11 -Xptxas=-v -arch=sm_61
CXXFLAGS := -fopenmp
LDFLAGS  := -lm
MPILIBS  := -I/opt/intel/compilers_and_libraries_2017.3.191/linux/mpi/intel64/include -L/opt/intel/compilers_and_libraries_2017.3.191/linux/mpi/intel64/lib -lmpi

TARGETS = apsp multi_gpu multi_node

.PHONY: all
all: $(TARGETS)

%: %.cu
	nvcc $(NVFLAGS) ${MPILIBS} -Xcompiler="$(CXXFLAGS)" $(LDFLAGS) -o $@ $?

.PHONY: clean
clean:
	rm -f $(TARGETS)
