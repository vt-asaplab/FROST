#ifndef __UTILS_H_
#define __UTILS_H_
#include <fstream>
#include <array>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <chrono>
#include <thread>
#include "MurmurHash3.hpp"
#include "encrypted_index.hpp"
#include "repir.hpp"

#define ENABLE_DEBUGGING

#define NUM_HASH_FUNC 7 

namespace fs = std::filesystem;

void handleErrors(void);
int prf(unsigned char *key, unsigned char *input, int input_len, unsigned char *output);
void convert_ZZ_p_to_uint64(uint64_t &uint64_value, const NTL::ZZ_p &ZZ_p_value);
void convert_uint64_to_ZZ_p(NTL::ZZ_p &ZZ_p_value, const uint64_t &scalar);
std::array<uint64_t, 2> mm_hash(const uint8_t* data, size_t len);
double time_from(const std::chrono::time_point<std::chrono::high_resolution_clock>& s);
void process_text_file(const fs::path& file_path, const size_t doc_count);
void list_and_read_text_files(const fs::path& folder_path);

inline uint64_t nth_hash(uint8_t i, uint64_t hash_a, uint64_t hash_b, uint64_t filter_size) {
    return (hash_a + i * hash_b) % filter_size;
}

inline std::chrono::time_point<std::chrono::high_resolution_clock> clock_start() { 
	return std::chrono::high_resolution_clock::now();
}

inline double time_from(const std::chrono::time_point<std::chrono::high_resolution_clock>& s) {
	return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - s).count();
}

#endif 
