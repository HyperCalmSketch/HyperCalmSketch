# Codes on CPU Platform 

We conduct CPU experiments to compare the performance of Buckerized HyperCalm, Basic HyperCalm and a strawman solution (Clock-Sketch + Unbiased Space-Saving) in finding periodic batches. We compare the performance of HyperBF, Clock-Sketch, Time-Out Bloom filter, and SWAMP in detecting item batches, and experimented with filtering out small batches by adding counters to HyperBF.And we compare the performance of Bucketized CalmSS, Basic Calmss, Space-Saving, Unbiased Space-Saving (USS) and Cold-Filter + SS (CF+SS) in finding top-k items. We also implemented HyperCalm in redis and tested its speed.

Experimental results show that Bucketized HyperCalm well achieves our design goal. Bucketized HyperCalm outperforms the Basic HyperCalm 14.1 times in term of average relative error and 1.6 times in term of processing speed, when Basic HyperCalm outperforms the strawman solutions 4 times in term of average relative error and 13.2 times in term of processing speed. Besides, we find that both HyperBF and Bucketized CalmSS, significantly outperform the state-of-the-art solutions in detecting item batches and finding top-k items, respectively. Our experiments are conducted on an 18-core CPU server (Intel i9-10980XE) with 128GB DDR4 memory and 24.75MB L3 cache. We set the CPU frequency to 4.2GHz and set the memory frequency to 3200MHz. 


## File Structure 

- `HyperCalm`: Source codes for the HyperCalm sketch, including HyperBF, Basic CalmSS, Bucketized CalmSS. 
- `ComparedAlgorithms`: Source codes for the related algorithms in our paper, including Time-Out Bloom filter (TOBF), Clock-Sketch, SWAMP, Space-Saving (SS), Unbiased Space-Saving (USS), Cold-Filter + Space-Saving (CF+SS). 
- `Batch`: Source codes for detecting item batches. 
- `PeriodicBatch`: Source codes for finding top-k periodic batches.  
- `TopK`: Source codes for finding top-k items.
- `lib`: The hash table, hash function, and some common functions used by many algorithms. 

## Dependency 

- **Murmur Hash**: The hash functions we use are 32-bit Murmur hash functions, obtained [here](https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp).
- **g++, C++17, CMake**: We implement all the codes with C++ and build them with g++ 7.5.0 (Ubuntu 7.5.0-6ubuntu2) and -O3 option. 
- **Boost**: Our codes use the [Boost C++ Libraries](https://www.boost.org). 
- **AVX-512 (optional)**: We use 512-bit SIMD instructions to accelerate the operations of HyperCalm, and conduct the experiments on a CPU that supports AVX-512 instruction set. We also provide a basic version of HyperCalm that does not use SIMD instructions. You can decide which version of HyperCalm to build by whether or not defining the `SIMD` macro when calling the `make` command. More details can be found in the folders. 


## How to run

Please enter the folder `PeriodicBatch`, `Batch`, and `TopK` to run the experiments in finding periodic batches, item batches, and top-k items, respectively. 

More details can be found in the folders. 

