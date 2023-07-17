# Codes for Finding Periodic Batches

We combine the state-of-the-art Clock-Sketch and Unbiased Space-Saving to form a strawman solution for finding top-k periodic batches (Clock+USS), and compare our HyperCalm with it. Experimental results show that our HyperCalm outperforms the strawman solution in terms of both accuracy and processing speed. 

## How to run

You can use the following commands to build and run our tests. 

```bash
$ make
$ ./periodic_batch_test -f FILENAME -s {1-2} [-t REPEAT_TIME] [-k TOPK] [-m MEMORY] [-b BATCH_TIME] [-u UNIT_TIME]
```

1. `-f`: Path of the dataset you want to run.

2. `-s`: An integer(1-2), specifying the algorithm you want to test. The corresponding relationship is as follows. 

   | 1         | 2         |
   | --------- | --------- |
   | HYPERCALM | CLOCK_USS |

3. `-t`: An integer, specifying the number of repetitions of each execution. The default value is 1.

4. `-k`: An integer, specifying the top-k threshold. The default value is 200. 

5. `-m`: An integer, specifying the memory size (in bytes) used by the algorithm. The default value is $5 \times 10^5$. 

6. `-b`: The predefined batch threshold that spaces two adjacent item batches. The default value is average interval $\div 10$ of items in the dataset. 

7. `-u`: The unit time for detecting periodic batch (each batch interval is rounded down to the nearest multiple of `UNIT_TIME`). The default value is $10\times$ BATCH_TIME_THRESHOLD.  


For example, you can run the following command to test the performance of HyperCalm under the default parameter settings. 

```bash
./periodic_batch_test -f ../datasets/CAIDA.dat -s 1
```


## Output Format

Our program prints the processing speed, Recall Rate, Average Absolute Error (AAE), and Average Relative Error (ARE) of the tested algorithm on the target dataset. 





