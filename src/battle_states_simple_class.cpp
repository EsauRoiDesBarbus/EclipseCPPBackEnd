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

    _who_is_firing = vector<int> (16, 1.0);
    for (int i=0; i<8; i++) _who_is_firing[2*i] = -1.0; //states where ancient is firing

    _state_bundles.resize (4); //states that need to be computed in bundle
    _state_bundles[0] = make_tuple(0,1);
    _state_bundles[1] = make_tuple(2,3);
    _state_bundles[2] = make_tuple(6,7);
    _state_bundles[3] = make_tuple(8,9);

    //live ships
    _live_attacker_ships.resize (16);
    _live_defender_ships.resize (16);
    for (int i=0; i<16; i++) {
        _live_attacker_ships[i] = {1};
        _live_defender_ships[i] = {1};
    }
    _live_defender_ships[ 4] = {0}; _live_defender_ships[ 5] = {0};
    _live_defender_ships[10] = {0}; _live_defender_ships[11] = {0};
    _live_attacker_ships[12] = {0}; _live_attacker_ships[13] = {0};
    _live_attacker_ships[14] = {0}; _live_attacker_ships[15] = {0};


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

CruiserAndIntVSIntBattleStates::CruiserAndIntVSIntBattleStates (int cruiser_computer) {
    // state index: damage taken by cruiser, damage taken by attack interceptor, damage taken by defense interceptor, ship that is firing (D = defense interceptor, I = attack interceptor, C=Cruiser)
    //  0: 0 0 0 D,  1: 0 0 0 I,  2: 0 0 0 C
    //  3: 0 1 0 D,  4: 0 1 0 I,  5: 0 1 0 C
    //  6: 1 0 0 D,  7: 1 0 0 I,  8: 1 0 0 C
    //  9: 1 1 0 D, 10: 1 1 0 I, 11: 1 1 0 C
    // 12: 2 0 0 D, 13: 2 0 0 I, 14: 2 0 0 C
    // 15: 2 1 0 D, 16: 2 1 0 I, 17: 2 1 0 C Defender wins
    // 18: * 0 1 *, 19: * 1 1 *, 20: 2 0 1 * Attacker wins (all states where defense is dead are condensed into one otherwis state space would be twice as big)
    _states_where_attacker_wins = {18,19,20};
    _states_where_defender_wins = {15,16,17};

    _who_is_firing = vector<int> (21, 1.0);
    for (int i=0; i<7; i++) _who_is_firing[3*i] = -1.0; //states where defender is firing

    _state_bundles.resize (5); //states that need to be computed in bundle
    for (int i =0; i<5; i++) _state_bundles[i] = make_tuple(3*i,3*i+2);

    //live ships
    _live_attacker_ships.resize (21);
    _live_defender_ships.resize (21);
    for (int i=0; i<18; i++)    {
        _live_attacker_ships [i] = {1, 1};
        _live_defender_ships [i] = {1};
    }
    _live_attacker_ships[18] = {0, 0};
    _live_attacker_ships[19] = {0, 0};
    _live_attacker_ships[20] = {0, 0};

    _live_attacker_ships[18] = {1, 1};
    _live_attacker_ships[19] = {1, 0};
    _live_attacker_ships[20] = {0, 1};
    

    //hard coded graph (the tricky part)
    _dice_rolls.resize (21);

    for (int i=0; i<5; i++){
        // Def int can roll 2 results : no hit, 1 hit; if it hits, it can either focus the int or cru
        _dice_rolls[3*i  ].resize (2);
        _dice_rolls[3*i  ][0] = {float (5.0/6.0), {3*i+1}};
        _dice_rolls[3*i  ][1] = {float (1.0/6.0), {3*i+4, 3*i+7}};

        if (i%2==0) {
            //attack interceptor is alive
            _dice_rolls[3*i+1].resize (2);
            _dice_rolls[3*i+1][0] = {float (5.0/6.0), {3*i+2}};
            _dice_rolls[3*i+1][1] = {float (1.0/6.0), {20}};

            _dice_rolls[3*i  ][1] = {float (1.0/6.0), {3*i+4}};
        } else {
            //attack interceptor is dead
            _live_attacker_ships[3*i  ][1] = 0;
            _live_attacker_ships[3*i+1][1] = 0;
            _live_attacker_ships[3*i+2][1] = 0;

            _dice_rolls[3*i+1].resize (1);
            _dice_rolls[3*i+1][0] = {float (1.0), {3*i+2}};

        }

        if (i<4) {
            //attack cruiser is alive
            _dice_rolls[3*i+2].resize (2);
            _dice_rolls[3*i+2][0] = {float ((5.0-cruiser_computer)/6.0), {3*i}};
            _dice_rolls[3*i+2][1] = {float ((1.0+cruiser_computer)/6.0), {19}};

            _dice_rolls[3*i  ][1] = {float (1.0/6.0), {3*i+7}};
        } else {
            //attack cruiser is dead
            _live_attacker_ships[3*i  ][0] = 0;
            _live_attacker_ships[3*i+1][0] = 0;
            _live_attacker_ships[3*i+2][0] = 0;

            _dice_rolls[3*i+2].resize (1);
            _dice_rolls[3*i+2][0] = {float (1.0), {3*i}};
        }

        if ((i%2==0)and(i<4)) {
            //both attack ships are alive
            _dice_rolls[3*i  ][1] = {float (1.0/6.0), {3*i+4, 3*i+7}};
            // if cruiser or int hits, they go to end state where both attacker ships are alive
            _dice_rolls[3*i+1][1] = {float (1.0/6.0), {18}};
            _dice_rolls[3*i+2][1] = {float ((1.0+cruiser_computer)/6.0), {18}};
        }
    }

}