XRT_PLATFORM ?= xilinx_u250_gen3x16_xdma_4_1_202210_1

m_axi_groestl_256:
	mkdir -p work
	cd work ; \
	v++ --link --platform $(XRT_PLATFORM) --save-temps ../pl/groestl/work/*.xo
