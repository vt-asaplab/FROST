We implement the differential privacy-based defense mechanism of FROST in the public source code of the following attacks: 

- [Frequency, Graph Matching, SAP, IHOP attacks](https://github.com/simon-oya/USENIX22-ihop-code/)
- [Jigsaw attack](https://github.com/JigsawAttack/JigsawAttack/)

These attacks already provide datasets (e.g., Enron, Lucene, Wiki), and also give instructions on how to obtain datasets. Please refer to their ``README.md`` for more details. 

Install required Python libraries:
```
pip3 install numpy scipy matplotlib scikit-learn tqdm pytrends nltk
```

Make sure that you compile the graphm binary ``graphm-0.52.tar``. The code to this binary should be assigned to the ``GRAPHM_PATH`` variable in **Freq_GraphM_SAP_IHOP/attacks/graphm.py**.

- For the Frequency, Graph Matching, SAP, and IHOP attacks, the attack accuracy can be evaluated by going into **Freq_GraphM_SAP_IHOP** folder then execute:

```
python3 debug.py [--nqr <number_of_queries>] [--nkw <number_of_keywords>] [--tpr <true_positive_rate>] [--fpr <false_positive_rate>] [--dataset <dataset>] [--att <attack_type>] [--defense <defense_mechanism>]
```

For example: 

```
python3 debug.py 
```

It executes with default parameters: number_of_queries = 1,000, number_of_keywords = 1,000, true_positive_rate = 0.95, false_positive_rate = 0.1, dataset = Enron-full, attack_type = freq (Frequency attack), and defense_mechanism = frost. 

The dataset can be "**enron-full**", "**lucene**", and "**wiki_sec**" (see available datasets in the **datasets_pro** folder). The attack can be "**freq**", "**ihop**", "**sap**", and "**graphm**". We can change the defense mechanism to "**clrz**", "**osse**", "**frost**", and "**none**" (no defense) to evaluate their effectiveness against different statistical leakage-abuse attacks.

- For the Jigsaw attack, the attack accuracy can be evaluated by going into **Jigsaw** folder then execute:
```
python3 test_against_countermeasure.py [--nqrpw <number_of_queries_per_week>] [--nw <number_of_weeks>] [--nkw <number_of_keywords>] [--tpr <true_positive_rate>] [--fpr <false_positive_rate>] [--dataset <dataset>] [--defense <defense_mechanism>]
```

For example:
```
python3 test_against_countermeasure.py
```

It executes with default parameters: number_of_queries_per_week = 500, number_of_weeks = 50 (in total of 25,000 queries), number_of_keywords = 1,000, true_positive_rate = 0.95, false_positive_rate = 0.1, dataset = enron, and defense_mechanism = frost. 

The dataset can be "**enron**", "**lucene**", and "**wiki**". We can change the defense mechanism to "**clrz**", "**osse**", "**frost**", and "**none**" (no defense) to evaluate their effectiveness against different pattern leakage attacks.
