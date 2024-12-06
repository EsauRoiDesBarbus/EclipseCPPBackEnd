//////////////////////////////////////////////////////////////////////////
// Abstract class to represent eclipse battles as stochastic automatons //
//////////////////////////////////////////////////////////////////////////
#ifndef BATTLE_STATES_MOTHER_CLASS_HPP
#define BATTLE_STATES_MOTHER_CLASS_HPP

#include <vector>
#include <tuple>
#include <string>

class BattleStates {
    public: //everything is public because it's a boardgame calculator, who cares about security?

    // all possible states in the battle are ordered in a vector,
    // with the first cell corresponding to the initial state of the battle,
    // and subsequent cells corresponding to other state of the battle (not that there are multiple way to order those states)
    std::vector<int> _who_is_firing; // for each state, either the attacker is firing (1) or the defender is firing (-1)

    // states can loop on themselves and need to be computed in bundle
    // BATTLE STATES MUST BE ORDERED SO THAT SUCH STATES FORM A CONTIGUOUS BLOCK
    // Each couple in the following vector is the first and last element of such a bundle 
    std::vector<std::tuple<int,int>> _state_bundles; 

    // some states correspond to one side winning
    std::vector<int> _states_where_attacker_wins; // list of state index corresponding to a win of the attacker
    std::vector<int> _states_where_defender_wins; // same for defender

    std::vector<std::vector<int>> _live_ships; // for each state, lists how many ships of each type are alive

    // graph edges (the tricky part)
    std::vector< //for each state, 
        std::vector< //we roll dice and for each possible result,
            std::tuple<float, std::vector<int>> //we have a probability of the result, plus a list of states this roll can lead to
        >
    > _dice_rolls;

    // no constructor, we initialize all vectors empty, it's the subclasses that will fill them

    // utility
    std::string toString (); //returns

};



#endif