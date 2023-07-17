# Appliction in Network Measurement

We evaluate the performance of HyperCalm and Bucketized HyperCalm in performing two network measurement tasks: finding packet drops and finding inflated delays. Experimental results show that our HyperCalm sketch achieves high Recall Rate, Precision Rate in finding network anomalies. In addition, HyperCalm sketch has high accuracy in estimating batch sizes and batch timespans.

## How to run

You can use the following command to build the codes. 

```bash
$ make 
```

You can use the following command to run our tests. 

```bash
$ make
$ ./delay_loss_test -f FILENAME -s {1-2} -c {1-2} [-t REPEAT_TIME] [-k TOPK] [-m MEMORY] [-b BATCH_TIME] [-u UNIT_TIME]
```

1. `-f`: Path of the dataset you want to run.

2. `-s`: An integer(1-2), specifying the algorithm you want to test. The corresponding relationship is as follows. 

   | 1                | 2                     |
   | ---------------- | --------------------- |
   | HYPERCALM_BASIC  | HYPERCALM_BUCKETIZED  |

3. `-c`: An integer(1-2), specifying the type you want to test. The corresponding relationship is as follows. 

   | 1      | 2     |
   | ------ | ----- |
   | Delay  | Loss  |

4. `-t`: An integer, specifying the number of repetitions of each execution. The default value is 1.

5. `-k`: An integer, specifying the top-k threshold. The default value is 200. 

6. `-m`: An integer, specifying the memory size (in bytes) used by the algorithm. The default value is $5 \times 10^5$. 

7. `-b`: The predefined batch threshold that spaces two adjacent item batches. The default value is average interval $\div 10$ of items in the dataset. 

8. `-u`: The unit time for detecting periodic batch (each batch interval is rounded down to the nearest multiple of `UNIT_TIME`). The default value is $10\times$ BATCH_TIME_THRESHOLD.  


For example, you can run the following command to test the performance of HyperCalm under the default parameter settings. 

```bash
./delay_loss_test -f ../datasets/CAIDA.dat -s 1 -c 1
```


## Output Format

Our program prints the Recall Rate, Percision Rate, and F1 Score of the tested algorithm on the target dataset. 



