XXFLAGS = -g -lboost_program_options --std=c++17 -O3 $(USER_DEFINES)
ifeq ($(USER_DEFINES), -DSIMD)
    XXFLAGS += -mavx2 -mbmi -mavx512bw
endif

obj := batch_test
ifeq ($(OBJ_LOCAL), 1)
	obj := ../dst/$(obj)
endif

$(obj): main.cpp parse.cpp 
	g++ $^ $(XXFLAGS) -o $@

clean: 
	rm $(obj)
