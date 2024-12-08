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

    StateClock (vector<Ship*> ships) {
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
    // sort ships by shield
    sort(_attacker_ships.begin(), _attacker_ships.end(), [](const Ship* a, const Ship* b) {
        return a->_shield > b->_shield;  // Compare based on shield
    });
    sort(_defender_ships.begin(), _defender_ships.end(), [](const Ship* a, const Ship* b) {
        return a->_shield > b->_shield;  // Compare based on shield
    });

    if (DEBUG) {
        cout << "attacker shields=";
        for (int i=0; i<int(_attacker_ships.size()); i++) cout << _attacker_ships[i]->_shield << ",";
        cout << "defender shields=";
        for (int i=0; i<int(_defender_ships.size()); i++) cout << _defender_ships[i]->_shield << ",";
        cout << endl;
    }

    // create another list of ships sorted in initiative order
    _ships_by_initiative = _defender_ships;
    _ships_by_initiative.insert(_ships_by_initiative.end(), _attacker_ships.begin(), _attacker_ships.end());
    sort(_ships_by_initiative.begin(), _ships_by_initiative.end(), [](const Ship* a, const Ship* b) {
        return a->_init > b->_init;  // Compare based on shield
    });

    if (DEBUG) {
        cout << "Ships by initiative=";
        for (int i=0; i<int(_ships_by_initiative.size()); i++) cout << _ships_by_initiative[i]->_type <<" "<< _ships_by_initiative[i]->_init << ",";
        cout << endl;
    }
}

int ShipBattleStates::ExtendedStateToState (vector<int> extended_state) {
    int state = 0;
    int nb_ship_types = _ships_by_initiative.size();
    for (int ship = nb_ship_types-1; ship>=0; ship--) {
        state*= _ships_by_initiative[ship]->totalStates ();
        state+= extended_state[1+ship];
    }
    state*= 2*_ships_by_initiative.size();
    state+= extended_state [0];
    return state;
}

vector<int> ShipBattleStates::StateToExtendedState (int state) {
    int nb_ship_types = _ships_by_initiative.size();
    vector<int> extended_state (1+nb_ship_types);
    int nb_rounds = 2*nb_ship_types;
    extended_state[0] = state%nb_rounds;
    state/=nb_rounds;
    for (int ship = 0; ship<nb_ship_types; ship++) {
        int total_states = _ships_by_initiative[ship]->totalStates ();
        extended_state[1+ship] = state%total_states;
        state/= total_states;
    }
    return extended_state;
}

void ShipBattleStates::initializeStateInfo () {
    StateClock state_clock (_ships_by_initiative);
    int total_states = state_clock.totalStates ();
    for (int state=0; state<total_states; state++){
        

        if (DEBUG) {
            cout << state_clock.toString ();
            cout << " conversion test " << ExtendedStateToState (state_clock._extended_state) << "<->";
            vector<int> extended_state = StateToExtendedState (state_clock._state);
            for (int i=0; i<int(extended_state.size()); i++) cout<<extended_state[i]<<",";
            cout << endl;
        }

        state_clock.increment ();
    }
}



ShipBattleStates::ShipBattleStates (std::vector<Ship*> att_ships, BattleModifiers, std::vector<Ship*> def_ships, BattleModifiers): 
    _attacker_ships(att_ships), _defender_ships(def_ships) {
    initialSort ();
    initializeStateInfo ();

}
