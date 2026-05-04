#include "utils.hpp"

namespace fs = std::filesystem;

// extern std::vector<std::vector<NTL::Mat<NTL::ZZ_p>>> encrypted_search_index;
extern std::vector<std::vector<std::vector<std::vector<uint32_t>>>> encrypted_search_index;
extern size_t BF_size, max_docs;
extern unsigned char k1[32], k2[32];
extern std::vector<uint32_t> state; 
extern NTL::ZZ p, pp;
extern int l;

#ifdef ENABLE_DEBUGGING
std::vector<std::unordered_set<std::string>> test; 
#endif 

std::unordered_set<std::string> stopwords = {
        "a", "about", "above", "across", "after",
		"afterwards", "again", "against", "all", "almost", "alone", "along",
		"already", "also", "although", "always", "am", "among", "amongst", "amoungst",
		"amount", "an", "and", "another", "any", "anyhow", "anyone", "anything", "anyway",
		"anywhere", "are", "around", "as", "at", "back", "be", "became", "because", "become",
		"becomes", "becoming", "been", "before", "beforehand", "behind", "being", "below",
		"beside", "besides", "between", "beyond", "bill", "both", "bottom", "but", "by",
		"call", "can", "cannot", "cant", "co", "con", "could", "couldnt", "cry", "de",
		"describe", "detail", "do", "done", "down", "due", "during", "each", "eg", "eight",
		"either", "eleven", "else", "elsewhere", "empty", "enough", "etc", "even", "ever",
		"every", "everyone", "everything", "everywhere", "except", "few", "fifteen", "fify",
		"fill", "find", "fire", "first", "five", "for", "former", "formerly", "forty", "found",
		"four", "from", "front", "full", "further", "get", "give", "go", "had", "has", "hasnt",
		"have", "he", "hence", "her", "here", "hereafter", "hereby", "herein", "hereupon",
		"hers", "herself", "him", "himself", "his", "how", "however", "hundred", "ie", "if",
		"in", "inc", "indeed", "interest", "into", "is", "it", "its", "itself", "keep", "last",
		"latter", "latterly", "least", "less", "ltd", "made", "many", "may", "me", "meanwhile",
		"might", "mill", "mine", "more", "moreover", "most", "mostly", "move", "much", "must",
		"my", "myself", "name", "namely", "neither", "never", "nevertheless", "next", "nine",
		"no", "nobody", "none", "noone", "nor", "not", "nothing", "now", "nowhere", "of", "off",
		"often", "on", "once", "one", "only", "onto", "or", "other", "others", "otherwise",
		"our", "ours", "ourselves", "out", "over", "own", "part", "per", "perhaps", "please",
		"put", "rather", "re", "same", "see", "seem", "seemed", "seeming", "seems", "serious",
		"several", "she", "should", "show", "side", "since", "sincere", "six", "sixty", "so",
		"some", "somehow", "someone", "something", "sometime", "sometimes", "somewhere",
		"still", "such", "system", "take", "ten", "than", "that", "the", "their", "them",
		"themselves", "then", "thence", "there", "thereafter", "thereby", "therefore",
		"therein", "thereupon", "these", "they", "thickv", "thin", "third", "this", "those",
		"though", "three", "through", "throughout", "thru", "thus", "to", "together", "too",
		"top", "toward", "towards", "twelve", "twenty", "two", "un", "under", "until", "up",
		"upon", "us", "very", "via", "was", "we", "well", "were", "what", "whatever", "when",
		"whence", "whenever", "where", "whereafter", "whereas", "whereby", "wherein",
		"whereupon", "wherever", "whether", "which", "while", "whither", "who", "whoever",
		"whole", "whom", "whose", "why", "will", "with", "within", "without", "would", "yet",
		"you", "your", "yours", "yourself", "yourselves"};

void handleErrors(void) {
    ERR_print_errors_fp(stderr);
    abort();
}

int prf(unsigned char *key, unsigned char *input, int input_len, unsigned char *output) {
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;

    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, key, NULL))
        handleErrors();

    if(1 != EVP_EncryptUpdate(ctx, output, &len, input, input_len))
        handleErrors();

    ciphertext_len = len;

    if(1 != EVP_EncryptFinal_ex(ctx, output + len, &len))
        handleErrors();
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

void convert_ZZ_p_to_uint64(uint64_t &uint64_value, const NTL::ZZ_p &ZZ_p_value) {
    NTL::ZZ temp;
    conv(temp, ZZ_p_value);
    long val;
    conv(val, temp & 0xFFFFFFFF);
    uint64_value = val;
    temp >>= 32;
    conv(val, temp & 0xFFFFFFFF);
    uint64_value = (val << 32) | uint64_value;
}

void convert_uint64_to_ZZ_p(NTL::ZZ_p &ZZ_p_value, const uint64_t &scalar) {
    NTL::ZZ ZZ_value = NTL::ZZ(0);
    ZZ_value = (scalar >> 32) & 0xFFFFFFFF;
    ZZ_value = (ZZ_value << 32) | (scalar & 0xFFFFFFFF);
    ZZ_p_value = NTL::conv<NTL::ZZ_p>(ZZ_value);
}

std::array<uint64_t, 2> mm_hash(const uint8_t* data, size_t len) {
    std::array<uint64_t, 2> hash_value;
    MurmurHash3_x64_128(data, len, 0, hash_value.data());
    return hash_value;
}

void process_text_file(const fs::path& file_path, const size_t doc_count) {
    std::ifstream infile(file_path);
    NTL::ZZ_pContext ctx(p);
    ctx.restore();

    if (!infile.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return;
    }

    std::cout << "Reading file: " << file_path << std::endl;

    std::string word;
    while (infile >> word) {  
        bool skip = false;
        if(word.length() < 4 || word.length() > 20 || stopwords.count(word) > 0) {
            skip = true;
        }
        else {
            for(char &ch: word) {
                if(!std::isalpha(ch)) {
                    skip = true;
                    break;
                }
                else {
                    ch = std::tolower(ch);
                }
            }
        }
        if(!skip) {
            // std::cout << word << std::endl;
#ifdef ENABLE_DEBUGGING
            test[doc_count].insert(word);
#endif 
            std::array<uint64_t, 2> hash_value = mm_hash((uint8_t*)word.c_str(), word.length());
            std::vector<int> available(BF_size); 
            std::iota(available.begin(), available.end(), 0); 
            size_t partition_size = BF_size / NUM_HASH_FUNC;
            for (int j = 0; j < NUM_HASH_FUNC; ++j) {
                uint64_t pos = nth_hash(j, hash_value[0], hash_value[1], available.size());
                int partition_index = available[pos] / partition_size;
                for(int k = 0; k < l; ++k) {
                    std::string input = std::to_string(k) + 
                                        std::to_string(available[pos]) + 
                                        std::to_string(doc_count) + 
                                        std::to_string(state[doc_count]) + "1";
                    uint32_t temp[4];
                    prf(k1, (unsigned char*)input.c_str(), input.length(), (unsigned char*)temp);
                    uint32_t value = temp[0] % 18483887;
                    // encrypted_search_index[partition_index][k][doc_count][available[pos] % partition_size] = NTL::conv<NTL::ZZ>(temp[0]) % pp;
                    input.pop_back();
                    prf(k2, (unsigned char*)input.c_str(), input.length(), (unsigned char*)temp);
                    value = ((value << 1) + (temp[0] % 80172943)) % 80172943;
                    encrypted_search_index[partition_index][k][doc_count][available[pos] % partition_size] = value;
                    // encrypted_search_index[partition_index][k][doc_count][available[pos] % partition_size] = NTL::conv<NTL::ZZ_p>(value);
                    // encrypted_search_index[partition_index][k][doc_count][available[pos] % partition_size] = 
                    //         ((encrypted_search_index[partition_index][k][doc_count][available[pos] % partition_size] << 1) + 
                    //                                 NTL::conv<NTL::ZZ>(temp[0])) % p;
                }
                partition_index = pos / partition_size;
                available.erase(available.begin() + partition_index * partition_size, 
                        available.begin() + (partition_index + 1) * partition_size);
            }
        }
    }

    infile.close();
}

void list_and_read_text_files(const fs::path& folder_path) {
    if (!fs::exists(folder_path) || !fs::is_directory(folder_path)) {
        std::cerr << "Invalid directory: " << folder_path << std::endl;
        return;
    }

#ifdef ENABLE_DEBUGGING
    test.resize(max_docs);
#endif
    std::vector<fs::path> files;
    files.reserve(max_docs);

    for (const auto& entry : fs::recursive_directory_iterator(folder_path)) {
        if (files.size() >= max_docs) break;
        if (entry.is_regular_file()) {
            files.push_back(entry.path());
        }
    }

    // Use all available CPU cores for faster initialization
    const unsigned num_threads = std::max(1u, std::thread::hardware_concurrency());
    
    std::atomic<size_t> next_index{0};
    std::vector<std::thread> workers;
    workers.reserve(num_threads);

    auto worker = [&]() {
        while (true) {
            size_t idx = next_index.fetch_add(1, std::memory_order_relaxed);
            if (idx >= files.size()) break;

            process_text_file(files[idx], idx);
        }
    };

    for (unsigned t = 0; t < num_threads; ++t) {
        workers.emplace_back(worker);
    }

    for (auto& t: workers) {
        t.join();
    }
}
/*
void list_and_read_text_files(const fs::path& folder_path) {
    if (!fs::exists(folder_path) || !fs::is_directory(folder_path)) {
        std::cerr << "Invalid directory: " << folder_path << std::endl;
        return;
    }
    
#ifdef ENABLE_DEBUGGING
    test.resize(max_docs);
#endif 

    size_t doc_count = 0;

    for (const auto& entry : fs::recursive_directory_iterator(folder_path)) {
        if (doc_count >= max_docs) {
            break;  
        }

        if (entry.is_regular_file()) {
            process_text_file(entry.path(), doc_count);
            ++doc_count;
        }
    }
}
*/
