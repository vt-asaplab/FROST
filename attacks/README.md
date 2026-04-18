We implement the differential privacy-based defense mechanism of FROST in the public source code of the following attacks: 

- [Frequency, Graph Matching, SAP, IHOP attacks](https://github.com/simon-oya/USENIX22-ihop-code/)
- [Jigsaw attack](https://github.com/JigsawAttack/JigsawAttack/)

These attacks already provide datasets (e.g., Enron, Lucene, Wiki), and also give instructions on how to obtain datasets. Please refer to their ``README.md`` for more details. 

Install required Python libraries:
```
pip3 install numpy scipy matplotlib scikit-learn tqdm pytrends nltk
```

Make sure that you compile the graphm binary ``graphm-0.52.tar``. The code to this binary should be assigned to the ``GRAPHM_PATH`` variable in **Freq_GraphM_SAP_IHOP/attacks/graphm.py**.

- For Frequency, Graph Matching, SAP, and IHOP attacks, the attack accuracy can be evaluated by going into **Freq_GraphM_SAP_IHOP** folder then execute:
```
python3 debug.py 
```
Parameters (e.g., datasets, true/false positive rate, attack type, etc.) can be configured by modifying the arguments in ``debug.py``. We can change the defense mechanism to **clrz**, **osse**, **frost**, and **none** to evaluate their effectiveness against different pattern leakage attacks.

- For Jigsaw attack, the attack accuracy can be evaluated by going into **Jigsaw** folder then execute:
```
python3 test_against_countermeasure.py
```
Similarly, parameters can be configured by modifying the arguments in ``test_against_countermeasure.py``. 
