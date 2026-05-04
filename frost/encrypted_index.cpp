#include "encrypted_index.hpp"

extern long n;
extern NTL::ZZ q;
extern std::vector<std::vector<NTL::Mat<NTL::ZZ_p>>> A;
extern std::vector<std::vector<NTL::Mat<NTL::ZZ_p>>> B, iB;
extern std::vector<std::vector<NTL::Mat<NTL::ZZ_p>>> C;
//extern std::vector<std::vector<NTL::Mat<NTL::ZZ_p>>> hint;
extern std::vector<std::vector<std::vector<std::vector<uint64_t>>>> hint;

int l = 5;
// std::vector<std::vector<NTL::Mat<NTL::ZZ_p>>> encrypted_search_index;
std::vector<std::vector<std::vector<std::vector<uint32_t>>>> encrypted_search_index;
size_t BF_size, max_docs;
unsigned char k1[32], k2[32];
std::vector<uint32_t> state; 
NTL::ZZ pp, p;

#ifdef ENABLE_DEBUGGING
extern std::vector<std::unordered_set<std::string>> test; 
#endif 

void build_encrypted_search_index(fs::path &database, size_t num_cols, size_t num_rows) {
    std::cout << "Initializing an empty encrypted search index ..." << std::endl;
    BF_size = num_cols;
    if(BF_size % NUM_HASH_FUNC) {
        BF_size = (BF_size / NUM_HASH_FUNC + 1) * NUM_HASH_FUNC;
    }
    max_docs = num_rows;
    state.resize(max_docs, 0);
    for (int i = 0; i < 32; i++) {
        k1[i] = static_cast<unsigned char>(NTL::RandomBnd(256));
        k2[i] = static_cast<unsigned char>(NTL::RandomBnd(256));
    }

    pp = NTL::conv<NTL::ZZ>(18483887);
    p = NTL::conv<NTL::ZZ>(80172943);

    // encrypted_search_index.resize(NUM_HASH_FUNC, std::vector<NTL::Mat<NTL::ZZ_p>>(l));
    encrypted_search_index.resize(NUM_HASH_FUNC, std::vector<std::vector<std::vector<uint32_t>>>(l));
    size_t partition_size = BF_size / NUM_HASH_FUNC;

    std::vector<std::thread> threads;
    threads.reserve(NUM_HASH_FUNC * l);

    for(int i = 0; i < NUM_HASH_FUNC; ++i) {
        for(int t = 0; t < l; ++t) {
            threads.emplace_back([&, i, t]() {
                uint32_t temp[4];
                NTL::ZZ_pContext ctx(p);
                ctx.restore();
                // encrypted_search_index[i][t].SetDims(max_docs, partition_size);
                encrypted_search_index[i][t].resize(max_docs, std::vector<uint32_t>(partition_size));
                std::string input;
                for(int j = 0; j < max_docs; ++j) {
                    for(int k = 0; k < partition_size; ++k) {
                        input = std::to_string(t) + 
                                std::to_string(k + i * partition_size) + 
                                std::to_string(j) + 
                                std::to_string(state[j]) + "0";
                        prf(k1, (unsigned char*)input.c_str(), input.length(), (unsigned char*)temp);
                        uint32_t value = temp[0] % 18483887;
                        // encrypted_search_index[i][t][j][k] = NTL::conv<NTL::ZZ>(temp[0]) % pp;
                        input.pop_back();
                        prf(k2, (unsigned char*)input.c_str(), input.length(), (unsigned char*)temp);
                        value = ((value << 1) + (temp[0] % 80172943)) % 80172943;
                        encrypted_search_index[i][t][j][k] = value;
                        // encrypted_search_index[i][t][j][k] = NTL::conv<NTL::ZZ_p>(value);
                        // encrypted_search_index[i][t][j][k] = ((encrypted_search_index[i][t][j][k] << 1) + 
                        //                             NTL::conv<NTL::ZZ>(temp[0])) % p;
                    }
                }
            });
        }
    }
    for (auto &t: threads) {
        t.join();
    }
#ifdef ENABLE_DEBUGGING
    test.resize(max_docs);
#endif 
    list_and_read_text_files(database);
    gen_params();
}

void search_keyword(const std::string &keyword, double tpr, double fpr) {
    // User
    auto start = clock_start();
    std::array<uint64_t, 2> hash_value = mm_hash((uint8_t*)keyword.c_str(), keyword.length());
    std::vector<std::pair<uint32_t, uint32_t>> pos;
    int count_total = 0, count_false_positive = 0;

    std::vector<int> available(BF_size); 
    std::iota(available.begin(), available.end(), 0); 
    size_t partition_size = BF_size / NUM_HASH_FUNC;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    std::unordered_set<int> tp, fp; 
    for(int i = 0; i < max_docs; ++i) {
        double coin_flip = dist(gen);
        if(coin_flip < tpr) {
            tp.insert(i);
        }
        coin_flip = dist(gen);
        if(coin_flip < fpr) {
            fp.insert(i);
        }
    }

    std::cout << "Added " << fp.size() << " false positives: ";
    for(int doc: fp) {
        // std::cout << doc << " ";
    }
    std::cout << std::endl;

    for (int i = 0; i < NUM_HASH_FUNC; ++i) {
        uint64_t temp = nth_hash(i, hash_value[0], hash_value[1], available.size());
        int partition_index = available[temp] / partition_size;
        pos.push_back({available[temp], partition_index});
        partition_index = temp / partition_size;
        available.erase(available.begin() + partition_index * partition_size, 
                available.begin() + (partition_index + 1) * partition_size);
    }

    std::cout << "=====================================================" << std::endl;
    std::cout << "Executing search query: " << std::endl;
    std::vector<std::vector<query_t>> query(NUM_HASH_FUNC);
    std::vector<std::vector<NTL::Vec<NTL::ZZ>>> output(NUM_HASH_FUNC);
    std::vector<std::vector<NTL::Vec<NTL::ZZ>>> tv(NUM_HASH_FUNC); 
    NTL::Vec<NTL::ZZ> mv;
    mv.SetLength(max_docs);
    clear(mv);
    
    std::vector<NTL::Vec<NTL::ZZ>> mv_temp(NUM_THREADS);
    for(int i = 0; i < NUM_THREADS; ++i) {
        mv_temp[i].SetLength(max_docs);
        clear(mv_temp[i]);
    }

    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);
    // RePIR query
    for (int j = 0; j < NUM_HASH_FUNC; j++) {
        threads.emplace_back([&, j]() {
            query[j].resize(l);
            for(int k = 0; k < l; ++k) {
                query[j][k] = create_query(pos[j].first, k); 
            }
        });
    }
    for (auto &t: threads) {
        t.join();
    }
    threads.clear();

    int chunk = (max_docs + NUM_THREADS - 1) / NUM_THREADS;
    // Transforming vectors
    for (int j = 0; j < NUM_HASH_FUNC; j++) {
        tv[j].resize(max_docs);
        for(int c = 0; c < NUM_THREADS; ++c) {
            int start = c * chunk;
            int end   = std::min(start + chunk, (int)max_docs);
            if (start < end) {
                threads.emplace_back([&, j, c, start, end]() {
                    NTL::Vec<NTL::ZZ> vm, vnm;
                    vm.SetLength(l);
                    vnm.SetLength(l);
                    for(int k = start; k < end; ++k) {
                        tv[j][k].SetLength(l);
                        for (int t = 0; t < l; ++t) {
                            tv[j][k][t] = RandomBnd(pp);
                        }
                        if(fp.find(k) != fp.end()) {
                            for(int t = 0; t < l; ++t) {
                                std::string input = std::to_string(t) + 
                                                    std::to_string(pos[j].first) + 
                                                    std::to_string(k) + 
                                                    std::to_string(state[k]) + "1";
                                uint32_t temp[4];
                                prf(k1, (unsigned char*)input.c_str(), input.length(), (unsigned char*)temp);
                                vm[t] = (NTL::conv<NTL::ZZ>(temp[0]) + query[j][t].r[k]) % pp;

                                input[input.length() - 1] = '0';
                                prf(k1, (unsigned char*)input.c_str(), input.length(), (unsigned char*)temp);
                                vnm[t] = (NTL::conv<NTL::ZZ>(temp[0]) + query[j][t].r[k]) % pp;
                            }
                            int u = 0;
                            for(; u < l; ++u) {
                                if(vm[u] != vnm[u]) {
                                    break;
                                }
                            }
                            NTL::ZZ last = NTL::ZZ(0);
                            for(int v = 0; v < l; ++v) {
                                if(v != u) {
                                    last = (last + (vm[v] - vnm[v]) * tv[j][k][v]) % pp;
                                }
                            }
                            NTL::ZZ temp_inv = InvMod((vnm[u] - vm[u]) % pp, pp);
                            last = (last * temp_inv) % pp;
                            tv[j][k][u] = last;
                            mv_temp[c][k] = (mv_temp[c][k] + dot_product(tv[j][k], vm, pp)) % pp; 
                        }
                        else if(tp.find(k) != tp.end()) {
                            for(int t = 0; t < l; ++t) {
                                std::string input = std::to_string(t) + 
                                                    std::to_string(pos[j].first) + 
                                                    std::to_string(k) + 
                                                    std::to_string(state[k]) + "1";
                                uint32_t temp[4];
                                prf(k1, (unsigned char*)input.c_str(), input.length(), (unsigned char*)temp);
                                vm[t] = (NTL::conv<NTL::ZZ>(temp[0]) + query[j][t].r[k]) % pp;
                            }
                            mv_temp[c][k] = (mv_temp[c][k] + dot_product(tv[j][k], vm, pp)) % pp;
                        }
                        else {
                            mv_temp[c][k] = (mv_temp[c][k] + RandomBnd(pp)) % pp;
                        }
                    }
                });
            }
        }
        for (auto &t: threads) {
            t.join();
        }
        threads.clear();
    }

    for(int i = 0; i < max_docs; ++i) {
        for(int j = 0; j < NUM_THREADS; ++j) {
            mv[i] = (mv[i] + mv_temp[j][i]) % pp;
        }
    }
    std::cout << "User-side search query generation time: " << time_from(start)/1000000 << "s" << std::endl;

    // Server
    start = clock_start();

    for (int j = 0; j < NUM_HASH_FUNC; j++) {
        output[j].resize(l);
        tv[j].resize(max_docs);
        for(int k = 0; k < l; ++k) {
            output[j][k] = execute_query(query[j][k], pos[j].second, k);
        }
    }

    std::cout << "Server-side search query execution time: " << time_from(start)/1000000 << "s" << std::endl;

    // User
    std::cout << "=====================================================" << std::endl;
    std::cout << "Keyword \"" << keyword << "\" appears in these file IDs: ";

    NTL::Vec<NTL::ZZ> ot;
    ot.SetLength(max_docs);
    clear(ot);

    for (int i = 0; i < max_docs; i++) {        
        for (int j = 0; j < NUM_HASH_FUNC; j++) {
            NTL::Vec<NTL::ZZ> temp;
            temp.SetLength(l);
            for(int k = 0; k < l; ++k) {
                temp[k] = output[j][k][i];
            }
            ot[i] = (ot[i] + dot_product(temp, tv[j][i], pp)) % pp;
        }
        
        if (mv[i] == ot[i]) {
            count_total++;
            std::cout << i << " ";
#ifdef ENABLE_DEBUGGING
            if(test[i].find(keyword) == test[i].end()) {
                count_false_positive++;
            }
#endif 
        }
    }
    std::cout << "\nThere are " << count_total << " files containing \"" << keyword << "\" in database." << std::endl;
#ifdef ENABLE_DEBUGGING
    std::cout << "There are " << count_false_positive << " false positives in results." << std::endl;
#endif 
    std::cout << "=====================================================" << std::endl;
    std::cout << "Search bandwidth: " << (((BF_size + n) * sizeof(uint64_t) + 
            max_docs * sizeof(uint32_t)) * NUM_HASH_FUNC * l + 
            max_docs * l * NUM_HASH_FUNC * sizeof(uint32_t) + 
            max_docs * sizeof(uint32_t) + count_total * sizeof(uint32_t)) 
            / 1048576.0 << " MB "<< std::endl;

#ifdef ENABLE_DEBUGGING
    std::cout << "Grouth truth: ";
    int correct_count = 0;
    for(int i = 0; i < max_docs; ++i) {
        if(test[i].count(keyword) > 0) {
            std::cout << i << " ";
            ++correct_count;
        }
    }
    std::cout << std::endl;
    std::cout << "There are " << correct_count << " matched documents." << std::endl;
#endif 
}

void update_document(int doc_id, int num_updates) {
    auto start = clock_start();

    std::ifstream updated_file("words.txt");

    state[doc_id] += 1;
    size_t partition_size = BF_size / NUM_HASH_FUNC;
    std::vector<std::thread> threads;
    threads.reserve(NUM_HASH_FUNC);

    for(int i = 0; i < NUM_HASH_FUNC; ++i) {
        threads.emplace_back([&, i]() {
            for(int j = 0; j < l; ++j) {
                uint32_t temp[4];
                NTL::ZZ_pContext ctx(p);
                ctx.restore();
                std::string input;
                for(int k = 0; k < partition_size; ++k) {
                    input = std::to_string(j) + 
                            std::to_string(k + i * partition_size) + 
                            std::to_string(doc_id) + 
                            std::to_string(state[doc_id]) + "0";
                    prf(k1, (unsigned char*)input.c_str(), input.length(), (unsigned char*)temp);
                    uint32_t value = temp[0] % 18483887;
                    input.pop_back();
                    prf(k2, (unsigned char*)input.c_str(), input.length(), (unsigned char*)temp);
                    value = ((value << 1) + (temp[0] % 80172943)) % 80172943;
                    encrypted_search_index[i][j][doc_id][k] = value;
                }
            }
        });
    }
    for (auto &t: threads) {
        t.join();
    }
    threads.clear();

    std::string keyword;
    // Update encrypted search index
#ifdef ENABLE_DEBUGGING
    test[doc_id].clear();
#endif 
    for(int i = 0; i < num_updates && getline(updated_file, keyword); ++i) {
#ifdef ENABLE_DEBUGGING
        test[doc_id].insert(keyword);
#endif 
        std::array<uint64_t, 2> hash_value = mm_hash((uint8_t*)keyword.c_str(), keyword.length());
        std::vector<int> available(BF_size); 
        std::iota(available.begin(), available.end(), 0); 
        size_t partition_size = BF_size / NUM_HASH_FUNC;
        for (int j = 0; j < NUM_HASH_FUNC; ++j) {
            uint64_t pos = nth_hash(j, hash_value[0], hash_value[1], available.size());
            int partition_index = available[pos] / partition_size;
            for(int k = 0; k < l; ++k) {
                std::string input = std::to_string(k) + 
                                    std::to_string(available[pos]) + 
                                    std::to_string(doc_id) + 
                                    std::to_string(state[doc_id]) + "1";
                uint32_t temp[4];
                prf(k1, (unsigned char*)input.c_str(), input.length(), (unsigned char*)temp);
                uint32_t value = temp[0] % 18483887;
                input.pop_back();
                prf(k2, (unsigned char*)input.c_str(), input.length(), (unsigned char*)temp);
                value = ((value << 1) + (temp[0] % 80172943)) % 80172943;
                encrypted_search_index[partition_index][k][doc_id][available[pos] % partition_size] = value;
            }
            partition_index = pos / partition_size;
            available.erase(available.begin() + partition_index * partition_size, 
                    available.begin() + (partition_index + 1) * partition_size);
        }
    }
    
    // Update hint
    NTL::ZZ_pContext ctx(q);
    ctx.restore();
    for(int i = 0; i < n; ++i) {
        random(C[0][0][doc_id][i]);
    }

    NTL::mat_ZZ_p temp = multiply_matrices(A[0][0], iB[0][0], q);
    for (int i = 0; i < NUM_HASH_FUNC; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < l; ++j) {
                NTL::vec_ZZ_p eidx;
                vec_u32_to_vec_ZZ_p(eidx, encrypted_search_index[i][j][doc_id], p);
                NTL::vec_ZZ_p updated_hint = subtract_vectors(
                        vector_matrix_multiply(eidx, temp, q),
                        C[0][0][doc_id], q
                );
                vec_ZZ_p_to_vec_u64(hint[i][j][doc_id], updated_hint, q);
            }
        });
    }
    for (auto &t: threads) {
        t.join();
    }

    std::cout << "Document update execution time: " << time_from(start)/1000000 << "s" << std::endl;
}
