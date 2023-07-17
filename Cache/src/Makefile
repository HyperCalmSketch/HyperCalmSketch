CPFLAGS = --std=c++17 -O3

1: main.cpp throughput.cpp
	g++ main.cpp -lboost_program_options -o cache_test -g $(CPFLAGS)
	g++ throughput.cpp -lboost_program_options -o cache_throughput -g $(CPFLAGS)
	

clean: 
	rm cache_test cache_throughput *.csv