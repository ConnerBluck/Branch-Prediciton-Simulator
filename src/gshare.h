#ifndef GSHARE_H
#define GSHARE_H

#include "branchsim.h"
#include <vector>
using std::vector;

// Gshare predictor definition
class gshare : public branch_predictor_base
{

private:
uint64_t GHR = 0;
uint64_t G;
uint64_t N;
uint64_t idx;
uint64_t tag;
struct pattern_table {
    uint64_t SC;
    uint64_t tag;
};
vector <pattern_table> patt;
    

public:
    void init_predictor(branchsim_conf *sim_conf);
    
    // Return the prediction ({taken/not-taken}, target-address)
    bool predict(branch *branch, branchsim_stats *sim_stats);
    
    // Update the branch predictor state
    void update_predictor(branch *branch);

    // Cleanup any initialized memory here
    ~gshare();
};

#endif
