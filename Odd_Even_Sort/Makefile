CC = mpicc
CXX = mpicxx
CFLAGS = -O3
CXXFLAGS = -O3


#for test
test: test.o
	$(CXX) -o $@ $(CXXFLAGS) $^

#make for CPP cide
basic: basic.o
	$(CXX) -o $@ $(CXXFLAGS) $^

advanced: advanced.o
	$(CXX) -o $@ $(CXXFLAGS) $^
	
%.o: %.cc %.h
	$(CXX) -c $< $(CXXFLAGS)



$(OBJECTS): $(SOURCES) $(HEADERS)
	$(CXX) -c $< $(CXXFLAGS)
	
	
#make for C code
# basic: basic.o
	# $(CC) -o $@ $(CFLAGS) $< -lm

# %.o: %.c
	# $(CC) -c $< $(CFLAGS) -lm	
	
clean: 
	rm -rf *.o basic advanced test
