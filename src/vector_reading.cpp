#include "vector_reading.hpp"

#include <vector>
#include <iostream>

#include <map>

using namespace std;

shared_ptr<Ship> vectorToShipPointer (array<int, SHIP_VECTOR_SIZE> vec, int type) {
    Ship ship (vec[0], type, vec[1], vec[2], vec[3], vec[4], {vec[6], vec[7], vec[8], vec[9], vec[10]}, {vec[11], vec[12], vec[13], vec[14], vec[15]});
    return make_shared<Ship> (ship);
}

shared_ptr<ShipBattleStates> vectorToShipBattleStatesPointer (array<int, BATTLE_VECTOR_SIZE> vec, time_t timeout) {
    // base ships for simple battle test

    vector<shared_ptr<Ship>> attacker_ships(0), defender_ships(0);

    vector<int> ship_types = {INT, CRU, DRE, SBA};

    int it = 0;
    for (int ship=0; ship<3; ship++) { //attacker has 3 ship types
        if (vec[it]>0) {
            array<int, SHIP_VECTOR_SIZE> ship_vec;
            for (int i=0; i<SHIP_VECTOR_SIZE; i++) ship_vec[i] = vec[it+i];
            attacker_ships.push_back(vectorToShipPointer(ship_vec, ship_types[ship]));
        }
        it+=SHIP_VECTOR_SIZE;
    }
    BattleModifiers attacker_bonus({(vec[it  ]>0), (vec[it+1]>0)});
    it+=MODIFIER_VECTOR_SIZE;

    for (int ship=0; ship<4; ship++) { //defender has 4 ship types
        if (vec[it]>0) {
            array<int, SHIP_VECTOR_SIZE> ship_vec;
            for (int i=0; i<SHIP_VECTOR_SIZE; i++) ship_vec[i] = vec[it+i];
            defender_ships.push_back(vectorToShipPointer(ship_vec, ship_types[ship]));
        }
        it+=SHIP_VECTOR_SIZE;
    }
    BattleModifiers defender_bonus({(vec[it  ]>0), (vec[it+1]>0)});
    it+=MODIFIER_VECTOR_SIZE;

    ShipBattleStates battle_states (attacker_ships, attacker_bonus, defender_ships, defender_bonus, timeout);

    return make_shared<ShipBattleStates>(battle_states);
}

std::array<float, RESULT_VECTOR_SIZE> resultToVector (BattleResult result, shared_ptr<ShipBattleStates> battle_states) {
    // writes the battle result as a vector for the neural network
    //  0   : attacker win chance,
    //  1- 8: attacker INT survival chance,
    //  9-12: attacker CRU survival chance,
    // 13-14: attacker DRE survival chance,
    // 15-22: defender INT survival chance,
    // 23-26: defender CRU survival chance,
    // 27-28: defender DRE survival chance,
    // 29-32: defender SBA survival chance.

    map<int, int> attacker_place_in_vec;
    attacker_place_in_vec[INT]=  1;
    attacker_place_in_vec[CRU]=  9;
    attacker_place_in_vec[DRE]= 13;

    map<int, int> defender_place_in_vec;
    defender_place_in_vec[INT]= 15;
    defender_place_in_vec[CRU]= 23;
    defender_place_in_vec[DRE]= 27;
    defender_place_in_vec[SBA]= 29;

    std::array<float, RESULT_VECTOR_SIZE> vec;
    // if timeout, return -1.0, 0, 0,...
    if (result._timeout) {
        vec[0] = -1.0;
        return vec;
    }

    
    vec[0] = result._attacker_win_chance;
    
    for (int ship_id=0; ship_id<int(battle_states->_attacker_ships.size()); ship_id++) {
        int place_in_vec = attacker_place_in_vec[battle_states->_attacker_ships[ship_id]->_type];
        for (int i=0; i<int(result._attacker_ship_survival_chance[ship_id].size()); i++) {
            vec[place_in_vec+i] = result._attacker_ship_survival_chance[ship_id][i];
        }
    }

    for (int ship_id=0; ship_id<int(battle_states->_defender_ships.size()); ship_id++) {
        int place_in_vec = defender_place_in_vec[battle_states->_defender_ships[ship_id]->_type];
        for (int i=0; i<int(result._defender_ship_survival_chance[ship_id].size()); i++) {
            vec[place_in_vec+i] = result._defender_ship_survival_chance[ship_id][i];
        }
    }
    return vec;
}