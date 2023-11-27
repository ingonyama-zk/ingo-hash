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
    for (int i = 0; i < 32; i++) {
        printf("%02x ", (unsigned char) hash.bytes[i]);
    }
    printf("\n");
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
    constexpr uint64_t HASH_SIZE = 32;
    constexpr uint64_t MSG_SIZE = 8192;
    constexpr uint32_t BATCH_SIZE = 32;
    // setup
    xrt::device device = xrt::device(0);
    xrt::uuid xclbin_uuid = init_device(device);

    xrt::kernel k_hash = xrt::kernel(device, xclbin_uuid, "kernel_m_axi_groestl_256");

    auto k_hash_gmem_msg_bank_group = k_hash.group_id(0);
    auto k_hash_gmem_hash_bank_group = k_hash.group_id(1);

    auto msg_buffer = xrt::bo(device, MSG_SIZE * BATCH_SIZE, k_hash_gmem_msg_bank_group);
    auto hash_buffer = xrt::bo(device, HASH_SIZE * BATCH_SIZE, k_hash_gmem_hash_bank_group);

    msg_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    std::cout << msg_buffer.size() << std::endl;
    std::cout << hash_buffer.size() << std::endl;
    std::cout << msg_buffer.address() << std::endl;
    std::cout << hash_buffer.address() << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    auto run = k_hash(msg_buffer, hash_buffer, MSG_SIZE, BATCH_SIZE);
    run.wait();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(stop - start).count();
    double total_gigabytes = static_cast<double>(MSG_SIZE * BATCH_SIZE) / (1ULL << 30);
    double throughput = total_gigabytes / duration;
    std::cout << "Throughput: " << throughput << " GiB/s" << std::endl;

    hash_buffer.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

    hash_256_t hashes[BATCH_SIZE] = {0};
    hash_buffer.read(hashes);

    for (int i = 0; i < BATCH_SIZE; i++) {
        std::cout << "hash[" << i << "]: ";
        print_hash_256(hashes[i]);
    }
}
