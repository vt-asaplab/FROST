#ifndef __REPIR_H_
#define __REPIR_H_
#include <NTL/mat_ZZ.h>
#include <NTL/vec_ZZ.h>
#include <NTL/mat_ZZ_p.h>
#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/BasicThreadPool.h>
#include <random>
#include "utils.hpp"

#define NUM_THREADS 8 

typedef struct {
    NTL::Vec<NTL::ZZ> q;
    NTL::Vec<NTL::ZZ> sp;
    NTL::Vec<NTL::ZZ> m;
    NTL::Vec<NTL::ZZ> r;
} query_t;

NTL::Mat<NTL::ZZ_p> create_random_matrix(long rows, long cols, const NTL::ZZ &m);
std::vector<std::vector<uint64_t>> multiply_matrices(const std::vector<std::vector<uint32_t>>& A, const NTL::mat_ZZ_p& B, const NTL::ZZ& m);
NTL::mat_ZZ_p multiply_matrices(const NTL::mat_ZZ_p& A, const NTL::mat_ZZ_p& B, const NTL::ZZ& m);
NTL::mat_ZZ_p subtract_matrices(const NTL::mat_ZZ_p& A, const NTL::mat_ZZ_p& B, const NTL::ZZ& m);
std::vector<std::vector<uint64_t>> subtract_matrices(const std::vector<std::vector<uint64_t>>& A, const NTL::mat_ZZ_p& B, const NTL::ZZ& m);
NTL::vec_ZZ vect_scalar_multiply(const NTL::vec_ZZ& v, const NTL::ZZ& scalar, const NTL::ZZ& m);
NTL::vec_ZZ add_vectors(const NTL::vec_ZZ& v1, const NTL::vec_ZZ& v2, const NTL::ZZ& m);
void fillMatRange(NTL::mat_ZZ_p& mat, const std::vector<std::vector<uint32_t>>& vec, long startRow, long endRow);
NTL::mat_ZZ_p vectorToMatZZ_p(const std::vector<std::vector<uint32_t>>& vec, int numThreads = NUM_THREADS);
void vec_ZZ_p_to_vec_u64(std::vector<uint64_t>& out, const NTL::vec_ZZ_p& vec, const NTL::ZZ &m);
void matToVectorWorker(const NTL::mat_ZZ_p& mat, std::vector<std::vector<uint64_t>>& out, const NTL::ZZ &m, long start_row, long end_row);
std::vector<std::vector<uint64_t>> matZZpToVector(const NTL::mat_ZZ_p& mat, const NTL::ZZ &m, int numThreads = NUM_THREADS);
NTL::ZZ dot_product(const NTL::Vec<NTL::ZZ>& a, const NTL::Vec<NTL::ZZ>& b, const NTL::ZZ& m);
void multiply_worker(NTL::Vec<NTL::ZZ>& result, const NTL::Mat<NTL::ZZ_p>& A, const NTL::Vec<NTL::ZZ>& v, const NTL::ZZ& q, long start_row, long end_row);
NTL::Vec<NTL::ZZ> matrix_vector_multiply(const NTL::Mat<NTL::ZZ_p>& A, const NTL::Vec<NTL::ZZ>& v, const NTL::ZZ& q, int num_threads = NUM_THREADS);
void vector_matrix_multiply_worker(NTL::Vec<NTL::ZZ_p>& result, const NTL::Mat<NTL::ZZ_p>& A, const NTL::Vec<NTL::ZZ_p>& v, const NTL::ZZ& q, long start_col, long end_col);
NTL::Vec<NTL::ZZ_p> vector_matrix_multiply(const NTL::Vec<NTL::ZZ_p>& v, const NTL::Mat<NTL::ZZ_p>& A, const NTL::ZZ& q, int num_threads = NUM_THREADS);
void vec_u32_to_vec_ZZ_p(NTL::vec_ZZ_p& vec_ZZ_p, const std::vector<uint32_t>& vec_u32, const NTL::ZZ &m);
void mat_u64_to_mat_ZZ_p_worker(NTL::Mat<NTL::ZZ_p>& mat, const std::vector<std::vector<uint64_t>>& vec, const NTL::ZZ &m, long start_row, long end_row);
NTL::Mat<NTL::ZZ_p> mat_u64_to_mat_ZZ_p(const std::vector<std::vector<uint64_t>>& vec, const NTL::ZZ &m, int numThreads = NUM_THREADS);
NTL::vec_ZZ subtract_vectors(const NTL::vec_ZZ& v1, const NTL::vec_ZZ& v2, const NTL::ZZ& m);
NTL::vec_ZZ_p subtract_vectors(const NTL::vec_ZZ_p& v1, const NTL::vec_ZZ_p& v2, const NTL::ZZ& m);
std::vector<double> divide_vector_by_scalar(const NTL::vec_ZZ& v, double scalar);
std::vector<uint64_t> rounding(const std::vector<double>& v);
std::vector<uint64_t> flooring(const std::vector<double>& v);
std::vector<uint64_t> ceiling(const std::vector<double>& v);
NTL::vec_ZZ mod_vector(const NTL::vec_ZZ& v, const NTL::ZZ& m);
NTL::mat_ZZ lshift_matrix(const NTL::mat_ZZ& matrix);
NTL::vec_ZZ lshift_vector(const NTL::vec_ZZ& v);
NTL::vec_ZZ rshift_vector(const NTL::vec_ZZ& v);
NTL::vec_ZZ to_vec_ZZ(const std::vector<uint64_t>& v);
NTL::vec_ZZ to_vec_ZZ(const std::vector<double>& v);
NTL::vec_ZZ to_vec_ZZ_double(const std::vector<double>& v);
void print_vec(const NTL::Vec<NTL::ZZ>& v, const std::string& name);
void print_mat(const NTL::Mat<NTL::ZZ>& mat, const std::string& name);
NTL::Mat<NTL::ZZ> to_MatZZ(const NTL::Mat<NTL::ZZ_p>& A);
void gen_params();
query_t create_query(const int queried_index, int sub_index);
NTL::Vec<NTL::ZZ> execute_query(const query_t &query, const int partition_index, int sub_index);

#endif 
