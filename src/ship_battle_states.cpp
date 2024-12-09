#include "ship_battle_states.hpp"

#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

#define DEBUG false



struct StateClock {
    // structure used to iterate between all states of the battle
    int _state;
    vector <int> _extended_state;
    vector <int> _bounds_of_rows;
    int _nb_ships;

    StateClock (vector<ShipWrapper>& ships) {
        _state=0;
        _nb_ships = ships.size();
        _extended_state.resize (1+_nb_ships);
        _bounds_of_rows.resize (1+_nb_ships);
        _extended_state[0]=0;
        _bounds_of_rows[0]=2*_nb_ships; // canon + missile rounds
        for (int ship=0; ship<_nb_ships; ship++) {
            _extended_state[1+ship]=0;
            _bounds_of_rows[1+ship]=ships[ship]->totalStates();
        }
    }

    int totalStates () {
        int total = 1;
        for (int i=0; i <1+_nb_ships; i++) total*=_bounds_of_rows[i];
        return total;
    }

    int operator[](int i) const {
        return _extended_state[i];
    }

    void increment () {
        _state++;
        for (int i=0; i <1+_nb_ships; i++) {
            if (_extended_state[i] < _bounds_of_rows[i]-1) {
                // increase i th value and exit
                _extended_state[i]++;
                break;
            } else {
                // reinititialize i th value and continue
                _extended_state[i]=0;
            }
        }
    }

    int extendedStateToState (vector<int>& extended_state) {
        int state = 0;
        int nb_ship_types_plus_one = extended_state.size();
        for (int i = nb_ship_types_plus_one-1; i>=0; i--) {
            state*= _bounds_of_rows[i];
            state+= extended_state[i];
        }
        return state;
    }

    string toString () {
        stringstream output;
        output << "state="<<_state<<", extended state=(";
        for (int i=0; i<_nb_ships+1; i++) output << _extended_state[i]<< " ";
        output << ")";
        return output.str ();
    }
    int size () {return int(_extended_state.size());}
};

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


void ShipBattleStates::initialSort () {
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

    if (DEBUG) {
        cout << "attacker shields=";
        for (int i=0; i<int(_attacker_ships.size()); i++) cout << _attacker_ships[i]->_shield << ",";
        cout << "defender shields=";
        for (int i=0; i<int(_defender_ships.size()); i++) cout << _defender_ships[i]->_shield << ",";
        cout << endl;
    }

    // create another list of ships sorted in initiative order
    _both_ships_by_initiative = _defender_ships_by_shield;
    _both_ships_by_initiative.insert(_both_ships_by_initiative.end(), _attacker_ships_by_shield.begin(), _attacker_ships_by_shield.end());
    sort(_both_ships_by_initiative.begin(), _both_ships_by_initiative.end(), [](const ShipWrapper a, const ShipWrapper b) {
        return a._ship_ptr->_init > b._ship_ptr->_init;  // Compare based on shield
    });

    if (DEBUG) {
        cout << "Ships by initiative (place init,)=";
        for (int i=0; i<int(_both_ships_by_initiative.size()); i++) cout << _both_ships_by_initiative[i]._place_in_initiative_order <<" "<< _both_ships_by_initiative[i]->_init << ",";
        cout << endl;
    }
}

int ShipBattleStates::extendedStateToState (vector<int> extended_state) {
    int state = 0;
    int nb_ship_types = _both_ships_by_initiative.size();
    for (int ship = nb_ship_types-1; ship>=0; ship--) {
        state*= _both_ships_by_initiative[ship]->totalStates ();
        state+= extended_state[1+ship];
    }
    state*= 2*_both_ships_by_initiative.size();
    state+= extended_state [0];
    return state;
}

vector<int> ShipBattleStates::stateToExtendedState (int state) {
    int nb_ship_types = _both_ships_by_initiative.size();
    vector<int> extended_state (1+nb_ship_types);
    int nb_rounds = 2*nb_ship_types;
    extended_state[0] = state%nb_rounds;
    state/=nb_rounds;
    for (int ship = 0; ship<nb_ship_types; ship++) {
        int total_states = _both_ships_by_initiative[ship]->totalStates ();
        extended_state[1+ship] = state%total_states;
        state/= total_states;
    }
    return extended_state;
}

void ShipBattleStates::initializeStateInfo () {
    // initialize _who_is_firing, _states_where_attacker_wins, _states_where_defender_wins and _live_ships
    StateClock state_clock (_both_ships_by_initiative);
    int total_states = state_clock.totalStates ();
    int nb_ships = _both_ships_by_initiative.size (); 

    // allocate memory
    _who_is_firing.resize(total_states);
    _live_ships.resize(total_states);
    _states_where_attacker_wins.resize(0);
    _states_where_defender_wins.resize(0);


    //range all states
    for (int state=0; state<total_states; state++){
        //_live_ships
        int attacker_ships = 0;
        int defender_ships = 0;
        _live_ships[state].resize(nb_ships);
        for (int ship=0; ship<nb_ships; ship++) {
            int alive = _both_ships_by_initiative[ship]->countLiveShips (state_clock[1+ship]);
            _live_ships[state][ship] = alive; //TODO : maybe change form of _live_ships ?
            if (_both_ships_by_initiative[ship]._side == 1) attacker_ships+=alive;
            else                                            defender_ships+=alive;
        }

        //_states_where_attacker_wins and _states_where_defender_wins
        if      (attacker_ships==0) _states_where_defender_wins.push_back(state);
        else if (defender_ships==0) _states_where_attacker_wins.push_back(state);

        //_who_is_firing
        int round = state_clock[0];
        _who_is_firing[state]=_both_ships_by_initiative[round%nb_ships]._side;

        if (DEBUG) { //check if state clock and state conversion function work
            cout << state_clock.toString ();
            cout << " conversion test " << extendedStateToState (state_clock._extended_state) << "<->(";
            vector<int> extended_state = stateToExtendedState (state_clock._state);
            for (int i=0; i<int(extended_state.size()); i++) cout<<extended_state[i]<<" ";
            cout << ")\n";
        }

        state_clock.increment ();
    }
}

int increaseRound (int current_round, int nb_ships) {
    int next_round;
    if (current_round<2*nb_ships-1) next_round = {current_round+1};
    else                            next_round = {       nb_ships};
    return next_round;
}

Roll allocateRoll(StateClock& state_clock, RollUnallocated& roll_unallocated, vector<ShipWrapper>& ships_by_shield) {
    // helper function for initializeDiceRolls
    Roll output;
    // get info
    int nb_ships = state_clock.size()-1;
    // copy proba
    output._proba = roll_unallocated._proba;

    //allocate damage and deduce end states (the hard part)
    DamageClock damage_clock (roll_unallocated);
    bool finished = false;
    while (finished==false) { //range all possible damage allocation of that roll
        // compute all possible extended states
        vector<vector<int>> all_extended_states (1+nb_ships);

        // initialize with current clock values
        for (int i=0; i<1+nb_ships; i++) all_extended_states[i]={state_clock[i]};

        int nb_enemy_ships = ships_by_shield.size();
        for (int ship_by_shield=0; ship_by_shield<nb_enemy_ships; ship_by_shield++) {
            int ship_by_init = ships_by_shield[ship_by_shield]._place_in_initiative_order;
            // state of the ship is state_clock[ship_by_init+1], allocated damage is damage_clock[ship_by_shield]
            all_extended_states[1+ship_by_init] = ships_by_shield[ship_by_shield]->takeHits (state_clock[ship_by_init+1], damage_clock[ship_by_shield]);
        }

        // increase round number (after a ship fires, the next ship in initiative fires)
        // if the last ship is firing his canon, we go back to the start of the canon round, that is 2*nb_ships-1->nb_ships
        all_extended_states[0] = {increaseRound(state_clock[0], nb_ships)};

        // transform into an array of states TODO range all possibilities, we only do one here
        vector<int> extended_state (1+nb_ships);
        for (int i=0; i<1+nb_ships;i++) extended_state[i] = all_extended_states[i][0];

        int next_state = state_clock.extendedStateToState (extended_state);


        output._allocations.push_back(next_state);
        finished = damage_clock.increment();
    }
    // clean up allocations to remove duplicates
    sort                              (output._allocations.begin(), output._allocations.end());
    vector<int>::iterator last= unique(output._allocations.begin(), output._allocations.end());
    output._allocations.erase         (last                       , output._allocations.end());

    return output;
}

void ShipBattleStates::initializeDiceRolls () {
    // initialize _dice_rolls and _state_bundles
    StateClock state_clock (_both_ships_by_initiative);
    int total_states = state_clock.totalStates ();
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
            int round = state_clock[0];
            int player = _both_ships_by_initiative[round%nb_ships]._side; //1 if attacker, -1 if defender
            int alive  = _both_ships_by_initiative[round%nb_ships]->countLiveShips (state_clock[1+(round%nb_ships)]);

            vector<RollUnallocated> rolls_unallocated = big_table_of_rolls[round][alive];
            int nb_rolls = rolls_unallocated.size ();
            _dice_rolls[state].resize(nb_rolls); //output

            //compute the step we will reach if the ship deals no damage (as it needs to be removed when further states can be reached)
            vector<int> no_damage_extended_state = state_clock._extended_state;
            no_damage_extended_state[0] = increaseRound (no_damage_extended_state[0], nb_ships);
            int no_damage_state = extendedStateToState (no_damage_extended_state); //state that will be reached if no damage is dealt

            // range all rolls
            for (int roll=0; roll<nb_rolls; roll++) { //range all possible roll

                vector<ShipWrapper> ships_by_shield; //attacker or defender depending on ship side
                if (player==ATTACKER) ships_by_shield = _defender_ships_by_shield;
                else                  ships_by_shield = _attacker_ships_by_shield;

                // find all possible allocations of damage of that roll
                _dice_rolls[state][roll] = allocateRoll(state_clock, rolls_unallocated[roll], ships_by_shield);

                // if there are multiple elements, that means we can reach multiple states, hence remove the no damage state (which will be first in the list)
                if ((_dice_rolls[state][roll]._allocations.size()>=2)and(_dice_rolls[state][roll]._allocations[0]=no_damage_state))
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

ShipBattleStates::ShipBattleStates (std::vector<shared_ptr<Ship>> att_ships, BattleModifiers, std::vector<shared_ptr<Ship>> def_ships, BattleModifiers): 
    _attacker_ships(att_ships), _defender_ships(def_ships) {
    initialSort ();
    initializeStateInfo ();
    initializeDiceRolls ();

}
