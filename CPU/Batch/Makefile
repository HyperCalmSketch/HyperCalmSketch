XXFLAGS = -o batch_test -g -lboost_program_options --std=c++17 -O3 $(USER_DEFINES)
ifeq ($(USER_DEFINES), -DSIMD)
    XXFLAGS += -mavx2 -mbmi -mavx512bw
endif

batch_test: main.cpp
	g++ main.cpp $(XXFLAGS)

clean: 
	rm batch_test