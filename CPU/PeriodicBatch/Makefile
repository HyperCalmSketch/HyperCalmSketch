XXFLAGS = -o periodic_batch_test -g -lboost_program_options --std=c++17 -O3 $(USER_DEFINES)
ifeq ($(USER_DEFINES), -DSIMD)
    XXFLAGS += -mavx2 -mbmi -mavx512bw
endif

periodic_batch_test: main.cpp
	g++ main.cpp $(XXFLAGS)

clean: 
	rm periodic_batch_test