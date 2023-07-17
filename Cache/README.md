# Applying HyperCalm to Cache Systems

We apply HyperCalm to a cache system to showcase a promising application scenario. In cache systems, the data stream is formed by many memory access requests, and an item batch is a group of identical memory access requests. By accurately detecting periodic batches in real time, HyperCalm yields two insights to optimize cache replacement policies and guide prefetching data, respectively. First, with the help of real-time item batch detection, we can find out the item batches that are still active now. When the cache is full, we do not discard those items that are still in an active batch since they are highly likely to arrive again in the near future. Second, with the historical knowledge of periodic batches, we can forecast the arrival time of new batches, so as to prefetch the data into cache before their arrival. Our experimental results show that with the help of HyperCalm, the hit rates of both LFU and LRU are significantly enhanced. 


## File Structure

*  `datasets`: Sample datasets extracted from the real-world datasets used in cache experiments.
*  `src`: Source codes that integrate HyperCalm and the related algorithms into a cache system. 

## Cache Datasets

The datsets used in cache experiments are files containing many memory access requests. Each memory access request has a memory address and an arrival time. We provide two sample datasets in the `datasets` directory, which are extracted from the real datasets used in our paper. Each row of our dataset is a 2-tuple consisting of an ID (32-bit Integer) and a timestamp (Double), seperated by a space. Here, ID is the memory access address and the timestamp is the arrival time of the memory access request. 

- `CAIDA.dat` is a dataset extracted from the CAIDA dataset. We use a simple trick to transform the flow IDs in CAIDA dataset to 32-bit memory addresses.  For the full CAIDA datasets, please register in [CAIDA](http://www.caida.org/home/) first and then apply for the traces. 

- `synthesis.dat` is a dataset used to conduct the LRU experiments. In the synthetic dataset, we let half of the items (memory access requests) arrive periodically and the other half arrive randomly. Further, we carefully design the periods of the periodic items so that LRU is completely unable to hit them.

For more details, please refer to our paper. 

#### Dataset Information

| Dataset       | Freq. of the hottest item     | # distinct items  |  # items  |
| :------------ | :---------------------------: | :---------------: | :-------: |
| CAIDA.dat     |            77,254             |      39,954       | 1,000,000 |
| synthesis.dat |              851              |      415,232      | 1,000,000 |

#### How to generate the entire synthetic dataset

To make our results easy to reproduce, we provide a script `make` in the `datasets` directory, which can generate the entire synthetic dataset used in our paper. You can use the following commands to build and run. 

```bash
$ cd datasets
$ make
$ ./generate_synthesis
```

## How to Run 

#### Test hit rates

You can use the following commands to build and run our tests. 

```bash
$ cd src
$ make
$ ./cache_test -f FILENAME -m MEMORY -c CACHESIZE
```

1. `-f`: Path of the dataset you want to run. 
2. `-m`: An integer, specifying the memory size (in bytes) used by the HyperCalm sketch. 
3. `-c`: An integer, specifying the size (# lines) of the cache. 

You can modify the `BATCH_TIME_THRESHOLD` in `main.cpp` to adapt to different datasets. 

For example, you can run the following command to get the LFU experimental results in our paper. 

```bash
$ ./cache_test -f ../datasets/CAIDA.dat -m 20000 -c 640
```

You can run the following command to get the LRU experimental results in our paper. 

```bash
$ ./cache_test -f ../datasets/synthesis.dat -m 20000 -c 640
```

Our program first prints the statistics about the input dataset and the parameters of the candidate algorithms. Then our program prints the hit rates of the two replacement policies (LFU and LRU) under different optimization methods. 


#### Test throughput

You can use the following commands to build and run our tests. 

```bash
$ cd src
$ make
$ ./cache_throughput -f FILENAME -m MEMORY -c CACHESIZE -v {1-5}
```

1. `-f`: Path of the dataset you want to run. 
2. `-m`: An integer, specifying the memory size (in bytes) used by the HyperCalm sketch. 
3. `-c`: An integer, specifying the size (# lines) of the cache. 
4. `-v`: An integer (1-5), specifying the optimization method you want to test. The corresponding relationship is as follows. 

|  1   |       2       |  3   |     4     |       5       |
| :--: | :-----------: | :--: | :-------: | :-----------: |
| LRU  | LRU+HyperCalm | LFU  | LFU+Clock | LFU+HyperCalm |

Our program first prints the parameters of the candidate algorithms, and then prints the processing speed of our cache system under the target optimization method. 