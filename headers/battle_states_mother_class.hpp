//////////////////////////////////////////////////////////////////////////
// Abstract class to represent eclipse battles as stochastic automatons //
//////////////////////////////////////////////////////////////////////////
#ifndef BATTLE_STATES_MOTHER_CLASS_HPP
#define BATTLE_STATES_MOTHER_CLASS_HPP

#include <vector>
#include <tuple>

class BattleStates {
    public: //everything is public because it's a boardgame calculator, who cares about security?

    // all possible states in the battle are ordered in a vector,
    // with the first cell corresponding to the initial state of the battle,
    // and subsequent cells corresponding to other state of the battle (not that there are multiple way to order those states)

    // some states correspond to one side winning
    std::vector<int> _states_where_attacker_wins; // list of state index corresponding to a win of the attacker
    std::vector<int> _states_where_defender_wins; // same for defender

    // graph edges (the tricky part)
    std::vector< //for each state, 
        std::vector< //we roll dice and for each possible result,
            std::tuple<float, std::vector<int>> //we have a probability of the result, plus a list of states this roll can lead to
            >
        > _dice_rolls; // _graph_edges [i] is the vector containing all the vertices vertex i can lead to

    // no constructor, we initialize all vectors empty, it's the subclasses that will fill them

};



#endif