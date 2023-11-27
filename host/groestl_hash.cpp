#include <iostream>
#include <cassert>
#include <xrt.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_bo.h>
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_aie.h>
#include <xrt/xrt_graph.h>
#include <xrt/xrt_uuid.h>
#include <experimental/xrt_xclbin.h>
#include <experimental/xrt_ip.h>

struct hash_256_t {
    char bytes[64];
};

static void print_hash_256(hash_256_t hash) {
    for (int i = 0; i < 64; i++) {
        std::cout << hash.bytes[i];
    }
    std::cout << std::endl;
}

static xrt::uuid init_device(xrt::device &device) {
    std::string xclbin_file;
    char *env_emu;
    if (env_emu = getenv("XCL_EMULATION_MODE")) {
        std::string mode(env_emu);
        if (mode == "hw_emu") {
            std::cout << "Program running in hardware emulation mode" << std::endl;
            xclbin_file = "emu.xclbin";
        } else {
            assert("[ERROR] Unsupported Emulation Mode: ");
        }
    } else {
        std::cout << "Program running in hardware mode" << std::endl;
        xclbin_file = "hw.xclbin";
    }
    xrt::uuid xclbin_uuid = device.load_xclbin(xclbin_file);
    return xclbin_uuid;
}

int main() {
    constexpr uint64_t HASH_SIZE = 64;
    constexpr uint64_t MSG_SIZE = 8192;
    constexpr uint32_t BATCH_SIZE = 32;
    // setup
    xrt::device device = xrt::device(0);
    xrt::uuid xclbin_uuid = init_device(device);

    xrt::kernel k_hash = xrt::kernel(device, xclbin_uuid, "k_m_axi_groestl_256_0");
    xrt::run run_k_hash(k_hash);

    auto k_hash_bank_group = k_hash.group_id(0);

    auto msg_buffer = xrt::bo(device, MSG_SIZE * BATCH_SIZE, k_hash_bank_group);
    auto hash_buffer = xrt::bo(device, HASH_SIZE * BATCH_SIZE, k_hash_bank_group);

    msg_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    run_k_hash.set_arg(0, msg_buffer);
    run_k_hash.set_arg(1, hash_buffer);
    run_k_hash.set_arg(2, MSG_SIZE);
    run_k_hash.set_arg(3, BATCH_SIZE);

    run_k_hash.start();
    run_k_hash.wait();

    hash_buffer.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

    hash_256_t hashes[BATCH_SIZE];
    hash_buffer.read(hashes);

    for (int i = 0; i < BATCH_SIZE; i++) {
        print_hash_256(hashes[i]);
    }
}
