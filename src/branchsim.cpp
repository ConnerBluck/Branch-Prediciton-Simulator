
#include <iostream>
#include <vector>

#include "branchsim.h"
#include "gshare.h"
#include "perceptron.h"
#include "yeh_patt.h"


// Don't modify this line -- Its to make the compiler happy
branch_predictor_base::~branch_predictor_base() {}


// ******* Student Code starts here *******

// Gshare Branch Predicto

void gshare::init_predictor(branchsim_conf *sim_conf)
{
    //size of pattern table
    uint64_t pat_table_size = (1 << sim_conf->G);

    //set size of pattern table vector
    patt.resize(pat_table_size);

    //init all smith counters to 1 and associated tag to -1
    for (uint64_t i = 0; i < pat_table_size; i++) {
        patt[i].SC = 1;
        patt[i].tag = (uint64_t) -1;
    }

    //put config values into variables to use elsewhere
    G = sim_conf->G;
    N = sim_conf->N;

}

bool gshare::predict(branch *branch, branchsim_stats *sim_stats)
{
    sim_stats->num_branch_instructions++;
    uint64_t mask = 0xFFFFFFFFFFFFFFFF; //mask to calculate index from PC
    tag = (branch->ip >> (G + 2));
    idx = (branch->ip >> 2);
    mask = mask >> G;
    mask = mask << G;
    mask = ~(mask);
    idx = (idx & mask);
    idx = idx ^ GHR;

    //set branch miss prediction stalls
    if (N <= 7) {
        sim_stats->stalls_per_miss_predicted_branch = 2;
    } else {
        sim_stats->stalls_per_miss_predicted_branch = ((N/2) - 1);
    }

    //check pattern table tag
    if (patt[idx].tag != tag) {
        sim_stats->num_tag_conflicts++;
    }

    //if smithcounter is <= 1, return not taken
    if (patt[idx].SC <= 1) {
        if (branch->is_taken == 0) {
            sim_stats->num_branches_correctly_predicted++;
        } else {
            sim_stats->num_branches_miss_predicted++;
        }
        return NOT_TAKEN;
    //if smithcounter is 2 or 3, return taken
    } else {
        if (branch->is_taken == 1) {
            sim_stats->num_branches_correctly_predicted++;
        } else {
            sim_stats->num_branches_miss_predicted++;
        }
        return TAKEN;
    }
}

void gshare::update_predictor(branch *branch)
{
    uint64_t mask = 0xFFFFFFFFFFFFFFFF; //mask to set correct bits in GHR
    mask = (mask >> G);
    mask = (mask << G);
    mask = ~(mask);

    //set tag for current index in pattern table
    patt[idx].tag = tag;
    //if branch is not taken, update smithercounter and GHR
    if (branch->is_taken == 0) {
        if (patt[idx].SC > 0) {
            patt[idx].SC--;
        }
        GHR = GHR << 1;
        GHR = (GHR & mask);
    //if branch is taken, update smithercounter and GHR
    } else {
        if (patt[idx].SC < 3) {
            patt[idx].SC++;
        }
        GHR = (GHR << 1);
        GHR = (GHR | 1);
        GHR = (GHR & mask);
    }

}

gshare::~gshare()
{
    patt.clear();
}


// Yeh-Patt Branch Predictor

void yeh_patt::init_predictor(branchsim_conf *sim_conf)
{   
    //size of history table and pattern table
    uint64_t hist_table_size = (1 << sim_conf->G);
    uint64_t pat_table_size = (1 << sim_conf->P);

    //set size of history table and pattern table vectors
    hist.resize(hist_table_size);
    pat_table.resize(pat_table_size);

    //init all entries in pattern table to 1
    for (uint64_t i = 0; i < pat_table_size; i++) {
        pat_table[i] = 1;
    }
    //init all shift regs in history table to 0 and all tags to -1
    for (uint64_t i = 0; i < hist_table_size; i++) {
        hist[i].shift_reg = 0;
        hist[i].tag = (uint64_t) -1;
    }

    //put config values into variables to use elsewhere
    G = sim_conf->G;
    P = sim_conf->P;
    N = sim_conf->N;

}

bool yeh_patt::predict(branch *branch, branchsim_stats *sim_stats)
{   
    sim_stats->num_branch_instructions++;
    uint64_t mask = 0xFFFFFFFFFFFFFFFF; //mask to calculate index from PC
    tag = (branch->ip >> (G + 2));
    idx = (branch->ip >> 2);
    mask = mask >> G;
    mask = mask << G;
    mask = ~(mask);
    idx = (idx & mask);

    //set branch miss prediction stalls
    if (N <= 7) {
        sim_stats->stalls_per_miss_predicted_branch = 2;
    } else {
        sim_stats->stalls_per_miss_predicted_branch = ((N/2) - 1);
    }

    //check history table tag
    if (hist[idx].tag != tag) {
        sim_stats->num_tag_conflicts++;
    }

    //if smithcounter is <= 1, return not taken
    if (pat_table[hist[idx].shift_reg] <= 1) {
        if (branch->is_taken == 0) {
            sim_stats->num_branches_correctly_predicted++;
        } else {
            sim_stats->num_branches_miss_predicted++;
        }
        return NOT_TAKEN;
    //if smithcounter is 2 or 3, return taken
    } else {
        if (branch->is_taken == 1) {
            sim_stats->num_branches_correctly_predicted++;
        } else {
            sim_stats->num_branches_miss_predicted++;
        }
        return TAKEN;
    }
}

void yeh_patt::update_predictor(branch *branch)
{
    uint64_t mask = 0xFFFFFFFFFFFFFFFF; //mask to set correct bits in history table
    mask = (mask >> P);
    mask = (mask << P);
    mask = ~(mask);

    //set tag for current index in history table
    hist[idx].tag = tag;
    //if branch is not taken, update smithercounter and history table
    if (branch->is_taken == 0) {
        if (pat_table[hist[idx].shift_reg] > 0) {
            pat_table[hist[idx].shift_reg]--;
        }
        hist[idx].shift_reg = hist[idx].shift_reg << 1;
        hist[idx].shift_reg = (hist[idx].shift_reg & mask);
    //if branch is taken, update smithercounter and history table
    } else {
        if (pat_table[hist[idx].shift_reg] < 3) {
            pat_table[hist[idx].shift_reg]++;
        }
        hist[idx].shift_reg = (hist[idx].shift_reg << 1);
        hist[idx].shift_reg = (hist[idx].shift_reg | 1);
        hist[idx].shift_reg = (hist[idx].shift_reg & mask);
    }

}

yeh_patt::~yeh_patt()
{
    hist.clear();
    pat_table.clear();

}


// Perceptron Branch Predictor

void perceptron_predictor::init_predictor(branchsim_conf *sim_conf)
{

}

bool perceptron_predictor::predict(branch *branch, branchsim_stats *sim_stats)
{
    return NOT_TAKEN;
}

void perceptron_predictor::update_predictor(branch *branch)
{

}

perceptron_predictor::~perceptron_predictor()
{

}


// Common Functions to update statistics and final computations, etc.

/**
 *  Function to update the branchsim stats per prediction
 *
 *  @param perdiction The prediction returned from the predictor's predict function
 *  @param branch Pointer to the branch that is being predicted -- contains actual behavior
 *  @param stats Pointer to the simulation statistics -- update in this function
*/
void branchsim_update_stats(bool prediction, branch *branch, branchsim_stats *sim_stats)
{
    sim_stats->total_instructions = branch->inst_num;   

}

/**
 *  Function to cleanup branchsim statistic computations such as prediction rate, etc.
 *
 *  @param stats Pointer to the simulation statistics -- update in this function
*/
void branchsim_cleanup_stats(branchsim_stats *sim_stats)
{
    sim_stats->fraction_branch_instructions = ((double) sim_stats->num_branch_instructions / (double) sim_stats->total_instructions);
    sim_stats->prediction_accuracy = ((double) sim_stats->num_branches_correctly_predicted / (double) sim_stats->num_branch_instructions);

    sim_stats->misses_per_kilo_instructions = (sim_stats->num_branches_miss_predicted / (sim_stats->total_instructions / 1000));
    sim_stats->average_CPI = 1 + ((1 - sim_stats->prediction_accuracy) * sim_stats->fraction_branch_instructions * sim_stats->stalls_per_miss_predicted_branch);

}
