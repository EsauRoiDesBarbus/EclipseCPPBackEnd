
#include "battle_states_mother_class.hpp"
#include "battle_states_simple_class.hpp"
#include "bellman_algorithm.hpp"
#include "ship.hpp"
#include "ship_battle_states.hpp"

using namespace std;
#include <iostream>

int main(){

    //CruiserVSAncientBattleStates example;
    //CruiserAndIntVSIntBattleStates example (0); // if arg=1, then win chance is 97.57%, if arg=0, then win chance = 91.83%
    //cout << example.toString () << endl;
    //BattleResult result = winChanceAndExpectancyCalculator (example);
    //cout << result.toString () << endl;

    //Ship two_dreads_with_plasma_turret (2, DRE, 1, 2, 1, 2, {2,2,0,0,0});
    //Ship interceptor (1, INT, 3, 0, 0, 0, {1,0,0,0,0});
    //Ship cruiser (1, CRU, 2, 1, 1, 0, {1,0,0,0,0});
    //vector<RollUnallocated> rolls = cruiser.listRolls (1, {2,0});
    //rolls = two_dreads_with_plasma_turret.listRolls (1, {1,0});
    //rolls = two_dreads_with_plasma_turret.listRolls (2, {2,1,0,0});

    shared_ptr<Ship> interceptor = make_shared<Ship> (Ship(1, INT, 3, 0, 0, 0, {1,0,0,0,0}));
    shared_ptr<Ship> turbo_cruis = make_shared<Ship> (Ship(1, CRU, 3, 1, 1, 0, {1,0,0,0,0}));
    shared_ptr<Ship> rho_cruiser = make_shared<Ship> (Ship(1, CRU, 2, 1, 1, 1, {1,0,0,0,0}));
    shared_ptr<Ship> dreadnought = make_shared<Ship> (Ship(1, DRE, 1, 2, 1, 0, {2,0,0,0,0}));
    shared_ptr<Ship> dice_master = make_shared<Ship> (Ship(1, DRE, 1, 0, 1, 0, {2,2,0,1,0}));

    BattleModifiers att, def;

    //ShipBattleStates battle ({turbo_cruis}, att, {dreadnought, rho_cruiser, interceptor}, def);
    //ShipBattleStates battle ({dice_master}, att, {rho_cruiser, interceptor, interceptor}, def); //damage clock test
    //ShipBattleStates battle ({interceptor}, att, {rho_cruiser}, def);
    ShipBattleStates battle ({rho_cruiser}, att, {dreadnought}, def);
    cout << battle.toString () << endl;

    BattleResult result = winChanceAndExpectancyCalculator (battle);
    cout << result.toString () << endl;





    


    return 0;
}