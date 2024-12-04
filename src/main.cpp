
#include "battle_states_mother_class.hpp"
#include "battle_states_simple_class.hpp"
#include "bellman_algorithm.hpp"

using namespace std;
#include <iostream>

int main(){

    BattleStates example = CruiserVSAncientBattleStates ();
    //BattleStates example = CruiserAndIntVSIntBattleStates ();

    cout << example.toString () << endl;

    BattleResult dump = winChanceAndExpectancyCalculator (example);


    return 0;
}