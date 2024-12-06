
#include "battle_states_mother_class.hpp"
#include "battle_states_simple_class.hpp"
#include "bellman_algorithm.hpp"

using namespace std;
#include <iostream>

int main(){

    //CruiserVSAncientBattleStates example;
    CruiserAndIntVSIntBattleStates example (0); // if arg=1, then win chance is 97.57%, if arg=0, then win chance = 91.83%

    cout << example.toString () << endl;

    BattleResult result = winChanceAndExpectancyCalculator (example);

    cout << result.toString () << endl;


    return 0;
}