import os
import sys
import time
from experiment import run_experiment
import numpy as np
from exp_params import ExpParams
import argparse

def print_exp_to_run(parameter_dict, n_runs):
    for key in parameter_dict:
        print('  {:s}: {}'.format(key, parameter_dict[key]))
    print("* Number of runs: {:d}".format(n_runs))


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Executing Freq/GraphM/SAP/IHOP Attack")

    parser.add_argument("--nqr", type=int, default=1000, help="Number of queries")
    parser.add_argument("--nkw", type=int, default=1000, help="Number of keywords")
    parser.add_argument("--tpr", type=float, default=0.95, help="True positive rate")
    parser.add_argument("--fpr", type=float, default=0.1, help="False positive rate")
    parser.add_argument("--dataset", type=str, default="enron-full", help="Dataset")
    parser.add_argument("--att", type=str, default="freq", help="Attack type")
    parser.add_argument("--defense", type=str, default="frost", help="Defense type")
    
    args = parser.parse_args()
    
    os.system('mesg n')
    
    time_init = time.time()
    PRO_DATASETS_PATH = 'datasets_pro'
    EXPERIMENTS_PATH = 'results'
    
    exp_params = ExpParams()
    ###### Simple Enron experiment; accuracy for seed=0 should be 0.058, 0.174, 0.904, and 0.952 for niters=0, 10, 100, and 1000, respectively.
    exp_params.set_general_params(dataset=args.dataset, nkw=args.nkw, nqr=args.nqr, ndoc=30_000, freq='file', mode_ds='splitn10000',
                                  mode_fs='past', mode_kw='rand', mode_query='iid')
    
    attack_map = {
        "freq": ('freq', {}),
        "ihop": ('ihop', {'mode': 'Vol', 'niters': 1000, 'pfree': 0.25}),
        "sap": ('sap', {'alpha': 0.5}),
        "graphm": ('graphm', {'alpha': 0.5}),
    }
    attack_list = [attack_map[args.att]]
    
    if args.defense == "frost":
        exp_params.set_defense_params('frost', tpr=args.tpr, fpr=args.fpr)
    elif args.defense == "osse":
        exp_params.set_defense_params('osse', tpr=args.tpr, fpr=args.fpr)
    elif args.defense == "clrz":
        exp_params.set_defense_params('clrz', tpr=args.tpr, fpr=args.fpr)
    else:
        exp_params.set_defense_params('none') 
    
    # attack_list = [('freq', {}),
    #                ('ihop', {'mode': 'Vol', 'niters': 1000, 'pfree': 0.25}),
    #                ('sap', {'alpha': 0.5}),
    #                ('graphm', {'alpha': 0.5})]
    niter_list = [0, 10, 100, 1000]

    ##### Other examples of param initialization
    # For frequency attack: 
    # exp_params.set_general_params(dataset='enron-full', nkw=1000, nqr=100000, ndoc=30_000, freq='zipf', mode_ds='splitn10000', mode_fs='past', mode_kw='rand', mode_query='iid')
    
    # Other examples:
    # exp_params.set_general_params(dataset='bow-nytimes', nkw=500, nqr=500, freq='file', mode_ds='split50', mode_fs='past', mode_kw='rand', mode_query='each')
    # exp_params.set_general_params(dataset='wiki_sec', nkw=500, nqr=100_000, freq='file', mode_ds='same', mode_fs='same', mode_kw='rand', mode_query='markov')
    # exp_params.set_general_params(dataset='enron-full', nkw=200, nqr=200, ndoc=30_000, freq='file', mode_ds='splitn10000', mode_fs='past', mode_kw='rand', mode_query='iid')
    # exp_params.set_general_params(dataset='enron-full', nkw=1000, nqr=1000, ndoc=30_000, freq='none', mode_ds='splitn10000', mode_fs='past', mode_kw='rand', mode_query='each')

    ##### DEFENSE EXAMPLES
    # exp_params.set_defense_params('none')
    # exp_params.set_defense_params('pancake')
    # exp_params.set_defense_params('clrz', tpr=0.9999, fpr=0.02)
    # exp_params.set_defense_params('frost', tpr=0.95, fpr=0.2)

    ###### ATTACK LIST EXAMPLE
    # attack_list = [
    #     ('freq', {}),
    #     ('sap', {'alpha': 0.}),
    #     ('sap', {'alpha': 0.5}),
    #     ('sap', {'alpha': 1}),
    #     ('ihop', {'mode': 'Vol_freq', 'niters': 1000, 'pfree': 0.25}),
    #     ('ihop', {'mode': 'Vol', 'niters': 100, 'pfree': 0.25}),
    #     ('ihop', {'mode': 'Vol', 'niters': 1000, 'pfree': 0.25}),
    #     ('umemaya', {}),
    #     ('fastpfp', {}),
    #     ('ikk', {'cooling': 0.99}),
    #     ('graphm', {'alpha': 0.5}),
    # ]

    np.set_printoptions(precision=4)
    print(exp_params)
    acc_list = [[] for _ in attack_list]
    for seed in range(10):
        print("Seed: ", seed)

        for i_att, (att, att_p) in enumerate(attack_list):
            exp_params.set_attack_params(att, **att_p)
            exp_params.att_params['niter_list'] = niter_list
            acc, accu, time_exp = run_experiment(exp_params, seed=seed, debug_mode=True)
            if type(acc) == list:
                acc_list[i_att].append((acc[-1], accu[-1]))
                for acc, accu, niters in zip(acc, accu, exp_params.att_params['niter_list']):
                    # print("{:d}-{:d}) {:s}, acc={:.3f}, accu={:.3f} ({:.2f} secs)".format(seed, niters, att, acc, accu, time_exp))
                    print("{:d}-{:d}) {:s}, acc={:.3f} ({:.2f} secs)".format(seed, niters, att, acc, time_exp))
            else:
                acc_list[i_att].append((acc, accu))
                # print("{:d}) {:s}, acc={:.3f}, accu={:.3f} ({:.2f} secs)".format(seed, att, acc, accu, time_exp))
                print("{:d}) {:s}, acc={:.3f} ({:.2f} secs)".format(seed, att, acc, time_exp))

    print("Summary of results:")
    for i_att, (att, att_p) in enumerate(attack_list):
        # print("{:s}: avg acc={:.3f}, avg accu={:.3f}".format(att, *[np.mean(aux) for aux in zip(*acc_list[i_att])]))
        print("{:s}: avg acc={:.3f}".format(att, np.mean(list(zip(*acc_list[i_att]))[0])))
