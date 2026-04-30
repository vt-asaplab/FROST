import pickle
import time
from tqdm import tqdm

import numpy as np
kws_extraction = "random"
kws_universe_size = 3000

print("read kws dict:")
time1 = time.time()
with open("./wiki_kws_dict.pkl","rb") as f:
    kws_dict = pickle.load(f)
print(time.time() - time1)

kws_list = list(kws_dict.keys())
np.random.shuffle(kws_list)
kws_list = kws_list[:kws_universe_size]

kws_dict_subset = {}
for kws in kws_list:
    kws_dict_subset[kws] = kws_dict[kws]
with open("kws_dict_"+str(kws_universe_size)+".pkl", "wb") as f:
    pickle.dump(kws_dict_subset,f)

print("read doc")
time1 = time.time()
with open("./wiki_doc_0.pkl","rb") as f:
    doc = pickle.load(f)
print(time.time() - time1)
print("doc number:",len(doc))
print("kws number:",len(kws_list))
doc_kwsid = np.zeros((len(doc),len(kws_list)),bool)

for i in tqdm(range(len(doc))):
    for j in range(len(kws_list)):
        if kws_list[j] in doc[i]:
            doc_kwsid[i][j]=1

time1 = time.time()
if kws_universe_size <= 3000:
    with open("./kws_list_and_doc_kws_all_new_0.pkl", "wb") as f:
        pickle.dump([kws_list,doc_kwsid],f)
else:
    with open("./kws_list_and_doc_kws_new_"+str(kws_universe_size)+"_0.pkl", "wb") as f:
        pickle.dump([kws_list,doc_kwsid],f)
print(time.time() - time1)
