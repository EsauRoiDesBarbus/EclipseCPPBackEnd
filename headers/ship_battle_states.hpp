////////////////////////////////////////////////////////////////////
// Main class to convert eclipse battles to stochastic automatons //
////////////////////////////////////////////////////////////////////
#ifndef SHIP_BATTLE_STATES_HPP
#define SHIP_BATTLE_STATES_HPP

#include "battle_states_mother_class.hpp"
#include "ship.hpp"

#include <vector>

struct BattleModifiers {
    bool _is_npc; //to know if that side follows npc allocation rule
    bool _antimatter_splitter; //true if that side has this tech
};

class ShipBattleStates: public BattleStates {
    public:
    // battle info
    std::vector<Ship*> _attacker_ships;
    std::vector<Ship*> _defender_ships;

    BattleModifiers _attacker_bonus; //TODO
    BattleModifiers _defender_bonus; //TODO


    // pre-treatment
    std::vector<Ship*> _ships_by_initiative;
    void initialSort (); //sort Ships by shield and initiative

    // state correspondance between state (index in battle state class) and extended class (initiative + ship state array)
    std::vector<int> StateToExtendedState (int);
    int ExtendedStateToState (std::vector<int>);

    // all state info :_whos_is_firing, _state_bundles, _states_where_attacker_wins, _states_where_defender_wins, _live_ships
    void initializeStateInfo ();

    // compute graph edges 
    void initializeDiceRolls (); 


    void initialize (); //does all previous states in order

    // constructor
    ShipBattleStates (std::vector<Ship*>, BattleModifiers, std::vector<Ship*>, BattleModifiers);
};




#endif