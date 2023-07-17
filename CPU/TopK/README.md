# Codes for Finding Top-k Items

We compare CalmSS with Space-Saving (SS) and Unbiased Space-Saving (USS). Experimental results show that CalmSS outperforms the other algorithms in terms of both accuracy and processing speed. 

## How to run

You can use the following commands to build and run our tests. 

```bash
$ make
$ ./topk_test -f FILENAME -s SKETCHNAME [-t REPEAT_TIME] [-k TOPK] [-m memory]
```

1. `-f`: Path of the dataset you want to run.	

2. `-s`: An integer(1-3), specifying the algorithm you want to test. The corresponding relationship is as follows. 

   | 1       | 2    | 3    |
   | ------- | ---- | ---- |
   | CALM SS | SS   | USS  |

3. `-t`: An integer, specifying the number of repetitions of each execution. The default value is 1.

4. `-k`: An integer, specifying the top-k threshold. The default value is 200. 

5. `-m`: An integer, specifying the memory size (in bytes) used by the algorithm. The default value is $5 \times 10^5$. 

For example, you can run the following command to test the performance of CalmSS under the default parameter settings. 

```bash
./topk_test -f ../datasets/CAIDA.dat -s 1
```

## Output Format

Our program prints the processing speed, Recall Rate, Average Absolute Error (AAE), and Average Relative Error (ARE) of the tested algorithm on the target dataset. 