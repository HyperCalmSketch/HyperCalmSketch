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
$ ./batch_test -f FILENAME -s {1-4} -c {1-3} [-t REPEAT_TIME] [-k TOPK] [-l LIMIT] [-m MEMORY] [-b BATCH_TIME] [-u UNIT_TIME]
```


1. `-f`: Path of the dataset you want to run.

2. `-s`: An integer(1-4), specifying the algorithm you want to test. The corresponding relationship is as follows. 

   | 1       | 2     | 3    | 4     |
   | ------- | ----- | ---- | ----- |
   | HyperBF | CLOCK | TOBF | SWAMP |

3. `-c`: An integer(1-3), specifying the type of test you want to perform. The corresponding relationship is as follows. 

   | 1          | 2         | 3               | 
   | ---------- | --------- | --------------- | 
   | size_test  | hit_test  | large_hit_test  | 

4. `-t`: An integer, specifying the number of repetitions of each execution. The default value is 1.

5. `-k`: An integer, specifying the top-k threshold. The default value is 200. 

6. `-l`: An integer(1-4), specifying the limit of batch size. The default value is 1.

7. `-m`: An integer, specifying the memory size (in bytes) used by the algorithm. The default value is $10^4$. 

8. `-b`: The predefined batch threshold that spaces two adjacent item batches. The default value is average interval of items in the dataset.

9. `-u`: The unit time for detecting periodic batch (each batch interval is rounded down to the nearest multiple of `UNIT_TIME`). The default value is $10\times$ BATCH_TIME_THRESHOLD.  



For example, you can run the following command to test HyperBF's performance on the "hit_test" test with the default parameter settings. 

```bash
./batch_test -f ../../datasets/CAIDA.dat -s 1 -c 1
```


## Output Format

### For Size Test:
Our program prints the Average Absolute Error (AAE) and Average Relative Error (ARE) of the tested algorithm on the target dataset.
### For Hit Test and Large Hit Test:
Our program prints the processing speed, Recall Rate and F1 Score of the tested algorithm on the target dataset. 

## Note

Our code enables 2bit counter in HyperBF by default, if you don't want to enable it, please modify the parameter counterType in HyperCalm/HyperBloomFilter.h to None

