# OSSE

- Installation: see **OSSE/README.md** 

- To evaluate the keyword search metrics for OSSE, navigate to the directory **OSSE/code** and run the following command:

```
python3 dp_sse_bench.py [-n <number_of_documents>] [-t <true_positive_rate>] [-f <false_positive_rate>]
```

For example:
```
python3 dp_sse_bench.py 
```

By default, it starts the process with default parameters number_of_documents = 1024, true_positive_rate = 0.95 and false_positive_rate = 0.1. 

# CLRZ

- Installation: see **Clusion/README.md** 

- To evaluate the keyword search metrics for CLRZ, navigate to the directory **Clusion** and run the following command:

```
java -Xmx32g org.crypto.sse.TestLocalRR2Lev [-d <path_to_dataset_directory>] [-n <number_of_documents>] [-t <true_positive_rate>] [-f <false_positive_rate>]
```

For example:
```
java -Xmx32g org.crypto.sse.TestLocalRR2Lev -d ./maildir 
```

By default, it starts the process with default parameters number_of_documents = 1024, true_positive_rate = 0.95 and false_positive_rate = 0.1. 

