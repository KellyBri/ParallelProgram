CC = mpicc
CXX = mpicxx
LDLIBS = -lpng 
CFLAGS = -O3 -fopenmp
CXXFLAGS = -O3 -fopenmp


mpi_static: mpi_static.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LDLIBS)

#make for CPP cide
mpi_dynamic: mpi_dynamic.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LDLIBS)

openmp: openmp.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LDLIBS)

hybrid: hybrid.o
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LDLIBS)
	
%.o: %.cc
	$(CXX) -c $< $(CXXFLAGS) $(LDLIBS)

	
clean: 
	rm -rf *.o hybrid openmp mpi_dynamic mpi_static
