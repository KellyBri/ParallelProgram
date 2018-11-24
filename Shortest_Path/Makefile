CXX = mpicxx 
LIBS = -pthread -fopenmp
FLAGS = -O3 -fopenmp 
TARGETS = sssp apsp partition

all: $(TARGETS)

%: %.cc
	$(CXX) -o $@ $(FLAGS) $^ $(LIBS)

clean:
	rm -f $(TARGETS)
