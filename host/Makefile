
compile:
	g++ groestl_hash.cpp -std=c++17 -I${XILINX_XRT}/include -lxrt_coreutil -L${XILINX_XRT}/lib -o groestl_hash

bench: compile
	./groestl_hash