# CPU Datasets

The datsets used in CPU experiments are files containing many items. We provide two sample datasets that are extracted from the real-world CAIDA dataset and the Criteo datset, respectively. For the full datasets, please register in [CAIDA](http://www.caida.org/home/) or [Criteo](https://ailab.criteo.com/ressources/) first, and then apply for the traces. 

- `CAIDA.dat` is a dataset extracted from the CAIDA dataset. CAIDA Anonymized Internet Trace is a data stream of anonymized IP trace collected in 2018. Each item is identified by its source IP address (4 bytes) and destination IP address (4 bytes). 

- `CRITEO.log` is a dataset extracted from the Criteo dataset. Criteo dataset is an online advertising click data stream consisting of about 45M ad impressions. Each item is identified by its categorical feature values and conversion feedback. 


## Dataset Information

| Dataset    | Freq. of the hottest item | # distincnt items |  # items  |
| :--------- | :-----------------------: | :---------------: | :-------: |
| CAIDA.dat  |          77,254           |      39,954       | 1,000,000 |
| CRITEO.log |           1,910           |      75,228       | 1,000,000 |


## Notification 

These data files are only used for testing the performance of HyperCalm and the related algorithms in this project. Please do not use these traces for other purpose. 
