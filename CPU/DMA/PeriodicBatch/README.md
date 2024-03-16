# Codes for Variable HyperCalm

We implemented dynamic version of Bucketized HyperCalm, and tested its performance with different number of memory expansions/compressions. The experimental results demonstrate that the dynamic version of Bucketized HyperCalm still maintains similar performances in terms of both accuracy and processing speed as the normal version of Bucketized HyperCalm.

## How to run

You can use the following command to build the codes. 

```bash
$ make 
```

You can use the following command to run our tests. 

```bash
$ make
$ ./periodic_batch_test -f FILENAME -v {1-2} [-t REPEAT_TIME] [-k TOPK] [-r RATE] [-m MEMORY] [-b BATCH_TIME] [-u UNIT_TIME]
```

1. `-f`: Path of the dataset you want to run.

2. `-v`: An integer(1-2), specifying the dynamic type you want to test. The corresponding relationship is as follows. 

   | 1        | 2      |
   | -------- | ------ | 
   | compress | extend | 

3. `-t`: An integer, specifying the number of repetitions of each execution. The default value is 1.

4. `-k`: An integer, specifying the top-k threshold. The default value is 200. 

5. `-r`: An integer($\geq 0$), specifying the number of expansions/compressions. The default value is 0. 

6. `-m`: An integer, specifying the memory size (in bytes) used by the algorithm. The default value is $5 \times 10^5$. 

7. `-b`: The predefined batch threshold that spaces two adjacent item batches. The default value is average interval $\div 10$ of items in the dataset. 

8. `-u`: The unit time for detecting periodic batch (each batch interval is rounded down to the nearest multiple of `UNIT_TIME`). The default value is $10\times$ BATCH_TIME_THRESHOLD.  


For example, you can run the following command to test the performance of compressible HyperCalm under the default parameter settings. 

```bash
./periodic_batch_test -f ../../../datasets/CAIDA.dat -v 1
```


## Output Format

Our program prints the processing speed, Recall Rate, Average Absolute Error (AAE), and Average Relative Error (ARE) of the tested algorithm on the target dataset. 


## Note

If you want to test compressible HyperCalm, assuming the number of compressions is r, you need to adjust the number of arrays in CalmSS to be a multiple of $2^r$ at initialization time so that it can support r compressions. To make this easy to implement, you can do this by modifying the hbit function in SpaceSaving.h.


