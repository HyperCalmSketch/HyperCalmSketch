# Codes on CPU Platform 

We conduct CPU experiments to compare the performance of HyperCalm and a strawman solution (Clock-Sketch + Unbiased Space-Saving) in finding periodic batches. We also compare the performance of HyperBF, Clock-Sketch, Time-Out Bloom filter, and SWAMP in detecting item batches, and the performance of CalmSS, Space-Saving, and Unbiased Space-Saving (USS) in finding top-k items. Experimental results show that HyperCalm well achieves our design goal. HyperCalm outperforms the strawman solutions 4 times in term of average relative error and 13.2 times in term of processing speed. Besides, we find that both HyperBF and CalmSS, significantly outperform the state-of-the-art solutions in detecting item batches and finding top-k items, respectively. Our experiments are conducted on an 18-core CPU server (Intel i9-10980XE) with 128GB DDR4 memory and 24.75MB L3 cache. We set the CPU frequency to 4.2GHz and set the memory frequency to 3200MHz. 


## File Structure 

- `HyperCalm`: Source codes for the HyperCalm sketch, including HyperBF and CalmSS. 
- `ComparedAlgorithms`: Source codes for the related algorithms in our paper, including Time-Out Bloom filter (TOBF), Clock-Sketch, SWAMP, Space-Saving (SS), Unbiased Space-Saving (USS). 
- `Batch`: Source codes for detecting item batches. 
- `PeriodicBatch`: Source codes for finding top-k periodic batches.  
- `TopK`: Source codes for finding top-k items. 
- `datasets`: Sample datasets extracted from the real-world datasets used in our CPU experiments.
- `lib`: The hash table, hash function, and some common functions used by many algorithms. 

## Dependency 

- **Murmur Hash**: The hash functions we use are 32-bit Murmur hash functions, obtained [here](https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp).
- **g++, C++17, CMake**: We implement all the codes with C++ and build them with g++ 7.5.0 (Ubuntu 7.5.0-6ubuntu2) and -O3 option. 
- **Boost**: Our codes use the [Boost C++ Libraries](https://www.boost.org). 
- **AVX-512 (optional)**: We use 512-bit SIMD instructions to accelerate the operations of HyperBF, and conduct the experiments on a CPU that supports AVX-512 instruction set. We also provide a basic version of HyperBF that does not use SIMD instructions. You can decide which version of HyperBF to build by whether or not defining the `SIMD` macro when calling the `make` command. More details can be found in the folders. 


## CPU Datasets

The datsets used in CPU experiments are files containing many items. We provide two sample datasets in the `datasets` directory, which are extracted from the real-world CAIDA dataset and the Criteo datset, respectively. For the full datasets, please register in [CAIDA](http://www.caida.org/home/) or [Criteo](https://ailab.criteo.com/ressources/) first, and then apply for the traces. 

- ` CAIDA.dat` is a dataset extracted from the CAIDA dataset. CAIDA Anonymized Internet Trace is a data stream of anonymized IP trace collected in 2018. Each item is identified by its source IP address (4 bytes) and destination IP address (4 bytes). 

- `CRITEO.log` is a dataset extracted from the Criteo dataset. Criteo dataset is an online advertising click data stream consisting of about 45M ad impressions. Each item is identified by its categorical feature values and conversion feedback. 


#### Dataset Information

| Dataset    | Freq. of the hottest item | # distincnt items |  # items  |
| :--------- | :-----------------------: | :---------------: | :-------: |
| CAIDA.dat  |          77,254           |      39,954       | 1,000,000 |
| CRITEO.log |           1,910           |      75,228       | 1,000,000 |


## How to run

Please enter the folder `PeriodicBatch`, `Batch`, and `TopK` to run the experiments in finding periodic batches, item batches, and top-k items, respectively. 

More details can be found in the folders. 

