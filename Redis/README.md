# Codes for Redis HyperCalm

We implement HyperCalm in Redis database, a popular in-memory data structure store widely used by database, cache, and streaming engine, showing that HyperCalm can be easily integrated into mainstream KV databases

## How to run

You can use the following commands to build and run our tests. 

```bash
$ make
$ redis-server --loadmodule ./RedisSketches.so
$ python RedisControl.py -f FILENAME -t REPEAT_TIME -m memory -b BATCH_TIME
```

1. `-f`: Path of the dataset you want to run.	

2. `-t`: An integer, specifying the number of repetitions of each execution. 

3. `-m`: An integer, specifying the memory size (in bytes) used by the algorithm. 

4. `-b`: The predefined batch threshold that spaces two adjacent item batches.

For example, you can run the following command to test the performance of CalmSS under the default parameter settings. 

```bash
python RedisControl.py -f ../datasets/CAIDA.dat -t 1 -m 100000 -b 0.00072
```

## Output Format

Our program prints the throughput of Hyper Bloom Filter, Calm Space-Saving and HyperCalm on the target dataset. 