# HyperCalm Sketch


## Introduction


Batch is an important pattern in data streams, which refers to a group of identical items that arrive closely. We find that some special batches that arrive periodically are of great value. In this paper, we formally define a new pattern, namely periodic batches. A group of periodic batches refers to several batches of the same item, where these batches arrive periodically. Studying periodic batches is important in many applications, such as caches, financial markets, online advertisements, networks, etc. We propose a one-pass sketching algorithm, namely the HyperCalm sketch, which takes two phases to detect periodic batches in real time. In phase 1, we propose a time-aware Bloom filter, namely HyperBloomFilter (HyperBF), to detect the start of batches. In phase 2, we propose an enhanced top-k algorithm, called Calm Space-Saving (CalmSS), to report topk periodic batches. We theoretically derive the error bounds for HyperBF and CalmSS. Extensive experiments show HyperCalm outperforms the strawman solutions 4× in term of average relative error and 13.2× in term of speed. We also apply HyperCalm to a cache system and integrate HyperCalm into Apache Flink. 



## About this repo

- `CPU` contains codes of HyperCalm and the related algorithms implemented on CPU platforms:
  - `HyperCalm`: Source codes for the HyperCalm sketch, including HyperBF and CalmSS. 
  - `ComparedAlgorithms`: Source codes for the related algorithms in our paper, including Time-Out Bloom filter (TOBF), Clock-Sketch, SWAMP, Space-Saving (SS), Unbiased Space-Saving (USS). 
  - `Batch`: Source codes for detecting item batches. 
  - `PeriodicBatch`: Source codes for finding top-k periodic batches.  
  - `TopK`: Source codes for finding top-k items. 
  - `datasets`: Sample datasets extracted from the real-world datasets used in our CPU experiments.
  - `lib`: The hash table, hash function, and some common functions used by many algorithms. 

- `Cache` contains codes that integrate HyperCalm and the related algorithms into a cache system.
    -  `src`: Source codes that integrate HyperCalm and the related algorithms into a cache system. 
    -  `datasets`: Sample datasets extracted from the real-world datasets used in cache experiments.

- `Flink` contains codes of HyperCalm implemented on top of Apache Flink and a sample dataset used in Flink experiments. 

- More details can be found in the folders.

## Requirements

#### CPU Requirements

- **g++, C++17, CMake**: We implement all the codes with C++ and build them with g++ 7.5.0 (Ubuntu 7.5.0-6ubuntu2) and -O3 option. 

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

- **AVX-512 (optional)**: We use 512-bit SIMD instructions to accelerate the operations of HyperBF, and conduct the experiments on a CPU that supports AVX-512 instruction set. We also provide a basic version of HyperBF that does not use SIMD instructions. You can decide which version of HyperBF to build by defining the `SIMD` macro when calling the `make` command or not. More details can be found in the folders. 

#### Flink Requirements

- **Flink 1.13.1**

- **Hadoop 2.8.3**

To address the problem of dependency when using Hadoop Distributed File System in Flink, [flink-shaded-hadoop-2-uber-2.8.3-9.0.jar]([Central Repository: org/apache/flink/flink-shaded-hadoop-2-uber/2.8.3-9.0](https://repo.maven.apache.org/maven2/org/apache/flink/flink-shaded-hadoop-2-uber/2.8.3-9.0/)) is needed. Please add it to `{FLINK_HOME}/lib/`. 



## How to run

- For CPU and Cache experiments, firstly make sure the **Boost** libraries have been successfully installed on your machine. Then enter the folder `CPU` or `Cache` to build and run the tests. More details can be found in the folders. 
- For Flink experiments, enter the folder `Flink` and follow the instructions there to build the package, set up Flink environment, and run the tests. 

