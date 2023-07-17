# Codes for Finding Periodic Batches

We combine the state-of-the-art Clock-Sketch and Unbiased Space-Saving to form a strawman solution for finding top-k periodic batches (Clock+USS), and compare our Bucketized HyperCalm with it. And we also compare Bucketized HyperCalm with Basic HyperCalm, which is the base version of Bucketized HyperCalm. Experimental results show that our Bucketized HyperCalm outperforms the strawman solution in terms of both accuracy and processing speed. 

## How to run

You can use the following command to build the codes. 

```bash
$ make [USER_DEFINES=-DSIMD]
```

`USER_DEFINES=-DSIMD`:  As mentioned in our paper, we use 512-bit SIMD instructions to accelerate the operations of Bucketized HyperCalm, and conduct the experiments on a CPU that supports AVX-512 instruction set. We also provide a basic version of Bucketized HyperCalm that does not use SIMD instructions. You can decide which version of Bucketized HyperCalm to build by whether or not defining the `SIMD` macro when calling the `make` command.



You can use the following command to run our tests. 

```bash
$ make
$ ./periodic_batch_test -f FILENAME -s {1-3} [-t REPEAT_TIME] [-k TOPK] [-l LIMIT] [-m MEMORY] [-b BATCH_TIME] [-u UNIT_TIME]
```

1. `-f`: Path of the dataset you want to run.

2. `-s`: An integer(1-3), specifying the algorithm you want to test. The corresponding relationship is as follows. 

   | 1               | 2                    | 3         |
   | --------------- | -------------------- | --------- |
   | HYPERCALM_BASIC | HYPERCALM_BUCKETIZED | CLOCK_USS |

3. `-t`: An integer, specifying the number of repetitions of each execution. The default value is 1.

4. `-k`: An integer, specifying the top-k threshold. The default value is 200. 

5. `-l`: An integer(1-4), specifying the limit of batch size. The default value is 1. This parameter is used to filter small batches in the experiments of finding periodic large batches.

6. `-m`: An integer, specifying the memory size (in bytes) used by the algorithm. The default value is $5 \times 10^5$. 

7. `-b`: The predefined batch threshold that spaces two adjacent item batches. The default value is average interval $\div 10$ of items in the dataset. 

8. `-u`: The unit time for detecting periodic batch (each batch interval is rounded down to the nearest multiple of `UNIT_TIME`). The default value is $10\times$ BATCH_TIME_THRESHOLD.  


For example, you can run the following command to test the performance of HyperCalm under the default parameter settings. 

```bash
./periodic_batch_test -f ../../datasets/CAIDA.dat -s 1
```


## Output Format

Our program prints the processing speed, Recall Rate, Average Absolute Error (AAE), and Average Relative Error (ARE) of the tested algorithm on the target dataset. 


## Note

Our code enables 2bit counter in HyperBF by default. If you want to test Bucketized HyperCalm without counter, please set parameter l to 1 and modify the parameter counterType in HyperCalm/HyperBloomFilter.h to None.


