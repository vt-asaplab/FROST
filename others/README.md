# OSSE
- Installation:

See **OSSE/README.md** 

- To evaluate the metrics of keyword search for OSSE, go to the **OSSE/code** directory and execute:

```
python3 dp_sse_bench.py [-n <number_of_documents>] [-t <true_positive_rate>] [-f <false_positive_rate>]
```

For example:
```
python3 dp_sse_bench.py 
```

By default, it starts the process with default parameters number_of_documents = 1024, true_positive_rate = 0.95 and false_positive_rate = 0.1. 
