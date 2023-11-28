XRT_PLATFORM ?= xilinx_u250_gen3x16_xdma_4_1_202210_1

pl/groestl/work/kernel_m_axi_groestl_256.xo:
	make -C ./pl/groestl/ csynth

work/$(XRT_PLATFORM)/m_axi_groestl_256/hw/m_axi_groestl_256.xclbin:
	mkdir -p ./work/$(XRT_PLATFORM)/m_axi_groestl_256/hw/
	cd ./work/$(XRT_PLATFORM)/m_axi_groestl_256/hw/ ; \
	v++ --link --target hw --platform $(XRT_PLATFORM) --save-temps ../../../../pl/groestl/work/*.xo -o m_axi_groestl_256.xclbin

work/$(XRT_PLATFORM)/m_axi_groestl_256/hw_emu/hw_emu_m_axi_groestl_256.xclbin:
	mkdir -p ./work/$(XRT_PLATFORM)/m_axi_groestl_256/hw_emu/
	cd ./work/$(XRT_PLATFORM)/m_axi_groestl_256/hw_emu/ ; \
	v++ --link --target hw_emu --platform $(XRT_PLATFORM) --save-temps ../../../../pl/groestl/work/*.xo -o hw_emu_m_axi_groestl_256.xclbin

clean:
	rm -rf work
	make -C ./pl/groestl/ clean
