////////////////////////////////////////////////////////////////////
// Main class to convert eclipse battles to stochastic automatons //
////////////////////////////////////////////////////////////////////
#ifndef SHIP_BATTLE_STATES_HPP
#define SHIP_BATTLE_STATES_HPP

#include "battle_states_mother_class.hpp"
#include "ship.hpp"
#include "clock_organizer.hpp"

#include <vector>
#include <memory>
#include <string>
#include <sstream>

#define ATTACKER  1
#define DEFENDER -1

#define ROUND -1 //for extended state

struct BattleModifiers {
    bool _is_npc; //to know if that side follows npc allocation rule
    bool _antimatter_splitter; //true if that side has this tech
};

struct ShipWrapper { //class to add info relative to other ships
    std::shared_ptr<Ship> _ship_ptr;
    int _side; // 1 if attacker, -1 if defender
    int _place_first_vector; // where it is in _attacker_ships or _defender_ship
    int _place_in_initiative_order; //0 has the most initiative, 1 the second most etc
    std::shared_ptr<Ship> operator-> () {return _ship_ptr;}
    //ShipWrapper (std::shared_ptr<Ship> a, int b, int c): _ship_ptr(a), _side(b), _place_first_vector(c) {}
};

struct ExtendedState {
    int _round;
    std::vector<int> _ship_states;

    int& operator[](int i) {if (i==ROUND) return _round; else return _ship_states[i];}

    std::string toString (); 
};

class ShipBattleStates: public BattleStates, public ClockOrganizer {
    public:
    // battle info
    std::vector<std::shared_ptr<Ship>> _attacker_ships;
    std::vector<std::shared_ptr<Ship>> _defender_ships;

    BattleModifiers _attacker_bonus; //TODO
    BattleModifiers _defender_bonus; //TODO


    // pre-treatment
    std::vector<ShipWrapper> _both_ships_by_initiative;
    std::vector<ShipWrapper> _attacker_ships_by_shield;
    std::vector<ShipWrapper> _defender_ships_by_shield;
    std::vector<ShipWrapper> _attacker_ships_with_rift;
    std::vector<ShipWrapper> _defender_ships_with_rift;
    void initializeShipWrapperVectors (); //sort Ships by shield and initiative

    // state correspondance between state (index in battle state class) and extended class (initiative + ship state array)
    void initializeClockOrganizer ();
    ExtendedState stateToExtendedState (int );
    int extendedStateToState (ExtendedState&);

    ExtendedState readExtendedState (ClockIterator&);

    // all state info :_whos_is_firing, _state_bundles, _states_where_attacker_wins, _states_where_defender_wins, _live_ships
    void initializeStateInfo ();

    // compute graph edges 
    void initializeDiceRolls ();
    Roll allocateRoll   (ExtendedState&, RollUnallocated&, std::vector<ShipWrapper>&);
    Roll allocateNPCRoll(ExtendedState&, RollUnallocated&, std::vector<ShipWrapper>&);
    

    void initialize (); //does all previous states in order

    // constructor
    ShipBattleStates (std::vector<std::shared_ptr<Ship>>, BattleModifiers, std::vector<std::shared_ptr<Ship>>, BattleModifiers);
};




#endif