# Cache Datasets

The datsets used in cache experiments are files containing many memory access requests. Each memory access request has a memory address and an arrival time. We provide two sample datasets in the `dataset` directory, which is extracted from the real datasets used in our paper. Each row of our dataset is a 2-tuple consisting of an ID (32-bit Integer) and a timestamp (Double), seperated by a space. Here, ID is the memory access address and the timestamp is the arrival time of the memory access request. 

- ` CAIDA.dat` is a dataset extracted from the CAIDA dataset. We use a simple trick to transform the flow IDs in CAIDA dataset to 32-bit memory addresses.  For the full CAIDA datasets, please register in [CAIDA](http://www.caida.org/home/) first and then apply for the traces. 

- `synthesis.dat` is a dataset used to conduct the LRU experiments. In the synthetic dataset, we let half of the items (memory access requests) arrive periodically and the other half arrive randomly. Further, we carefully design the periods of the periodic items so that LRU is completely unable to hit them.

For more details, please refer to our paper. 

## Dataset Information

| Dataset       | Freq. of the hottest item | # distincnt items |  # items  |
| :------------ | :-----------------------: | :---------------: | :-------: |
| CAIDA.dat     |          77,254           |      39,954       | 1,000,000 |
| synthesis.dat |            851            |      415,232      | 1,000,000 |

## How to generate the entire synthetic dataset

To make our results easy to reproduce, we provide a script `make` in the `datasets` directory, which can generate the entire synthetic dataset used in our paper. You can use the following commands to build and run. 

```bash
$ make
$ ./generate_synthesis
```

## Notification 

These data files are only used for testing the performance of HyperCalm and the related algorithms in this project. Please do not use these traces for other purpose. 

