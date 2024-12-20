#include "ship_battle_states.hpp"

#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

#define DEBUG false

///////////////////
// ExtendedState //
///////////////////
string ExtendedState::toString () {
    stringstream output;
    output << "round=" << _round <<" ships=";
    int nb_ships = _ship_states.size ();
    for (int i=0; i<nb_ships; i++) {
        output << _ship_states[i];
        if (i<nb_ships-1) output << " ";
    }
    output << ".";
    return output.str ();
}

/////////////////
// DamageClock //
/////////////////
struct DamageClock { //TODO make more efficient
    vector<Damage> _damage_allocation; //0 is damage taken by ship with the most ship, 1 is damage taken by the second ship with the most shield etc...
    int _nb_enemy_ships;
    vector<Damage> _damage_rolled;

    DamageClock (RollUnallocated& roll_unallocated) {
        _damage_rolled = roll_unallocated._damages;
        _nb_enemy_ships = _damage_rolled.size ();
        _damage_allocation.resize(_nb_enemy_ships);
        //by default, allocate all damage to ship with least shield
        for (int ship=0; ship<_nb_enemy_ships-1; ship++) {
            _damage_allocation[ship] = {0, 0, 0, 0};
        }
        _damage_allocation[_nb_enemy_ships-1] = {0, 0, 0, 0}; // this last ship takes all the damage
        for (int ship=0; ship<_nb_enemy_ships; ship++) {
            for (int die=0; die<4; die++) _damage_allocation[_nb_enemy_ships-1][die]+=roll_unallocated._damages[ship][die];
        }
    }

    Damage operator[](int ship) const {
        return _damage_allocation[ship]; //damage taken by that ship
    }

    bool increment () { //returns true if finished
        // by default, allocate all damage to ship with least shield
        // each increment will move 1 damage toward a ship with more shield, without exceeding rolled damage
        bool finished = true;
        for (int ship=_nb_enemy_ships-2; ship>=0; ship--) {
            for (int die=0; die<4; die++){
                if (finished==false) continue;
                int dice_this_ship_can_take =0;
                for (int ship2=0; ship2<=ship;ship2++) dice_this_ship_can_take+=_damage_rolled[ship2][die]-_damage_allocation[ship2][die];
                if (dice_this_ship_can_take>=1){
                    //transfer 1 hit from last ship and stop
                    _damage_allocation[          ship   ][die]++;
                    _damage_allocation[_nb_enemy_ships-1][die]--;
                    finished = false;
                } else {
                    //transfer all damage back and go on
                    _damage_allocation[_nb_enemy_ships-1][die]+=_damage_allocation[          ship   ][die];
                    _damage_allocation[          ship   ][die]=0;
                }
            }
        }
        return finished;
    }

    string toString () {
        stringstream output;
        for (int ship=0; ship<_nb_enemy_ships; ship++) output << _damage_allocation[ship].toString();
        return output.str ();
    }
};

//////////////////////
// ShipBattleStates //
//////////////////////
void ShipBattleStates::initializeShipWrapperVectors () {
    // create another list of ships sorted in initiative order
    int nb_attacker_ships = _attacker_ships.size();
    int nb_defender_ships = _defender_ships.size();

    _both_ships_by_initiative.resize(nb_defender_ships+nb_attacker_ships);
    for (int ship=0; ship<nb_defender_ships; ship++) _both_ships_by_initiative[                  ship] = {_defender_ships[ship],DEFENDER, ship, 0};
    for (int ship=0; ship<nb_attacker_ships; ship++) _both_ships_by_initiative[nb_defender_ships+ship] = {_attacker_ships[ship],ATTACKER, ship, 0};

    sort(_both_ships_by_initiative.begin(), _both_ships_by_initiative.end(), [](const ShipWrapper a, const ShipWrapper b) { // sort by initiative
        return a._ship_ptr->_init > b._ship_ptr->_init;  // Compare based on shield
    });
    // save initiative order in wrapper
    for (int ship=0; ship<nb_defender_ships+nb_attacker_ships; ship++) _both_ships_by_initiative[ship]._place_in_initiative_order=ship;

    // create separate lists with ships of each player sorted by shield, to use in damage allocation later
    _attacker_ships_by_shield.resize(0);
    _defender_ships_by_shield.resize(0);

    for (int ship=0; ship<nb_attacker_ships+nb_defender_ships; ship++) {
        if (_both_ships_by_initiative[ship]._side ==ATTACKER) _attacker_ships_by_shield.push_back (_both_ships_by_initiative[ship]);
        else                                                  _defender_ships_by_shield.push_back (_both_ships_by_initiative[ship]);
    }
    // sort ships by shield
    sort(_attacker_ships_by_shield.begin(), _attacker_ships_by_shield.end(), [](const ShipWrapper a, const ShipWrapper b) {
        return a._ship_ptr->_shield > b._ship_ptr->_shield;  // Compare based on shield
    });
    sort(_defender_ships_by_shield.begin(), _defender_ships_by_shield.end(), [](const ShipWrapper a, const ShipWrapper b) {
        return a._ship_ptr->_shield > b._ship_ptr->_shield;  // Compare based on shield
    });

    // save ships with rift_weapons
    _attacker_ships_with_rift.resize(0);
    _defender_ships_with_rift.resize(0);
    for (int i=0; i<nb_attacker_ships; i++) if (_attacker_ships_by_shield[i]->hasRift()) _attacker_ships_with_rift.push_back(_attacker_ships_by_shield[i]);
    for (int i=0; i<nb_defender_ships; i++) if (_defender_ships_by_shield[i]->hasRift()) _defender_ships_with_rift.push_back(_defender_ships_by_shield[i]);

    if (DEBUG) {
        cout << "attacker shields=";
        for (int i=0; i<nb_attacker_ships; i++) cout << _attacker_ships[i]->_shield << ",";
        cout << "defender shields=";
        for (int i=0; i<nb_defender_ships; i++) cout << _defender_ships[i]->_shield << ",";
        cout << endl;
    
        cout << "Ships by initiative (place init,)=";
        for (int i=0; i<int(_both_ships_by_initiative.size()); i++) cout << _both_ships_by_initiative[i]._place_in_initiative_order <<" "<< _both_ships_by_initiative[i]->_init << ",";
        cout << endl;
    }
}

///////////////////////////////////////
// ShipBattleStates : ClockOrganizer //
///////////////////////////////////////
void ShipBattleStates::initializeClockOrganizer () {
    // the clock will be (round, state of ship1, state of ship2,...)
    int nb_ships = _both_ships_by_initiative.size ();
    vector <int> bounds(1+nb_ships), cells_per_bound(1+nb_ships, 1); // all bounds have 1 cell
    bounds[0] = 2*nb_ships-1; //canon round + missile round, bounds are inclusive so we need to remove 1
    for (int ship=0; ship<nb_ships; ship++) bounds[1+ship]=_both_ships_by_initiative[ship]->totalStates ()-1;//bounds are inclusive so we need to remove 1
    setBounds (bounds, cells_per_bound);
    if (DEBUG) cout << "total states = " << totalStates () << endl;
}

int ShipBattleStates::extendedStateToState (ExtendedState& extended_state) {
    vector<int> iteration (1, extended_state[ROUND]);
    iteration.insert (iteration.end(), extended_state._ship_states.begin(), extended_state._ship_states.end());
    return iterationToIndex (iteration);
}

ExtendedState iterationToExtendedState (vector<int> iteration) {
    ExtendedState extended_state;
    extended_state._round = iteration[0];
    extended_state._ship_states = vector<int> (iteration.begin()+1, iteration.end());
    return extended_state;
}

ExtendedState ShipBattleStates::stateToExtendedState (int state) {
    return iterationToExtendedState(indexToIteration (state));
}

ExtendedState ShipBattleStates::readExtendedState (ClockIterator& clock_iterator) {
    return iterationToExtendedState(readData (clock_iterator, ITERATION));
}


////////////////////////////////////////////
// ShipBattleStates : initializeStateInfo //
////////////////////////////////////////////
void ShipBattleStates::initializeStateInfo () {
    // initialize _who_is_firing, _states_where_attacker_wins, _states_where_defender_wins and _live_ships
    ClockIterator state_clock = createClockIterator ();
    int total_states = totalStates ();
    int nb_ships = _both_ships_by_initiative.size (); 

    // allocate memory
    _who_is_firing.resize(total_states);
    _live_ships.resize(total_states);
    _states_where_attacker_wins.resize(0);
    _states_where_defender_wins.resize(0);


    //range all states
    for (int state=0; state<total_states; state++){
        ExtendedState extended_state = readExtendedState (state_clock);
        //_live_ships
        int attacker_ships = 0;
        int defender_ships = 0;
        _live_ships[state].resize(nb_ships);
        for (int ship=0; ship<nb_ships; ship++) {
            int alive = _both_ships_by_initiative[ship]->countLiveShips (extended_state[ship]);
            _live_ships[state][ship] = alive; //TODO : maybe change form of _live_ships ?
            if (_both_ships_by_initiative[ship]._side == 1) attacker_ships+=alive;
            else                                            defender_ships+=alive;
        }

        //_states_where_attacker_wins and _states_where_defender_wins
        if      (attacker_ships==0) _states_where_defender_wins.push_back(state);
        else if (defender_ships==0) _states_where_attacker_wins.push_back(state);

        //_who_is_firing
        int round = extended_state[ROUND];
        _who_is_firing[state]=_both_ships_by_initiative[round%nb_ships]._side;
        state_clock.increment ();
    }
}

////////////////////////////////////////////
// ShipBattleStates : initializeDiceRolls //
////////////////////////////////////////////
int increaseRound (int current_round, int nb_ships) {
    // increase round number (after a ship fires, the next ship in initiative fires)
    // if the last ship is firing his canon, we go back to the start of the canon round, that is 2*nb_ships-1->nb_ships
    int next_round;
    if (current_round<2*nb_ships-1) next_round = {current_round+1};
    else                            next_round = {       nb_ships};
    return next_round;
}

Roll ShipBattleStates::allocateRoll(ExtendedState& extended_state, RollUnallocated& roll_unallocated, vector<ShipWrapper>& ships_by_shield) {
    // helper function for initializeDiceRolls
    Roll output;
    // get info
    int nb_ships = extended_state._ship_states.size();
    // copy proba
    output._proba = roll_unallocated._proba;

    //allocate damage and deduce end states (the hard part)
    DamageClock damage_clock (roll_unallocated);
    bool finished = false;
    while (finished==false) { //range all possible damage allocation of that roll
        // compute all possible extended states
        vector<vector<int>> all_possible_ships_states (nb_ships);

        // initialize with current clock values
        for (int ship=0; ship<nb_ships; ship++) all_possible_ships_states[ship]={extended_state[ship]};

        int nb_enemy_ships = ships_by_shield.size();
        for (int ship_by_shield=0; ship_by_shield<nb_enemy_ships; ship_by_shield++) {
            int ship_by_init = ships_by_shield[ship_by_shield]._place_in_initiative_order;
            // state of the ship is state_clock[ship_by_init+1], allocated damage is damage_clock[ship_by_shield]
            all_possible_ships_states[ship_by_init] = ships_by_shield[ship_by_shield]->takeHits (extended_state[ship_by_init], damage_clock[ship_by_shield]);
        }

        ExtendedState next_extended_state = extended_state;

        // increase round number
        next_extended_state[ROUND] = increaseRound(extended_state[ROUND], nb_ships);

        // transform into an array of ll possible states
        vector<int> bounds (nb_ships);
        for (int ship=0; ship<nb_ships;ship++) bounds[ship]=all_possible_ships_states[ship].size()-1;
        vector<int> cells_per_bound (nb_ships, 1);
        ClockIterator clock_iterator (bounds, cells_per_bound);
        bool finished_2 = false;
        while (finished_2==false) {
            for (int ship=0; ship<nb_ships;ship++) next_extended_state[ship] = all_possible_ships_states[ship][clock_iterator[ship]];

            int next_state = extendedStateToState (next_extended_state);
            output._allocations.push_back(next_state);

            finished_2 = clock_iterator.increment();
        }
        finished = damage_clock.increment();
    }
    // clean up allocations to remove duplicates
    sort                              (output._allocations.begin(), output._allocations.end());
    vector<int>::iterator last= unique(output._allocations.begin(), output._allocations.end());
    output._allocations.erase         (last                       , output._allocations.end());

    return output;
}

ExtendedState allocateNPCDamage (ExtendedState& extended_state, RollUnallocated& roll_unallocated, vector<ShipWrapper>& ships_by_shield) {
    //allocate damage and deduce end extended state, used to compute NPC damage as will as rift self hits
    DamageClock damage_clock (roll_unallocated);
    bool finished = false;
    unsigned long int max_score = 0;
    ExtendedState best_extended_state = extended_state;

    while (finished==false) { //range all possible damage allocation of that roll
        // compute all possible extended states
        ExtendedState next_extended_state = extended_state;
        
        unsigned long int score = 0;

        int nb_enemy_ships = ships_by_shield.size();
        for (int ship_by_shield=0; ship_by_shield<nb_enemy_ships; ship_by_shield++) {
            int ship_by_init = ships_by_shield[ship_by_shield]._place_in_initiative_order;
            // state of the ship is state_clock[ship_by_init+1], allocated damage is damage_clock[ship_by_shield]
            StateNPCWrapper state_and_score = ships_by_shield[ship_by_shield]->takeNPCHits (extended_state[ship_by_init], damage_clock[ship_by_shield]);

            next_extended_state[ship_by_init] = state_and_score._state;
            score += state_and_score._npc_score;
        }
        if (score >= max_score){
            // this state is a better fit for NPC
            best_extended_state = next_extended_state;
            
            max_score = score;
        }
        finished = damage_clock.increment();
    }
    return best_extended_state;
}

Roll ShipBattleStates::allocateNPCRoll(ExtendedState& extended_state, RollUnallocated& roll_unallocated, vector<ShipWrapper>& ships_by_shield) {
    // follows NPC hit allocation rules using a score system
    Roll output;
    // get info
    int nb_ships = extended_state._ship_states.size();
    // copy proba
    output._proba = roll_unallocated._proba;

    ExtendedState next_extended_state = allocateNPCDamage (extended_state, roll_unallocated, ships_by_shield);

    // increase round number
    next_extended_state[ROUND] = increaseRound(next_extended_state[ROUND], nb_ships);
    //convert to state
    int next_state = extendedStateToState (next_extended_state);
    //there is only one possible state when NPC fire
    output._allocations.push_back(next_state);
    return output;
}

void ShipBattleStates::initializeDiceRolls () {
    // initialize _dice_rolls and _state_bundles
    ClockIterator state_clock = createClockIterator ();
    int total_states = totalStates ();
    int nb_ships = _both_ships_by_initiative.size (); 

    // allocate memory
    _state_bundles.resize(0);
    _dice_rolls.resize (total_states);

    // create shield arrays
    int nb_attacker_ships = _attacker_ships_by_shield.size();
    int nb_defender_ships = _defender_ships_by_shield.size();
    vector<int> attacker_shields(nb_attacker_ships);
    vector<int> defender_shields(nb_defender_ships);
    for (int ship=0; ship<nb_attacker_ships;ship++) attacker_shields[ship] = _attacker_ships_by_shield[ship]->_shield;
    for (int ship=0; ship<nb_defender_ships;ship++) defender_shields[ship] = _defender_ships_by_shield[ship]->_shield;

    // create big table of rolls (to avoid recomputing it at each step)
    vector< //for each ship type
        vector< //for each number of ships that are alive
            vector<RollUnallocated>  //we have one list of rolls
        >
    > big_table_of_rolls(2*nb_ships); //0-nb_ships-1 canon rounds, nb_ships-2*nb_ships-1 missile rounds

    for (int ship=0; ship<nb_ships; ship++) {
        int max_ships = _both_ships_by_initiative[ship]->_number;
        big_table_of_rolls[         ship].resize (max_ships+1); //first cell is when 0 ships are alive
        big_table_of_rolls[nb_ships+ship].resize (max_ships+1); //first cell is when 0 ships are alive
        int player = _both_ships_by_initiative[ship]._side; //1 if attacker, -1 if defender
        for (int alive=0; alive<=max_ships; alive++) {
            if (player==ATTACKER) big_table_of_rolls[         ship][alive] = _both_ships_by_initiative[ship]->listRolls (alive, MISSILES, defender_shields);
            else                  big_table_of_rolls[         ship][alive] = _both_ships_by_initiative[ship]->listRolls (alive, MISSILES, attacker_shields);
        }
        for (int alive=0; alive<=max_ships; alive++) {
            if (player==ATTACKER) big_table_of_rolls[nb_ships+ship][alive] = _both_ships_by_initiative[ship]->listRolls (alive,   CANONS, defender_shields);
            else                  big_table_of_rolls[nb_ships+ship][alive] = _both_ships_by_initiative[ship]->listRolls (alive,   CANONS, attacker_shields);
        }
    }

    //range all states
    int nb_attacker_wins = _states_where_attacker_wins.size();
    int nb_defender_wins = _states_where_defender_wins.size();
    int attacker_win_it =0;
    int defender_win_it =0;
    for (int state=0; state<total_states; state++){
        // check if victory so we avoid unnecessary computations
        if        ((attacker_win_it<nb_attacker_wins)&&(state==_states_where_attacker_wins[attacker_win_it])) {
            attacker_win_it++;
        } else if ((defender_win_it<nb_defender_wins)&&(state==_states_where_defender_wins[defender_win_it])) {
            defender_win_it++;
        } else {
            ExtendedState extended_state = readExtendedState (state_clock);
            int round = extended_state[ROUND];
            int player = _both_ships_by_initiative[round%nb_ships]._side; //1 if attacker, -1 if defender
            int alive  = _both_ships_by_initiative[round%nb_ships]->countLiveShips (extended_state[round%nb_ships]);

            vector<RollUnallocated> rolls_unallocated = big_table_of_rolls[round][alive];
            int nb_rolls = rolls_unallocated.size ();
            _dice_rolls[state].resize(nb_rolls); //output

            //compute the step we will reach if the ship deals no damage (as it needs to be removed when further states can be reached)
            ExtendedState no_damage_extended_state = extended_state;
            no_damage_extended_state[ROUND] = increaseRound (no_damage_extended_state[ROUND], nb_ships);
            int no_damage_state = extendedStateToState (no_damage_extended_state); //state that will be reached if no damage is dealt

            // range all rolls
            for (int roll=0; roll<nb_rolls; roll++) { //range all possible roll
                // apply rift self hits if there are any
                // this needs to be done first as it may affect player roll allocation
                if (rolls_unallocated[roll]._self_hits>0) {
                    vector<ShipWrapper> ships_with_rift;
                    if (player==ATTACKER) ships_with_rift = _attacker_ships_with_rift;
                    else                  ships_with_rift = _defender_ships_with_rift;

                    RollUnallocated rift_self_hits = rolls_unallocated[roll].selfHitsToRollUnallocated(ships_with_rift.size());
                    extended_state = allocateNPCDamage(extended_state, rift_self_hits, ships_with_rift);
                } 

                // allocate damage of roll
                vector<ShipWrapper> ships_by_shield; //attacker or defender depending on ship side
                BattleModifiers bonus;
                if (player==ATTACKER) {ships_by_shield = _defender_ships_by_shield; bonus = _attacker_bonus;}
                else                  {ships_by_shield = _attacker_ships_by_shield; bonus = _defender_bonus;}

                // find all possible allocations of damage of that roll
                if (bonus._is_npc) _dice_rolls[state][roll] = allocateNPCRoll(extended_state, rolls_unallocated[roll], ships_by_shield);
                else               _dice_rolls[state][roll] = allocateRoll   (extended_state, rolls_unallocated[roll], ships_by_shield);

                // if there are multiple elements, that means we can reach multiple states, hence remove the no damage state (which will be first in the list)
                if ((_dice_rolls[state][roll]._allocations.size()>=2)and(_dice_rolls[state][roll]._allocations[0]==no_damage_state))
                    _dice_rolls[state][roll]._allocations.erase (_dice_rolls[state][roll]._allocations.begin());

            }
            // save in bundle list if needed
            if (no_damage_state<state) _state_bundles.push_back ({no_damage_state, state}); //TODO better bundle method for regen
        }
        state_clock.increment ();
    }
    if (DEBUG){
        cout << "_state_bundles=";
        for (int i=0; i<int(_state_bundles.size ()); i++) cout << "("<<get<0>(_state_bundles[i]) << "," << get<1>(_state_bundles[i]) <<")";
        cout <<endl;
    }
}

////////////////////////////////////
// ShipBattleStates : constructor //
////////////////////////////////////
ShipBattleStates::ShipBattleStates (std::vector<shared_ptr<Ship>> att_ships, BattleModifiers att, std::vector<shared_ptr<Ship>> def_ships, BattleModifiers def): 
    _attacker_ships(att_ships), _defender_ships(def_ships), _attacker_bonus(att), _defender_bonus(def) {
    initializeShipWrapperVectors ();
    initializeClockOrganizer ();
    initializeStateInfo ();
    initializeDiceRolls ();

}
