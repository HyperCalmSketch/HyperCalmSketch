CPFLAGS = -g -rdynamic -mavx2 -mbmi -mavx512bw -mavx512dq --std=c++17 -O3

topk_test: main.cpp
	g++ main.cpp -o topk_test -g -lboost_program_options $(CPFLAGS) 

clean: 
	rm topk_test