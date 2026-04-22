# Efficient Single-Round Obfuscation of Search and Result Patterns in Searchable Encryption

This is our full implementation for our FROST.

**WARNING**: This is an academic proof-of-concept prototype and has not received careful code review. This implementation is NOT ready for production use.

# Required Libraries

1. [GMP 6.3.0](https://gmplib.org)

2. [NTL v11.6.0](http://www.shoup.net/ntl/download.html)

You can run the script file **auto_setup.sh** (user needs root permission) to automatically install the required libraries. 
```
./auto_setup.sh
```

# Dataset

1. Download Enron email dataset at: https://www.cs.cmu.edu/~enron/, then extract to obtain **maildir** folder.
2. Download Wiki dataset at [Wikipedia database backup dump](https://dumps.wikimedia.org/): https://dumps.wikimedia.org/enwiki/latest/enwiki-latest-pages-articles.xml.bz2, then use [WikiExtractor](https://github.com/attardi/wikiextractor) to extract articles in JSON format:
```
python3 -m wikiextractor.WikiExtractor enwiki-latest-pages-articles.xml.bz2 --json -o extracted
```
Finally, execute the python script in ``dataset/extract_random.py`` to randomly sample 500K documents, excluding the empty ones without any keywords. 

# Build & Compile

Go to the folder **frost** then execute:
``` 
make
```
This is going to create an executable file named ``main`` in the **frost** folder.

## Testing

- Go to the **frost** directory and launch the executable file `main`:

```
cd frost
./main [-d <Path to dataset>] [-m <Bloom_filter_size>] [-n <Number_of_documents>]
```

For example:
```
./main -d ./maildir 
```

The default parameters: Bloom_filter_size is 2912, and Number_of_documents is 1024. 

- Wait for the initialization process to complete, then enter a keyword to perform a search.
- It outputs processing latency and bandwidth overhead of keyword search for the true positive rate TPR = 0.95 w.r.t various false positive rates FPR = {0.025, 0.05, 0.1, 0.15, 0.2}. 
