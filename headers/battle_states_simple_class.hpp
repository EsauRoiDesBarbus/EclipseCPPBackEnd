///////////////////////////////////////
// Example of battle states subclass //
///////////////////////////////////////
#ifndef BATTLE_STATES_SIMPLE_CLASS_HPP
#define BATTLE_STATES_SIMPLE_CLASS_HPP

#include "battle_states_mother_class.hpp"


class CruiserVSAncientBattleStates : public BattleStates {
    public:
    CruiserVSAncientBattleStates (); //this constructor will return the graph of a default cruiser vs Ancient A
}; //hard coded example

class CruiserAndIntVSIntBattleStates : public BattleStates {
    public:
    CruiserAndIntVSIntBattleStates (); //this constructor will return the graph of a default cruiser vs Ancient A
}; //hard coded example

#endif