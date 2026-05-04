#ifndef __INDEX_H_
#define __INDEX_H_
#include <unordered_set>
#include <filesystem>
#include "utils.hpp"

namespace fs = std::filesystem;

void build_encrypted_search_index(fs::path &database, size_t num_cols, size_t num_rows);
void search_keyword(const std::string &keyword, double tpr = 0.95, double fpr = 0.1);
void update_document(int doc_id, int num_updates); 

#endif 
