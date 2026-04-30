import pickle
import time
from tqdm import tqdm
from cal_acc import calculate_acc_weighted
from run_single_attack import *
import os
import argparse

def run_Ours_IHOP_and_RSA_against_countermeasure(countermeasure,tpr,fpr,test_times=1,kws_uni_size=1000,\
                                                datasets=["enron"],kws_extraction="random",observe_query_number_per_week = 500,\
                                                observe_weeks = 50,time_offset = 0,refspeed=5,beta=0.9):
    if not os.path.exists("./results"):
        os.makedirs("./results")
    if not os.path.exists("./results/test_against_countermeasures"):
        os.makedirs("./results/test_against_countermeasures")
    print("Test Ours IHOP and RSA against countermeasure")
    for dataset in datasets:
        if countermeasure =="padding_linear_2":
            if dataset == "wiki":
                Countermeasure_params = [
                    {"alg":"padding_linear_2","n":0},
                    {"alg":"padding_linear_2","n":50000},
                    {"alg":"padding_linear_2","n":100000},
                    {"alg":"padding_linear_2","n":150000}]
            else:
                Countermeasure_params = [{"alg":"padding_linear_2","n":0},\
                    {"alg":"padding_linear_2","n":500},
                    {"alg":"padding_linear_2","n":1000},
                    {"alg":"padding_linear_2","n":1500}]
        elif countermeasure == "clrz" or countermeasure == "osse" or countermeasure == "frost":
            if dataset == "wiki":
                Countermeasure_params=[{"alg":countermeasure,"p":tpr,"q":fpr,"m":1}]
                
                # Countermeasure_params=[{"alg":countermeasure,"p":0.95,"q":0.2,"m":1},\
                #     {"alg":countermeasure,"p":0.95,"q":0.15,"m":1},\
                #     {"alg":countermeasure,"p":0.95,"q":0.1,"m":1},\
                #     {"alg":countermeasure,"p":0.95,"q":0.05,"m":1},\
                #     {"alg":countermeasure,"p":0.95,"q":0.025,"m":1}
                #     ]
            else:
                Countermeasure_params=[{"alg":countermeasure,"p":tpr,"q":fpr,"m":1}]
                
                # Countermeasure_params=[{"alg":countermeasure,"p":0.95,"q":0.2,"m":1},\
                #     {"alg":countermeasure,"p":0.95,"q":0.15,"m":1},\
                #     {"alg":countermeasure,"p":0.95,"q":0.1,"m":1},\
                #     {"alg":countermeasure,"p":0.95,"q":0.05,"m":1},\
                #     {"alg":countermeasure,"p":0.95,"q":0.025,"m":1}
                #     ]
        elif countermeasure == "padding_cluster":
            Countermeasure_params = [
                {"alg":"padding_cluster","knum_in_cluster":1},\
                {"alg":"padding_cluster","knum_in_cluster":2},
                {"alg":"padding_cluster","knum_in_cluster":4},
                {"alg":"padding_cluster","knum_in_cluster":8}]
        elif countermeasure == "padding_seal":
            Countermeasure_params = [
                {"alg":"padding_seal","n":1},
                {"alg":"padding_seal","n":2},
                {"alg":"padding_seal","n":3},
                {"alg":"padding_seal","n":4},
            ]
        else:
            Countermeasure_params = [{"alg": None}]
        
        for countermeasure_params in Countermeasure_params:
            Our_Result = []
            RSA_Result = []
            IHOP_Result = []
            Our_acc = []
            for i in tqdm(range(test_times)):
                rsa_attack_params={
                    "alg": "RSA",
                    "refinespeed":refspeed,
                    "known_query_number":15
                }
                ihop_attack_params={
                    "alg":"IHOP",
                    "niters":500,
                    "pfree":0.25,
                    "no_F":False
                    }
                our_attack_params={
                    "alg": "Ours",
                    "refinespeed":refspeed,
                    "alpha":0.1,
                    "beta":0.9,
                    "baseRec":15,
                    "confRec":10,
                    "step":3,
                    "no_F":False
                }
                if dataset == "wiki":
                    our_attack_params["refinespeed_exp"] = True
                    rsa_attack_params["refinespeed_exp"] = True
                else:
                    our_attack_params["refinespeed_exp"] = False
                    rsa_attack_params["refinespeed_exp"] = False

##################Our###################
                print(kws_uni_size,kws_uni_size,kws_extraction,observe_query_number_per_week,\
                    observe_weeks,time_offset,dataset,
                countermeasure_params,our_attack_params)
                result = run_single_attack(kws_uni_size,kws_uni_size,kws_extraction,observe_query_number_per_week,\
                    observe_weeks,time_offset,dataset,
                countermeasure_params,our_attack_params)
                
                data_for_acc_cal = result["data_for_acc_cal"]

                correct_count,acc,correct_id,wrong_id = \
                    calculate_acc_weighted(data_for_acc_cal,result["results"][0])
                print({"Ours step1:  dataset":dataset,"countermeasure_params":countermeasure_params,"acc":acc})

                correct_count,acc,correct_id,wrong_id = \
                    calculate_acc_weighted(data_for_acc_cal,result["results"][1])
                print({"Ours step2:  dataset":dataset,"countermeasure_params":countermeasure_params,"acc":acc})


                correct_count,acc,correct_id,wrong_id = \
                    calculate_acc_weighted(data_for_acc_cal,result["results"][2])
                print({"Ours:  dataset":dataset,"countermeasure_params":countermeasure_params,"acc":acc})
                
                Our_Result.append((dataset,countermeasure_params,acc,result))
                Our_acc.append(acc)
                
            if countermeasure_params["alg"] == "padding_linear_2":
                with open("./results/test_against_countermeasures/Ours_"+dataset+\
                    "_padding_linear_n_"+str(countermeasure_params["n"])+\
                    "_kws_uni_size_"+str(kws_uni_size)+\
                    "_test_times_"+str(test_times)+".pkl", "wb") as f:
                    pickle.dump(Our_Result,f)
                with open("./results/test_against_countermeasures/RSA_"+dataset+\
                        "_padding_linear_n_"+str(countermeasure_params["n"])+\
                        "_kws_uni_size_"+str(kws_uni_size)+\
                        "_test_times_"+str(test_times)+".pkl", "wb") as f:
                        pickle.dump(RSA_Result,f)
                with open("./results/test_against_countermeasures/IHOP_"+dataset+\
                    "_padding_linear_n_"+str(countermeasure_params["n"])+\
                    "_kws_uni_size_"+str(kws_uni_size)+\
                    "_test_times_"+str(test_times)+".pkl", "wb") as f:
                    pickle.dump(IHOP_Result,f)
            elif countermeasure_params["alg"] == "obfuscation":
                with open("./results/test_against_countermeasures/Ours_"+dataset+\
                    "_obfuscation_q_"+str(countermeasure_params["q"])+\
                    "_kws_uni_size_"+str(kws_uni_size)+\
                    "_test_times_"+str(test_times)+".pkl", "wb") as f:
                    pickle.dump(Our_Result,f)
                with open("./results/test_against_countermeasures/RSA_"+dataset+\
                    "_obfuscation_q_"+str(countermeasure_params["q"])+\
                    "_kws_uni_size_"+str(kws_uni_size)+\
                    "_test_times_"+str(test_times)+".pkl", "wb") as f:
                    pickle.dump(RSA_Result,f)
                with open("./results/test_against_countermeasures/IHOP_"+dataset+\
                    "_obfuscation_q_"+str(countermeasure_params["q"])+\
                    "_kws_uni_size_"+str(kws_uni_size)+\
                    "_test_times_"+str(test_times)+".pkl", "wb") as f:
                    pickle.dump(IHOP_Result,f)
            elif countermeasure_params["alg"] == "padding_cluster":
                with open("./results/test_against_countermeasures/Ours_"+dataset+\
                    "_padding_cluster_knum_in_cluster_"+str(countermeasure_params["knum_in_cluster"])+\
                    "_kws_uni_size_"+str(kws_uni_size)+\
                    "_test_times_"+str(test_times)+".pkl", "wb") as f:
                    pickle.dump(Our_Result,f)
                with open("./results/test_against_countermeasures/RSA_"+dataset+\
                    "_padding_cluster_knum_in_cluster_"+str(countermeasure_params["knum_in_cluster"])+\
                    "_kws_uni_size_"+str(kws_uni_size)+\
                    "_test_times_"+str(test_times)+".pkl", "wb") as f:
                    pickle.dump(RSA_Result,f)
                with open("./results/test_against_countermeasures/IHOP_"+dataset+\
                    "_padding_cluster_knum_in_cluster_"+str(countermeasure_params["knum_in_cluster"])+\
                    "_kws_uni_size_"+str(kws_uni_size)+\
                    "_test_times_"+str(test_times)+".pkl", "wb") as f:
                    pickle.dump(IHOP_Result,f)
            elif countermeasure_params["alg"] == "padding_seal":
                with open("./results/test_against_countermeasures/Ours_"+dataset+\
                    "_padding_seal_"+str(countermeasure_params["n"])+\
                    "_kws_uni_size_"+str(kws_uni_size)+\
                    "_test_times_"+str(test_times)+".pkl", "wb") as f:
                    pickle.dump(Our_Result,f)
                with open("./results/test_against_countermeasures/RSA_"+dataset+\
                        "_padding_seal_"+str(countermeasure_params["n"])+\
                        "_kws_uni_size_"+str(kws_uni_size)+\
                        "_test_times_"+str(test_times)+".pkl", "wb") as f:
                        pickle.dump(RSA_Result,f)
                with open("./results/test_against_countermeasures/IHOP_"+dataset+\
                    "_padding_seal_"+str(countermeasure_params["n"])+\
                    "_kws_uni_size_"+str(kws_uni_size)+\
                    "_test_times_"+str(test_times)+".pkl", "wb") as f:
                    pickle.dump(IHOP_Result,f)
    return 0

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Executing Jigsaw Attack")

    parser.add_argument("--nqrpw", type=int, default=500, help="Number of observed queries per week")
    parser.add_argument("--nw", type=int, default=50, help="Number of observed weeks")
    parser.add_argument("--nkw", type=int, default=1000, help="Number of keywords")
    parser.add_argument("--tpr", type=float, default=0.95, help="True positive rate")
    parser.add_argument("--fpr", type=float, default=0.1, help="False positive rate")
    parser.add_argument("--dataset", type=str, default="enron", help="Dataset")
    parser.add_argument("--defense", type=str, default="frost", help="Defense type")
    
    args = parser.parse_args()

    run_Ours_IHOP_and_RSA_against_countermeasure(args.defense,args.tpr,args.fpr,\
        test_times=10,kws_uni_size=args.nkw,datasets=[args.dataset],kws_extraction="random",\
        observe_query_number_per_week=args.nqrpw,observe_weeks=args.nw) 
    
    # run_Ours_IHOP_and_RSA_against_countermeasure("frost",\
    #     test_times=10,kws_uni_size=1000,datasets=["enron","lucene"],kws_extraction="random") 
  
    # run_Ours_IHOP_and_RSA_against_countermeasure("frost",\
    #     test_times=10,kws_uni_size=3000,datasets=["wiki"],kws_extraction="random",observe_query_number_per_week=3000,observe_weeks=30)
    
    # run_Ours_IHOP_and_RSA_against_countermeasure("frost",\
    #     test_times=10,kws_uni_size=5000,datasets=["wiki"],kws_extraction="random",observe_query_number_per_week=5000,observe_weeks=30)



