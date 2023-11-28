
compile:
	g++ -g -std=c++17 -I$(XILINX_XRT)/include -L$(XILINX_XRT)/lib -o groestl_hash groestl_hash.cpp -lxrt_coreutil -pthread

test: compile
	./groestl_hash
