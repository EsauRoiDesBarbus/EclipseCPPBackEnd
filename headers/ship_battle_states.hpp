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

    // state correspondance
    std::vector<int> _state_conversion_vector;
    void initializeStateVector ();
    int toState (std::vector<int>); //transforms initiative + ship state array into a single state index by dot product

    void initializeStateBundles ();
    void initializeStatesWhereWins ();
    void initializeWhoIsFiring ();
    void initializeLiveShips ();

    // compute graph edges 
    void initializeDiceRolls (); 

    // sort ships by shield 
    // compute initiative order
    // build index <-> initiative + ship state correspondance
    // transform RollUnallocated into Roll and save them in _dice_rolls

    void initialize (); //does all previous states in order

    // constructor
    ShipBattleStates (std::vector<Ship*>, BattleModifiers, std::vector<Ship*>, BattleModifiers);
};




#endif