#ifndef YEH_PATT_H
#define YEH_PATT_H

#include "branchsim.h"
using std::vector;

// Yeh-Patt predictor definition
class yeh_patt : public branch_predictor_base
{
private:
struct history_table {
    uint64_t shift_reg;
    uint64_t tag;
};
uint64_t G;
uint64_t P;
uint64_t N;
uint64_t idx;
uint64_t tag;
vector <uint64_t> pat_table;
vector <history_table> hist;

public:
    void init_predictor(branchsim_conf *sim_conf);
    
    // Return the prediction ({taken/not-taken}, target-address)
    bool predict(branch *branch, branchsim_stats *sim_stats);
    
    // Update the branch predictor state
    void update_predictor(branch *branch);

    // Cleanup any initialized memory here
    ~yeh_patt();
};

#endif
