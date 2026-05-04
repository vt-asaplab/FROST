#include "repir.hpp"

extern int l;
extern NTL::ZZ pp, p;
extern unsigned char k1[32], k2[32];
extern std::vector<std::vector<std::vector<std::vector<uint32_t>>>> encrypted_search_index;
extern size_t BF_size, max_docs;
extern std::vector<uint32_t> state; 

long n;
NTL::ZZ q;
NTL::ZZ Delta;

std::vector<std::vector<NTL::Mat<NTL::ZZ_p>>> A;
std::vector<std::vector<NTL::Mat<NTL::ZZ_p>>> B, iB;
std::vector<std::vector<NTL::Mat<NTL::ZZ_p>>> C;
// std::vector<std::vector<NTL::Mat<NTL::ZZ_p>>> hint;
std::vector<std::vector<std::vector<std::vector<uint64_t>>>> hint;

NTL::Mat<NTL::ZZ_p> create_random_matrix(long rows, long cols, const NTL::ZZ &m) {
    // NTL::Mat<NTL::ZZ> mat;
    // mat.SetDims(rows, cols);
    NTL::ZZ_pContext ctx(m);
    ctx.restore();
    NTL::Mat<NTL::ZZ_p> mat;
    mat.SetDims(rows, cols);
    for (long i = 0; i < rows; ++i) {
        for (long j = 0; j < cols; ++j) {
            // mat[i][j] = RandomBnd(max_val - min_val + 1) + min_val;
            random(mat[i][j]);
        }
    }
    return mat;
}

std::vector<std::vector<uint64_t>>
multiply_matrices(
    const std::vector<std::vector<uint32_t>>& A,
    const NTL::mat_ZZ_p& B,
    const NTL::ZZ& m
) {
    if (A.empty() || A[0].empty()) {
        throw std::invalid_argument("Matrix A is empty.");
    }

    if (static_cast<long>(A[0].size()) != B.NumRows()) {
        throw std::invalid_argument("Matrix dimensions incompatible.");
    }

    long n = static_cast<long>(A.size());      
    long k = static_cast<long>(A[0].size());   
    long p = B.NumCols();                    

    std::vector<std::vector<uint64_t>> C(
        n, std::vector<uint64_t>(p, 0)
    );

    auto worker = [&](long start_row, long end_row) {
        NTL::ZZ_pContext ctx(m);
        ctx.restore();
        for (long i = start_row; i < end_row; i++) {
            for (long j = 0; j < p; j++) {
                NTL::ZZ_p acc = NTL::ZZ_p(0);
                for (long t = 0; t < k; t++) {
                    NTL::ZZ_p a = NTL::conv<NTL::ZZ_p>(A[i][t]);
                    acc += a * B[t][j];
                }
                convert_ZZ_p_to_uint64(C[i][j], acc);
            }
        }
    };

    std::vector<std::thread> threads;
    long chunk_size = (n + NUM_THREADS - 1) / NUM_THREADS;
    for (long t = 0; t < NUM_THREADS; t++) {
        long start_row = t * chunk_size;
        long end_row = std::min(start_row + chunk_size, n);
        if (start_row >= n) break;
        threads.emplace_back(worker, start_row, end_row);
    }
    for (auto& th : threads) th.join();
    /*
    for (long i = 0; i < n; i++) {
        for (long j = 0; j < p; j++) {
            NTL::ZZ_p acc = NTL::ZZ_p(0);
            for (long t = 0; t < k; t++) {
                NTL::ZZ_p a = NTL::conv<NTL::ZZ_p>(A[i][t]);
                acc += a * B[t][j];
            }
            convert_ZZ_p_to_uint64(C[i][j], acc);
        }
    }*/
    return C;
}

NTL::mat_ZZ_p multiply_matrices(const NTL::mat_ZZ_p& A, const NTL::mat_ZZ_p& B, const NTL::ZZ& m) {
    if (A.NumCols() != B.NumRows()) {
        throw std::invalid_argument("Matrix dimensions incompatible.");
    }
    // NTL::ZZ_p::init(m);
    NTL::ZZ_pContext ctx(m);
    ctx.restore();
    NTL::mat_ZZ_p C;
    mul(C, A, B);
    return C;
}

NTL::mat_ZZ_p subtract_matrices(const NTL::mat_ZZ_p& A, const NTL::mat_ZZ_p& B, const NTL::ZZ& m) {
    if (A.NumRows() != B.NumRows() || A.NumCols() != B.NumCols()) {
        throw std::invalid_argument("Matrix dimensions must match.");
    }
    NTL::mat_ZZ_p C = A - B;
    return C;
}

std::vector<std::vector<uint64_t>>
subtract_matrices(
    const std::vector<std::vector<uint64_t>>& A,
    const NTL::mat_ZZ_p& B,
    const NTL::ZZ& m
) {
    if (A.empty() || A[0].empty()) {
        throw std::invalid_argument("Matrix A is empty.");
    }

    if (static_cast<long>(A.size()) != B.NumRows() ||
        static_cast<long>(A[0].size()) != B.NumCols()) {
        throw std::invalid_argument("Matrix dimensions must match.");
    }

    NTL::ZZ_pContext ctx(m);
    ctx.restore();
    
    long rows = B.NumRows();
    long cols = B.NumCols();

    std::vector<std::vector<uint64_t>> C(
        rows, std::vector<uint64_t>(cols, 0)
    );

    NTL::ZZ_p a;
    for (long i = 0; i < rows; i++) {
        for (long j = 0; j < cols; j++) {
            convert_uint64_to_ZZ_p(a, A[i][j]);
            NTL::ZZ_p c = a - B[i][j];
            convert_ZZ_p_to_uint64(C[i][j], c);
        }
    }
    return C;
}

NTL::vec_ZZ vect_scalar_multiply(const NTL::vec_ZZ& v, const NTL::ZZ& scalar, const NTL::ZZ& m) {
    NTL::vec_ZZ result;
    result.SetLength(v.length());
    for (long i = 0; i < v.length(); ++i)
        result[i] = (v[i] * scalar) % m;
    return result;
}

NTL::vec_ZZ add_vectors(const NTL::vec_ZZ& v1, const NTL::vec_ZZ& v2, const NTL::ZZ& m) {
    if (v1.length() != v2.length()) {
        throw std::invalid_argument("Vectors must have same length.");
    }
    NTL::vec_ZZ result;
    result.SetLength(v1.length());
    for (long i = 0; i < v1.length(); ++i)
        result[i] = (v1[i] + v2[i]) % m;
    return result;
}

void fillMatRange(NTL::mat_ZZ_p& mat, const std::vector<std::vector<uint32_t>>& vec,
                  long startRow, long endRow) {
    NTL::ZZ_pContext ctx(p);
    ctx.restore();
    for (long i = startRow; i < endRow; ++i) {
        long nCols = vec[i].size();
        for (long j = 0; j < nCols; ++j) {
            mat[i][j] = NTL::ZZ_p(vec[i][j]);
        }
    }
}

NTL::mat_ZZ_p vectorToMatZZ_p(const std::vector<std::vector<uint32_t>>& vec, int numThreads) {
    NTL::ZZ_pContext ctx(p);
    ctx.restore();

    long nRows = vec.size();
    long nCols = vec.empty() ? 0 : vec[0].size();

    NTL::mat_ZZ_p mat;
    mat.SetDims(nRows, nCols);

    long rowsPerThread = (nRows + numThreads - 1) / numThreads; 
    std::vector<std::thread> threads;

    for (int t = 0; t < numThreads; ++t) {
        long startRow = t * rowsPerThread;
        long endRow = std::min(startRow + rowsPerThread, nRows);
        if (startRow >= endRow) break;  
        threads.emplace_back(fillMatRange, std::ref(mat), std::ref(vec), startRow, endRow);
    }
    for (auto& th : threads) {
        th.join();
    }
    return mat;
}

void vec_ZZ_p_to_vec_u64(std::vector<uint64_t>& out,
                        const NTL::vec_ZZ_p& vec,
                        const NTL::ZZ &m) {
    NTL::ZZ_pContext ctx(q);
    ctx.restore();
    long n = vec.length();
    for (long i = 0; i < n; ++i) {
        convert_ZZ_p_to_uint64(out[i], vec[i]);
    }
}

void matToVectorWorker(const NTL::mat_ZZ_p& mat,
                       std::vector<std::vector<uint64_t>>& out,
                       const NTL::ZZ &m,
                       long start_row,
                       long end_row) {
    NTL::ZZ_pContext ctx(m);
    ctx.restore();
    long nCols = mat.NumCols();
    for (long i = start_row; i < end_row; ++i) {
        for (long j = 0; j < nCols; ++j) {
            convert_ZZ_p_to_uint64(out[i][j], mat[i][j]);
        }
    }
}

std::vector<std::vector<uint64_t>>
matZZpToVector(const NTL::mat_ZZ_p& mat, const NTL::ZZ &m, int numThreads) {
    long nRows = mat.NumRows();
    long nCols = mat.NumCols();
    std::vector<std::vector<uint64_t>> result(
        nRows, std::vector<uint64_t>(nCols));

    long rowsPerThread = (nRows + numThreads - 1) / numThreads;
    std::vector<std::thread> threads;

    for (int t = 0; t < numThreads; ++t) {
        long start = t * rowsPerThread;
        long end   = std::min(start + rowsPerThread, nRows);
        if (start >= end) break;
        threads.emplace_back(matToVectorWorker,
                             std::cref(mat),
                             std::ref(result),
                             m,
                             start,
                             end);
    }
    for (auto& th : threads) {
        th.join();
    }
    return result;
}

NTL::ZZ dot_product(const NTL::Vec<NTL::ZZ>& a, const NTL::Vec<NTL::ZZ>& b, const NTL::ZZ& m) {
    if (a.length() != b.length()) {
        throw std::invalid_argument("Vectors must have same length.");
    }
    NTL::ZZ result;
    clear(result);  
    for (long i = 0; i < a.length(); ++i) {
        result = (result + a[i] * b[i]) % m;
    }
    return result;
}

void multiply_worker(NTL::Vec<NTL::ZZ>& result, const NTL::Mat<NTL::ZZ_p>& A, 
    const NTL::Vec<NTL::ZZ>& v, const NTL::ZZ& q, long start_row, long end_row) {
    NTL::ZZ_pContext ctx(q);
    ctx.restore();
    long cols = A.NumCols();
    NTL::Vec<NTL::ZZ_p> vp;
    NTL::conv(vp, v);
    for (long i = start_row; i < end_row; ++i) {
        NTL::ZZ_p sum = NTL::ZZ_p(0);
        for (long j = 0; j < cols; ++j) {
            sum += A[i][j] * vp[j];
        }
        NTL::conv(result[i], sum);
    }
}

NTL::Vec<NTL::ZZ> matrix_vector_multiply(const NTL::Mat<NTL::ZZ_p>& A, const NTL::Vec<NTL::ZZ>& v, 
    const NTL::ZZ& q, int num_threads) { 
    long rows = A.NumRows();
    NTL::Vec<NTL::ZZ> result;
    result.SetLength(rows);
    
    NTL::ZZ_pContext ctx(q);
    ctx.restore();
    
    std::vector<std::thread> threads;
    long chunk_size = (rows + num_threads - 1) / num_threads;

    for (int t = 0; t < num_threads; ++t) {
        long start_row = t * chunk_size;
        long end_row = std::min(start_row + chunk_size, rows);
        threads.emplace_back(multiply_worker, std::ref(result), 
                    std::cref(A), std::cref(v), std::cref(q), start_row, end_row);
    }
    for (auto& th : threads) {
        th.join();
    }

    return result;
}

void vector_matrix_multiply_worker(
    NTL::Vec<NTL::ZZ_p>& result,
    const NTL::Mat<NTL::ZZ_p>& A,
    const NTL::Vec<NTL::ZZ_p>& v,
    const NTL::ZZ& q,
    long start_col,
    long end_col) {
    NTL::ZZ_pContext ctx(q);
    ctx.restore();
    long rows = A.NumRows();
    for (long j = start_col; j < end_col; ++j) {
        result[j] = NTL::ZZ_p(0);
        for (long i = 0; i < rows; ++i) {
            result[j] += v[i] * A[i][j];
        }
    }
}

NTL::Vec<NTL::ZZ_p> vector_matrix_multiply(
    const NTL::Vec<NTL::ZZ_p>& v,
    const NTL::Mat<NTL::ZZ_p>& A,
    const NTL::ZZ& q,
    int num_threads) {
    long rows = A.NumRows();
    long cols = A.NumCols();
    if (v.length() != rows) {
        throw std::invalid_argument("vector length must equal the row number of matrix");
    }
    NTL::Vec<NTL::ZZ_p> result;
    result.SetLength(cols);
    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    long chunk_size = (cols + num_threads - 1) / num_threads;
    for (int t = 0; t < num_threads; ++t) {
        long start_col = t * chunk_size;
        long end_col = std::min(start_col + chunk_size, cols);
        if (start_col < end_col) {
            threads.emplace_back(
                vector_matrix_multiply_worker,
                std::ref(result),
                std::cref(A),
                std::cref(v),
                std::cref(q),
                start_col,
                end_col
            );
        }
    }
    for (auto& th: threads) {
        th.join();
    }
    return result;
}

void vec_u32_to_vec_ZZ_p(NTL::vec_ZZ_p& vec_ZZ_p,
                        const std::vector<uint32_t>& vec_u32,
                        const NTL::ZZ &m) {
    NTL::ZZ_pContext ctx(m);
    ctx.restore();
    vec_ZZ_p.SetLength(vec_u32.size());
    for (long i = 0; i < vec_u32.size(); ++i) {
        vec_ZZ_p[i] = NTL::ZZ_p(vec_u32[i]);
    }
}

void mat_u64_to_mat_ZZ_p_worker(NTL::Mat<NTL::ZZ_p>& mat,
                        const std::vector<std::vector<uint64_t>>& vec,
                        const NTL::ZZ &m,
                        long start_row,
                        long end_row) {
    NTL::ZZ_pContext ctx(m);
    ctx.restore();
    for (long i = start_row; i < end_row; ++i) {
        long nCols = vec[i].size();
        for (long j = 0; j < nCols; ++j) {
            convert_uint64_to_ZZ_p(mat[i][j], vec[i][j]); 
        }
    }
}

NTL::Mat<NTL::ZZ_p> mat_u64_to_mat_ZZ_p(const std::vector<std::vector<uint64_t>>& vec, 
                                        const NTL::ZZ &m,
                                        int numThreads) {
    long nRows = vec.size();
    long nCols = vec.empty() ? 0 : vec[0].size();
    NTL::Mat<NTL::ZZ_p> mat;
    mat.SetDims(nRows, nCols);

    long rowsPerThread = (nRows + numThreads - 1) / numThreads;
    std::vector<std::thread> threads;

    for (int t = 0; t < numThreads; ++t) {
        long start = t * rowsPerThread;
        long end   = std::min(start + rowsPerThread, nRows);
        if (start >= end) break;
        threads.emplace_back(mat_u64_to_mat_ZZ_p_worker,
                             std::ref(mat),
                             std::cref(vec),
                             m,
                             start,
                             end);
    }
    for (auto& th: threads) {
        th.join();
    }
    return mat;
}

NTL::vec_ZZ subtract_vectors(const NTL::vec_ZZ& v1, const NTL::vec_ZZ& v2, const NTL::ZZ& m) {
    if (v1.length() != v2.length()) {
        throw std::invalid_argument("Vectors must have same length.");
    }
    NTL::vec_ZZ result;
    result.SetLength(v1.length());
    for (long i = 0; i < v1.length(); ++i)
        result[i] = (v1[i] - v2[i]) % m;
    return result;
}

NTL::vec_ZZ_p subtract_vectors(const NTL::vec_ZZ_p& v1, const NTL::vec_ZZ_p& v2, const NTL::ZZ& m) {
    NTL::ZZ_pContext ctx(q);
    ctx.restore();
    if (v1.length() != v2.length()) {
        throw std::invalid_argument("Vectors must have same length.");
    }
    NTL::vec_ZZ_p result;
    result.SetLength(v1.length());
    for (long i = 0; i < v1.length(); ++i)
        result[i] = v1[i] - v2[i];
    return result;
}

std::vector<double> divide_vector_by_scalar(const NTL::vec_ZZ& v, double scalar) {
    std::vector<double> result(v.length());
    for (long i = 0; i < v.length(); ++i)
        result[i] = NTL::to_double(v[i]) / scalar;
    return result;
}

std::vector<uint64_t> rounding(const std::vector<double>& v) {
    std::vector<uint64_t> result(v.size());
    for (size_t i = 0; i < v.size(); ++i)
        result[i] = round(v[i]);
    return result;
}

std::vector<uint64_t> flooring(const std::vector<double>& v) {
    std::vector<uint64_t> result(v.size());
    for (size_t i = 0; i < v.size(); ++i)
        result[i] = floor(v[i]);
    return result;
}

std::vector<uint64_t> ceiling(const std::vector<double>& v) {
    std::vector<uint64_t> result(v.size());
    for (size_t i = 0; i < v.size(); ++i)
        result[i] = ceil(v[i]);
    return result;
}

NTL::vec_ZZ mod_vector(const NTL::vec_ZZ& v, const NTL::ZZ& m) {
    NTL::vec_ZZ result;
    result.SetLength(v.length());
    for (long i = 0; i < v.length(); ++i)
        result[i] = v[i] % m;
    return result;
}

NTL::mat_ZZ lshift_matrix(const NTL::mat_ZZ& matrix) {
    NTL::mat_ZZ result;
    result.SetDims(matrix.NumRows(), matrix.NumCols());
    for (long i = 0; i < matrix.NumRows(); ++i)
        for (long j = 0; j < matrix.NumCols(); ++j)
            result[i][j] = matrix[i][j] << 1;
    return result;
}

NTL::vec_ZZ lshift_vector(const NTL::vec_ZZ& v) {
    NTL::vec_ZZ result;
    result.SetLength(v.length());
    for (long i = 0; i < v.length(); ++i)
        result[i] = v[i] << 1;
    return result;
}

NTL::vec_ZZ rshift_vector(const NTL::vec_ZZ& v) {
    NTL::vec_ZZ result;
    result.SetLength(v.length());
    for (long i = 0; i < v.length(); ++i)
        result[i] = NTL::conv<unsigned long>(v[i]) >> 1;
    return result;
}

NTL::vec_ZZ to_vec_ZZ(const std::vector<uint64_t>& v) {
    NTL::vec_ZZ result;
    result.SetLength(v.size());
    for (long i = 0; i < (long)v.size(); ++i) {
        result[i] = NTL::ZZ(v[i]);
    }
    return result;
}

NTL::vec_ZZ to_vec_ZZ(const std::vector<double>& v) {
    NTL::vec_ZZ result;
    result.SetLength(v.size());
    for (long i = 0; i < (long)v.size(); ++i) {
        result[i] = NTL::ZZ(static_cast<uint64_t>(std::round(v[i]))); 
    }
    return result;
}

NTL::vec_ZZ to_vec_ZZ_double(const std::vector<double>& v) {
    NTL::vec_ZZ res;
    res.SetLength(v.size());
    for (long i = 0; i < (long)v.size(); ++i) {
        res[i] = NTL::ZZ(static_cast<uint64_t>(std::round(v[i])));
    }
    return res;
}

void print_vec(const NTL::Vec<NTL::ZZ>& v, const std::string& name) {
    std::cout << name << " = [";
    for (long i = 0; i < v.length(); ++i) {
        std::cout << NTL::conv<unsigned long>(v[i]);
        if (i != v.length() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

void print_mat(const NTL::Mat<NTL::ZZ>& mat, const std::string& name) {
    std::cout << name << " = [" << std::endl;
    for (long i = 0; i < mat.NumRows(); ++i) {
        std::cout << "  [";
        for (long j = 0; j < mat.NumCols(); ++j) {
            std::cout << NTL::conv<unsigned long>(mat[i][j]);
            if (j != mat.NumCols() - 1) std::cout << ", ";
        }
        std::cout << "]";
        if (i != mat.NumRows() - 1) std::cout << ",";
        std::cout << std::endl;
    }
    std::cout << "]" << std::endl;
}

NTL::Mat<NTL::ZZ> to_MatZZ(const NTL::Mat<NTL::ZZ_p>& A) {
    long n = A.NumRows();
    long m = A.NumCols();
    NTL::Mat<NTL::ZZ> B;
    B.SetDims(n, m);

    for (long i = 0; i < n; i++) {
        for (long j = 0; j < m; j++) {
            B[i][j] = rep(A[i][j]);
        }
    }
    return B;
}

void gen_params() {
    A.resize(NUM_HASH_FUNC);
    B.resize(NUM_HASH_FUNC);
    iB.resize(NUM_HASH_FUNC);
    C.resize(NUM_HASH_FUNC);
    hint.resize(NUM_HASH_FUNC);

    NTL::SetNumThreads(NUM_THREADS);
    n = 2048;
    q = NTL::conv<NTL::ZZ>("18409157466922956641");
    Delta = q / p;
    // NTL::ZZ_p::init(q);
    NTL::ZZ_pContext ctx(q);
    ctx.restore();

    std::cout << "Generating LWE matrix A ..." << std::endl;
    // for(int i = 0; i < NUM_HASH_FUNC; ++i) {
    for(int i = 0; i < 1; ++i) {
        A[i].resize(l);
        // for(int j = 0; j < l; ++j) {
        for(int j = 0; j < 1; ++j) {
            A[i][j] = create_random_matrix(BF_size/NUM_HASH_FUNC, n, q);
        }
    }

    std::cout << "Generating non-singular matrix B ..." << std::endl;
    NTL::mat_ZZ_p Bt;
    NTL::ZZ_p det;
    // for(int i = 0; i < NUM_HASH_FUNC; ++i) {
    for(int i = 0; i < 1; ++i) {
        B[i].resize(l);
        iB[i].resize(l);
        // for(int j = 0; j < l; ++j) {
        for(int j = 0; j < 1; ++j) {
            do {
                Bt.SetDims(n, n);
                for (long i = 0; i < n; i++)
                    for (long j = 0; j < n; j++)
                        Bt[i][j] = NTL::random_ZZ_p();
                det = determinant(Bt); 
            } while (IsZero(det)); 
            NTL::mat_ZZ_p iBt;
            // std::cout << "Evaluating B inverse ..." << std::endl;
            inv(iBt, Bt);
            iB[i][j] = iBt;
            B[i][j] = Bt;
        }
    }

    std::cout << "Generating matrix C ..." << std::endl;
    // for(int i = 0; i < NUM_HASH_FUNC; ++i) {
    for(int i = 0; i < 1; ++i) {
        C[i].resize(l);
        // for(int j = 0; j < l; ++j) { 
        for(int j = 0; j < 1; ++j) { 
            C[i][j] = create_random_matrix(max_docs, n, q);
        }
    }
    
    std::cout << "Evaluating hint ..." << std::endl;
    std::vector<std::thread> threads;
    threads.reserve(NUM_HASH_FUNC * l);

    NTL::mat_ZZ_p temp = multiply_matrices(A[0][0], iB[0][0], q);
    for (int i = 0; i < NUM_HASH_FUNC; ++i) {
        hint[i].resize(l);
        for (int j = 0; j < l; ++j) {
            threads.emplace_back([&, i, j]() {
                // NTL::mat_ZZ_p eidx = vectorToMatZZ_p(encrypted_search_index[i][j]);
                // std::vector<std::vector<uint64_t>> x = 
                    // multiply_matrices(encrypted_search_index[i][j], temp, q);
                // NTL::mat_ZZ_p cx = mat_u64_to_mat_ZZ_p(x, q);
                // NTL::mat_ZZ_p htemp = subtract_matrices(
                //         cx,
                //         C[0][0], q
                // );
                // hint[i][j] = matZZpToVector(htemp, q);
                hint[i][j] = subtract_matrices(
                        multiply_matrices(encrypted_search_index[i][j], temp, q),
                        C[0][0], q
                );
            });
        }
    }
    for (auto &t: threads) {
        t.join();
    }
    /*
    for(int i = 0; i < NUM_HASH_FUNC; ++i) {
        hint[i].resize(l);
        for(int j = 0; j < l; ++j) { 
            // hint[i][j] = subtract_matrices(multiply_matrices(multiply_matrices(encrypted_search_index[i][j], A[i][j], q), iB[i][j], q), C[i][j], q);
            hint[i][j] = subtract_matrices(multiply_matrices(
                        multiply_matrices(encrypted_search_index[i][j], A[0][0], q), 
                        iB[0][0], q), C[0][0], q);
        }
    }
    */
}

query_t create_query(const int queried_index, int sub_index) {
    query_t query;
    int partition_size = BF_size / NUM_HASH_FUNC;
    int partition_index = queried_index/partition_size;
    int queried_position = queried_index % partition_size;
    
    NTL::Vec<NTL::ZZ> s;
    s.SetLength(n);
    for (long i = 0; i < n; ++i) {
        s[i] = RandomBnd(q);
    }
    // query.sp = matrix_vector_multiply(B[partition_index][sub_index], s, q);
    query.sp = matrix_vector_multiply(B[0][0], s, q);

    // Query generation
    NTL::Vec<NTL::ZZ> indicator;
    indicator.SetLength(partition_size);
    for (long i = 0; i < partition_size; ++i) {
        indicator[i] = NTL::ZZ(0);
    }
    indicator[queried_position] = NTL::ZZ(1);

    NTL::Vec<NTL::ZZ> error;
    error.SetLength(partition_size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> gaussian_dist(pow(2, -40), 6.4);
    for (long i = 0; i < partition_size; ++i) {
        error[i] = NTL::ZZ((long)(gaussian_dist(gen)));
    }

    query.q = add_vectors(
        add_vectors(vect_scalar_multiply(indicator, Delta, q), error, q),
        // matrix_vector_multiply(A[partition_index][sub_index], s, q), q);
        matrix_vector_multiply(A[0][0], s, q), q);
    
    // query.m = matrix_vector_multiply(C[partition_index][sub_index], query.sp, q);
    query.m = matrix_vector_multiply(C[0][0], query.sp, q);
    auto temp = flooring(divide_vector_by_scalar(query.m, NTL::conv<double>(Delta)));
    NTL::vec_ZZ temp_ZZ = to_vec_ZZ(temp);
    query.m = vect_scalar_multiply(temp_ZZ, Delta, q);
    
    query.r.SetLength(max_docs);
    for (long i = 0; i < max_docs; ++i) {
        query.r[i] = RandomBnd(pp);
    }

    auto temp_vd = divide_vector_by_scalar(query.m, NTL::conv<double>(Delta));
    temp_ZZ = to_vec_ZZ_double(temp_vd);
    query.m = subtract_vectors(lshift_vector(query.r), temp_ZZ, p);

    NTL::Vec<NTL::ZZ> e;
    e.SetLength(max_docs);

    auto worker = [&](int start, int end) {
        uint32_t prf_output[4];  
        for (int i = start; i < end; ++i) {
            std::string input = std::to_string(sub_index) +
                                std::to_string(queried_index) +
                                std::to_string(i) +
                                std::to_string(state[i]);
            prf(k2, (unsigned char*)input.c_str(), input.length(), (unsigned char*)prf_output);
            e[i] = NTL::conv<NTL::ZZ>(prf_output[0]) % p;
        }
    };

    std::vector<std::thread> threads;
    int chunk = (max_docs + NUM_THREADS - 1) / NUM_THREADS;
    for (int t = 0; t < NUM_THREADS; ++t) {
        int start = t * chunk;
        int end   = std::min(start + chunk, (int)max_docs);
        if (start < end) {
            threads.emplace_back(worker, start, end);
        }
    }
    for (auto& th : threads) {
        th.join();
    }

    /*
    uint32_t prf_output[4];
    for (long i = 0; i < max_docs; ++i) {
        std::string input = std::to_string(sub_index) + 
                            std::to_string(queried_index) + 
                            std::to_string(i) + 
                            std::to_string(state[i]);
        prf(k2, (unsigned char*)input.c_str(), input.length(), (unsigned char*)prf_output);
        e[i] = NTL::conv<NTL::ZZ>(prf_output[0]) % p;
    }
    */
    query.m = subtract_vectors(query.m, e, p);
    return query;
}

NTL::Vec<NTL::ZZ> execute_query(const query_t &query, const int partition_index, int sub_index) {
    NTL::mat_ZZ_p temp = mat_u64_to_mat_ZZ_p(hint[partition_index][sub_index], q);
    NTL::Vec<NTL::ZZ> opened_hint = matrix_vector_multiply(temp, query.sp, q);
    NTL::mat_ZZ_p eidx = vectorToMatZZ_p(encrypted_search_index[partition_index][sub_index]);
    NTL::Vec<NTL::ZZ> response = matrix_vector_multiply(eidx, query.q, q);
    NTL::Vec<NTL::ZZ> d = subtract_vectors(response, opened_hint, q);
    auto rounded_du = rounding(divide_vector_by_scalar(d, NTL::conv<double>(Delta)));
    NTL::vec_ZZ rounded_d = to_vec_ZZ(rounded_du);
    d = vect_scalar_multiply(rounded_d, Delta, q);
    auto divided_d = divide_vector_by_scalar(d, NTL::conv<double>(Delta));
    NTL::vec_ZZ divided_ZZd = to_vec_ZZ(divided_d);
    d = mod_vector(divided_ZZd, p);
    d = mod_vector(rshift_vector(add_vectors(d, query.m, p)), pp);
    return d;
}
