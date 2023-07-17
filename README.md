# HyperCalm Sketch


## Introduction


Batch is an important pattern in data streams, which refers to a group of identical items that arrive closely. We find that some special batches that arrive periodically are of great value. In this paper, we formally define a new pattern, namely periodic batches. A group of periodic batches refers to several batches of the same item, where these batches arrive periodically. Studying periodic batches is important in many applications, such as caches, financial markets, online advertisements, networks, etc. We propose a one-pass sketching algorithm, namely the HyperCalm sketch, which takes two phases to detect periodic batches in real time. In phase 1, we propose a time-aware Bloom filter, namely HyperBloomFilter (HyperBF), to detect the start of batches. In phase 2, we propose an enhanced top-k algorithm, called Calm Space-Saving (CalmSS), to report topk periodic batches. Further, we used a bucketized approach to unify the algorithm and further optimize the performance, the optimized algorithm is called Bucketized HyperCalm. We theoretically derive the error bounds for HyperBF and CalmSS. Extensive experimental results show that Bucketized HyperCalm well achieves our design goal. Bucketized HyperCalm outperforms the Basic HyperCalm 14.1 times in term of average relative error and 1.6 times in term of processing speed, when Basic HyperCalm outperforms the strawman solutions 4 times in term of average relative error and 13.2 times in term of processing speed. We appliction Bucketized HyperCalm in network measurement and compare it to Basic HyperCalm. We test the speed of HyperCalm in Redis. We also apply HyperCalm to a cache system and integrate HyperCalm into Apache Flink. 



## About this repo

- `CPU` contains codes of HyperCalm and the related algorithms implemented on CPU platforms:
  - `HyperCalm`: Source codes for the HyperCalm sketch, including HyperBF, Basic CalmSS, Bucketized CalmSS. 
  - `ComparedAlgorithms`: Source codes for the related algorithms in our paper, including Time-Out Bloom filter (TOBF), Clock-Sketch, SWAMP, Space-Saving (SS), Unbiased Space-Saving (USS), Cold-Filter + Space-Saving (CF+SS). 
  - `Batch`: Source codes for detecting item batches. 
  - `PeriodicBatch`: Source codes for finding top-k periodic batches.  
  - `TopK`: Source codes for finding top-k items.
  - `lib`: The hash table, hash function, and some common functions used by many algorithms. 
- `datasets`: Sample datasets extracted from the real-world datasets used in our CPU, Measurement and Redis experiments.
- `Measurement`: Source codes for finding Delay and Loss.
- `Redis`: Source codes for HyperCalm in Redis.
- `Cache` contains codes that integrate HyperCalm and the related algorithms into a cache system.
    -  `src`: Source codes that integrate HyperCalm and the related algorithms into a cache system. 
    -  `datasets`: Sample datasets extracted from the real-world datasets used in cache experiments.

- `Flink` contains codes of HyperCalm implemented on top of Apache Flink and a sample dataset used in Flink experiments. 

- More details can be found in the folders.

## Requirements

#### CPU Requirements

- **g++, C++17, CMake**: We implement most the codes with C++ and build them with g++ 7.5.0 (Ubuntu 7.5.0-6ubuntu2) and -O3 option. 

- **Boost**: Our codes use the [Boost C++ Libraries](https://www.boost.org). 

  - On Linux platform, you can install the Boost libraries using the following commands (it may take 5-10 minutes).

    ```bash
    $ wget http://sourceforge.net/projects/boost/files/boost/1.54.0/boost_1_54_0.tar.gz
    $ tar -xzvf boost_1_54_0.tar.gz
    $ cd boost_1_54_0
    $ ./bootstrap.sh --prefix=/usr/local
    $ ./b2 install --with=all
    ```

    The Boost libraries will be installed in `/usr/local/lib`. Next, use the following command to set the environment variable. 

    ```bash
    $ export LD_LIBRARY_PATH=/usr/local/lib
    ```

## Datasets

The datsets used in CPU, Measurement and Redis experiments are files containing many items. We provide two sample datasets in the `datasets` directory, which are extracted from the real-world CAIDA dataset and the Criteo datset, respectively. For the full datasets, please register in [CAIDA](http://www.caida.org/home/) or [Criteo](https://ailab.criteo.com/ressources/) first, and then apply for the traces. 

- ` CAIDA.dat` is a dataset extracted from the CAIDA dataset. CAIDA Anonymized Internet Trace is a data stream of anonymized IP trace collected in 2018. Each item is identified by its source IP address (4 bytes) and destination IP address (4 bytes). 

- `CRITEO.log` is a dataset extracted from the Criteo dataset. Criteo dataset is an online advertising click data stream consisting of about 45M ad impressions. Each item is identified by its categorical feature values and conversion feedback. 


#### Dataset Information

| Dataset    | Freq. of the hottest item | # distincnt items |  # items  |
| :--------- | :-----------------------: | :---------------: | :-------: |
| CAIDA.dat  |          77,254           |      39,954       | 1,000,000 |
| CRITEO.log |           1,910           |      75,228       | 1,000,000 |

- **AVX-512 (optional)**: We use 512-bit SIMD instructions to accelerate the operations of HyperCalm, and conduct the experiments on a CPU that supports AVX-512 instruction set. We also provide a basic version of HyperBF that does not use SIMD instructions. You can decide which version of HyperBF to build by defining the `SIMD` macro when calling the `make` command or not. More details can be found in the folders. 

- **Redis**: Our codes use the [Redis](https://redis.io/)
  - On Linux platform, you can install Redis using the following commands (it may take 5-10 minutes).
    ```bash
    $ yum install -y gcc tcl
    $ apt update
    $ apt install redis-server
    ```

- **python3**: We implement some code in python3 and need to install numpy and redis PIL.
  - On Linux platform, you can install these PIL using the following commands (it may take 5-10 minutes).
    ```bash
    $ pip install redis
    $ pip install numpy
    ```



#### Flink Requirements

- **Flink 1.13.1**

- **Hadoop 2.8.3**

To address the problem of dependency when using Hadoop Distributed File System in Flink, [flink-shaded-hadoop-2-uber-2.8.3-9.0.jar]([Central Repository: org/apache/flink/flink-shaded-hadoop-2-uber/2.8.3-9.0](https://repo.maven.apache.org/maven2/org/apache/flink/flink-shaded-hadoop-2-uber/2.8.3-9.0/)) is needed. Please add it to `{FLINK_HOME}/lib/`. 



## How to run

- For CPU, Cache and Measurement experiments, firstly make sure the **Boost** libraries have been successfully installed on your machine. Then enter the folder `CPU`, `Cache` or `Measurement` to build and run the tests. More details can be found in the folders. 
- For Flink experiments, enter the folder `Flink` and follow the instructions there to build the package, set up Flink environment, and run the tests. 
- For Redis experiments, firstly make sure the **Boost** libraries and **Redis** have been successfully installed on your machine.
Then enter the folder `Redis` to build and run the tests. More details can be found in the folders. 
