# Codes for Finding Item Batches

We compare HyperBF with Clock-Sketch, SWAMP, and Time-Out Bloom filter (TOBF) in finding item batches. Experimental results show that HyperBF outperforms the other algorithms in terms of both accuracy and processing speed. 

## How to run

You can use the following command to build the codes. 

```bash
$ make [USER_DEFINES=-DSIMD]
```

`USER_DEFINES=-DSIMD`:  As mentioned in our paper, we use 512-bit SIMD instructions to accelerate the operations of HyperBF, and conduct the experiments on a CPU that supports AVX-512 instruction set. We also provide a basic version of HyperBF that does not use SIMD instructions. You can decide which version of HyperBF to build by whether or not defining the `SIMD` macro when calling the `make` command.



You can use the following command to run our tests. 

```bash
$ ./batch_test -f FILENAME -s {1-4} [-t REPEAT_TIME] [-k TOPK] [-m MEMORY] [-b BATCH_TIME] [-u UNIT_TIME]
```


1. `-f`: Path of the dataset you want to run.

2. `-s`: An integer(1-4), specifying the algorithm you want to test. The corresponding relationship is as follows. 

   | 1       | 2     | 3    | 4     |
   | ------- | ----- | ---- | ----- |
   | HyperBF | CLOCK | TOBF | SWAMP |

3. `-t`: An integer, specifying the number of repetitions of each execution. The default value is 1.

4. `-k`: An integer, specifying the top-k threshold. The default value is 200. 

5. `-m`: An integer, specifying the memory size (in bytes) used by the algorithm. The default value is $10^4$. 

7. `-b`: The predefined batch threshold that spaces two adjacent item batches. The default value is average interval of items in the dataset.


For example, you can run the following command to test the performance of HyperBF under the default parameter settings. 

```bash
./batch_test -f ../datasets/CAIDA.dat -s 1
```


## Output Format

Our program prints the processing speed, Recall Rate, and Precision Rate of the tested algorithm on the target dataset. 

