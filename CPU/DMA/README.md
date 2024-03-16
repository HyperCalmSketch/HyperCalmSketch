# Codes for Dynamic HyperCalm

In practice, the density of data streams and the available memory resources might vary dynamically. It is desirable to perform on-the-fly reconfiguration on the sketchsize to adapt to these dynamic variations. Towards this goal,we implemented the dynamic memory adjustment operations for HyperBF and Bucketized TimeRecorder/LRU-Queue/Space-Saving, by which we can dynamically compress and expand their sizes by any integer factor. These operations allow us to dynamically adjust the memory: for HyperCalm sketch without losing the previously recorded information. 

Experimental results show that HyperBF/HyperCalm can flexibly manage the trade-off between its accuracy and memory usage via the memory adjustment operations, and the memory adjustment operations of HyperBF/HyperCalm can be efficiently completed within milliseconds.


## File Structure 

- `HyperCalm`: Source codes for the Dynamic Bucketized HyperCalm sketch, including Dynamic HyperBF, Dynamic Bucketized CalmSS. 
- `Batch`: Source codes for detecting item batches using Dynamic HyperBF. 
- `PeriodicBatch`: Source codes for finding top-k periodic batches using Dynamic Bucketized HyperCalm.  

## Dependency 

- **Murmur Hash**: The hash functions we use are 32-bit Murmur hash functions, obtained [here](https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp).
- **g++, C++17, CMake**: We implement all the codes with C++ and build them with g++ 7.5.0 (Ubuntu 7.5.0-6ubuntu2) and -O3 option. 
- **Boost**: Our codes use the [Boost C++ Libraries](https://www.boost.org). 
- **AVX-512 (optional)**: We use 512-bit SIMD instructions to accelerate the operations of HyperCalm, and conduct the experiments on a CPU that supports AVX-512 instruction set. We also provide a basic version of HyperCalm that does not use SIMD instructions. You can decide which version of HyperCalm to build by whether or not defining the `SIMD` macro when calling the `make` command. More details can be found in the folders. 


## How to run

Please enter the folder `PeriodicBatch` and `Batch` to run the experiments in finding periodic batches, item batches, and top-k items, respectively. 

More details can be found in the folders. 

