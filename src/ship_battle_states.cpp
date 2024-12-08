#include "ship_battle_states.hpp"

#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

#define DEBUG true



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

int ShipBattleStates::ExtendedStateToState (vector<int> extended_state) {
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

vector<int> ShipBattleStates::StateToExtendedState (int state) {
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
            cout << " conversion test " << ExtendedStateToState (state_clock._extended_state) << "<->(";
            vector<int> extended_state = StateToExtendedState (state_clock._state);
            for (int i=0; i<int(extended_state.size()); i++) cout<<extended_state[i]<<" ";
            cout << ")\n";
        }

        state_clock.increment ();
    }
}

vector<vector<int>> initializeDiceRollsHelper (StateClock& state_clock, DamageClock& damage_clock, vector<ShipWrapper>& _ships_by_shield) {
    // helper to easily switch between attack and defense ships
    int nb_ships_plus_1 = state_clock.size();
    vector<vector<int>> output(nb_ships_plus_1);
    // initialize with current clock values
    for (int i=0; i< nb_ships_plus_1; i++) output[i]={state_clock[i]};

    int nb_enemy_ships = _ships_by_shield.size ();
    for (int ship_by_shield=0; ship_by_shield<nb_enemy_ships; ship_by_shield++) {
        int ship_by_init = _ships_by_shield[ship_by_shield]._place_in_initiative_order;
        // state of the ship is state_clock[ship_by_init+1], allocated damage is damage_clock[ship_by_shield]
        output[1+ship_by_init] = _ships_by_shield[ship_by_shield]->takeHits (state_clock[ship_by_init+1], damage_clock[ship_by_shield]);
    }
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

    int start_state=0, close_state=0; //buffer for bundles

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
        for (int alive=0; alive<=max_ships; alive++) {// TODO missiles
            if (player==ATTACKER) big_table_of_rolls[nb_ships+ship][alive] = _both_ships_by_initiative[ship]->listRolls (alive,   CANONS, defender_shields);
            else                  big_table_of_rolls[nb_ships+ship][alive] = _both_ships_by_initiative[ship]->listRolls (alive,   CANONS, attacker_shields);
        }
    }

    //range all states
    for (int state=0; state<total_states; state++){
        int round = state_clock[0];
        int player = _both_ships_by_initiative[round%nb_ships]._side; //1 if attacker, -1 if defender
        int alive  = _both_ships_by_initiative[round%nb_ships]->countLiveShips (state_clock[1+(round%nb_ships)]);

        vector<RollUnallocated> rolls_unallocated = big_table_of_rolls[round][alive];
        int nb_rolls = rolls_unallocated.size ();
        _dice_rolls[state].resize(nb_rolls); //output
        for (int roll=0; roll<nb_rolls; roll++) {
            // copy proba
            _dice_rolls[state][roll]._proba = rolls_unallocated[roll]._proba;

            //allocate damage and deduce end states (the hard part)
            DamageClock damage_clock (rolls_unallocated[roll]);
            bool finished = false;
            //cout << rolls_unallocated[roll].toString() <<endl;;
            while (finished==false) {
                //cout << damage_clock.toString() << endl;
                finished = damage_clock.increment ();
            }
            // compute all possible extended states
            vector<vector<int>> all_extended_states;
            if (player==ATTACKER) all_extended_states = initializeDiceRollsHelper (state_clock, damage_clock, _defender_ships_by_shield);
            else                  all_extended_states = initializeDiceRollsHelper (state_clock, damage_clock, _attacker_ships_by_shield);

            // increase round number (after a ship fires, the next ship in initiative fires)
            // if the last ship is firing his canon, we go back to the start of the canon round, that is 2*nb_ships-1->nb_ships
            if (state_clock[0]<2*nb_ships-1) all_extended_states[0] = {state_clock[0]+1};
            else                             all_extended_states[0] = {        nb_ships};

            // transform into an array of states TODO range all possibilities, we only do one here
            vector<int> extended_state (1+nb_ships);
            for (int i=0; i<1+nb_ships;i++) extended_state[i] = all_extended_states[i][0];

            int next_state = ExtendedStateToState (extended_state);

            //TODO update bundle info if that state is before the current state

            _dice_rolls[state][roll]._allocations.push_back(next_state);
        }
        state_clock.increment ();
    }
}




ShipBattleStates::ShipBattleStates (std::vector<shared_ptr<Ship>> att_ships, BattleModifiers, std::vector<shared_ptr<Ship>> def_ships, BattleModifiers): 
    _attacker_ships(att_ships), _defender_ships(def_ships) {
    initialSort ();
    initializeStateInfo ();
    initializeDiceRolls ();

}
