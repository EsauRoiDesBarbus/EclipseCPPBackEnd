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

    StateClock (vector<ShipWrapper> ships) {
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
};


void ShipBattleStates::initialSort () {
    // create 
    int nb_attacker_ships = _attacker_ships.size();
    int nb_defender_ships = _defender_ships.size();
    _attacker_ships_by_shield.resize(nb_attacker_ships);
    _defender_ships_by_shield.resize(nb_defender_ships);
    for (int ship=0; ship<nb_attacker_ships; ship++) _attacker_ships_by_shield[ship] = {_attacker_ships[ship], 1, ship};
    for (int ship=0; ship<nb_defender_ships; ship++) _defender_ships_by_shield[ship] = {_defender_ships[ship],-1, ship};
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
        cout << "Ships by initiative=";
        for (int i=0; i<int(_both_ships_by_initiative.size()); i++) cout << _both_ships_by_initiative[i]->_type <<" "<< _both_ships_by_initiative[i]->_init << ",";
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
    // initialize _who_is_firing, _state_bundles, _states_where_attacker_wins, _states_where_defender_wins and _live_ships
    StateClock state_clock (_both_ships_by_initiative);
    int total_states = state_clock.totalStates ();
    int nb_ships = _both_ships_by_initiative.size (); 

    // allocate memory
    _who_is_firing.resize(total_states);
    _live_ships.resize(total_states);
    _state_bundles.resize(0);
    _states_where_attacker_wins.resize(0);
    _states_where_defender_wins.resize(0);
    _dice_rolls.resize (total_states);

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



ShipBattleStates::ShipBattleStates (std::vector<shared_ptr<Ship>> att_ships, BattleModifiers, std::vector<shared_ptr<Ship>> def_ships, BattleModifiers): 
    _attacker_ships(att_ships), _defender_ships(def_ships) {
    initialSort ();
    initializeStateInfo ();

}
