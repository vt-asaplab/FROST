#include <iostream>
#include "encrypted_index.hpp"

using namespace std; 

void test_ESSE(fs::path folder, int bloom_filter_size, int num_documents) {
    // cout << "Enter folder: " << endl;
    // cin >> folder;

    if (!fs::is_directory(folder)) {
        cout << "Error: dataset folder does not exist!!!" << endl;
        exit(1);
    }

    build_encrypted_search_index(folder, bloom_filter_size, num_documents);

    // Evaluate the delay of document update
    update_document(/* doc_id */ 977, /* #updates */ 100);

    // Evaluate the delay and communication cost of keyword search
    while(1) {
        string keyword;
        cout << "Enter keyword: ";
        cin >> keyword;
        double tpr = 0.95;
        vector<double> fpr_list = {0.025, 0.05, 0.1, 0.15, 0.2};
        for(double fpr: fpr_list) {
            auto start = clock_start();
            search_keyword(keyword, tpr, fpr);
            cout << "Total search latency (FPR " << fpr << "): " << 
                    time_from(start)/1000000 << "s\n" << endl;
        }
    }
}

int main(int argc, char** argv) {
    int bloom_filter_size = 2912;
    int num_documents = 1024; 
    fs::path folder = "./maildir";
    
    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-d") == 0)  
            folder = argv[++i];
        else if(strcmp(argv[i], "-m") == 0) 
            bloom_filter_size = atoi(argv[++i]);
        else if (strcmp(argv[i], "-n") == 0) 
            num_documents = atoi(argv[++i]);
        else {
            cout << "Option " << argv[i] << " does not exist!!!" << endl;
            exit(1);
        }
        i++;
    }
    
    test_ESSE(folder, bloom_filter_size, num_documents); 
    return 0;
}
