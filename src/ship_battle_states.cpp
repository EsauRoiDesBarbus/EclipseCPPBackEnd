#include "ship_battle_states.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

#define DEBUG true

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

void ShipBattleStates::initializeStateVector () {
    // state will be represented by a vector (round, state of ship 1 in init order, state of ship 2....)
    // we create a vector to transform that into a single index
    int nb_ship_types = _ships_by_initiative.size ();
    _state_conversion_vector.resize (1+nb_ship_types);
    _state_conversion_vector[0]=1;

    int increment = 2*nb_ship_types; //first value is round_size
    for (int ship=0; ship<nb_ship_types; ship++) {
        _state_conversion_vector[1+ship]=increment;
        increment*=_ships_by_initiative[ship]->totalStates();
    }

    if (DEBUG) {
        cout << "state conversion vector=";
        for (int i=0; i<1+nb_ship_types; i++) cout << _state_conversion_vector[i] << ",";
        cout << endl;
    }
}


ShipBattleStates::ShipBattleStates (std::vector<Ship*> att_ships, BattleModifiers, std::vector<Ship*> def_ships, BattleModifiers): 
    _attacker_ships(att_ships), _defender_ships(def_ships) {
    initialSort ();
    initializeStateVector ();

}
