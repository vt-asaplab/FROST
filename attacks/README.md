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
python3 debug.py [--nqr <number_of_queries>] [--nkw <number_of_keywords>] [--tpr <true_positive_rate>] [--fpr <false_positive_rate>] [--dataset <dataset>] [--att <attack_type>] [--defense <defense_type>]
```

For example: 

```
python3 debug.py 
```

It executes with default parameters: nqr = 1000, nkw = 1000, tpr = 0.95, fpr = 0.1, dataset = Enron-full, att = freq (Frequency attack), defense = frost. 

The dataset can be "**Enron-full**", "**lucene**", and "**wiki_sec**" (see available datasets in the **datasets_pro** folder). The attack can be "**freq**", "**ihop**", "**sap**", and "**graphm**". We can change the defense mechanism to "**clrz**", "**osse**", "**frost**", and "**none**" (no defense) to evaluate their effectiveness against different pattern leakage attacks.

- For Jigsaw attack, the attack accuracy can be evaluated by going into **Jigsaw** folder then execute:
```
python3 test_against_countermeasure.py
```
Similarly, parameters can be configured by modifying the arguments in ``test_against_countermeasure.py``. 
