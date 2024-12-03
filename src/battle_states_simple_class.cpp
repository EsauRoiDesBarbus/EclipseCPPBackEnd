#include "battle_states_simple_class.hpp"

using namespace std;

CruiserVSAncientBattleStates::CruiserVSAncientBattleStates (){
    // state index: damage taken by cruiser, damage taken by ancient, ship that is firing (A=Ancient, C=Cruiser)
    //  0: 0 0 A,  1: 0 0 C
    //  2: 0 1 A,  3: 0 1 C
    //  4: 0 2 A,  5: 0 2 C Attacker wins
    //  6: 1 0 A,  7: 1 0 C
    //  8: 1 1 A,  9: 1 1 C
    // 10: 1 2 A, 11: 1 2 C Attacker wins
    // 12: 2 0 A, 13: 2 0 C Defender wins
    // 14: 2 1 A, 15: 2 1 C Defender wins

    // list states where one side wins
    _states_where_attacker_wins = { 4, 5,10,11};
    _states_where_defender_wins = {12,13,14,15};

    //hard coded graph (the tricky part)
    _dice_rolls.resize (16);
    for (int i=0; i<5; i++){
        if (i==2) continue; //skip states 4 and 5 because attacker won
        // Ancient can roll 3 results : no hit, 1 hit, 2 hits
        _dice_rolls[2*i  ].resize (3);
        _dice_rolls[2*i  ][0] = {float (4.0/9.0), vector<int>(1, 2*i+1)};
        _dice_rolls[2*i  ][1] = {float (4.0/9.0), vector<int>(1, 2*i+7)};
        if (2*i+13<16) _dice_rolls[2*i  ][2] = {float (1.0/9.0), vector<int>(1, 2*i+13)};
        else           _dice_rolls[2*i  ][2] = {float (1.0/9.0), vector<int>(1, 2*i+ 7)};
        

        // cruiser can roll 2 results : no hit, 1 hit
        _dice_rolls[2*i+1].resize (2);
        _dice_rolls[2*i+1][0] = {float (2.0/3.0), vector<int>(1, 2*i  )};
        _dice_rolls[2*i+1][1] = {float (1.0/3.0), vector<int>(1, 2*i+2)};
    }
}