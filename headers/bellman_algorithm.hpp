/////////////////////////////////
// Methods to solve the battle //
/////////////////////////////////
#ifndef BELLMAN_ALGORITHM_HPP
#define BELLMAN_ALGORITHM_HPP

#include "battle_states_mother_class.hpp"

#include <string>

//container for battle results
class BattleResult {
    public: 
    float _attacker_win_chance;
    std::vector<std::vector<float>> _attacker_ship_survival_chance; //for the graph
    std::vector<std::vector<float>> _defender_ship_survival_chance; //for the graph

    std::string toString ();
};

//BackwardForward Bellman algorithm to compute win chance and expectancy of each state, then return the result of the battle
BattleResult winChanceAndExpectancyCalculator (BattleStates&);

int findBestAllocation (int sign, std::vector<int>& allocations, std::vector<float>& win_chance);

std::vector<float> solveLP(std::vector<std::vector<float>>& A, std::vector<float>& b);

#endif