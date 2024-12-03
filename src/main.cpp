
#include "battle_states_mother_class.hpp"
#include "battle_states_simple_class.hpp"

using namespace std;
#include <iostream>

int main(){

    BattleStates example = CruiserVSAncientBattleStates ();

    cout << example.toString () << endl;

    return 0;
}