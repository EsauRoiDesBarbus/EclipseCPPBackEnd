
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
    //vector<RollUnallocated> rolls = cruiser.listRolls (1, CANONS, {2,0});
    //rolls = two_dreads_with_plasma_turret.listRolls (1, CANONS, {1,0});
    //rolls = two_dreads_with_plasma_turret.listRolls (2, CANONS, {2,1,0,0});

    shared_ptr<Ship> interceptor = make_shared<Ship> (Ship(1, INT, 3, 0, 0, 0, {1,0,0,0,0}));
    shared_ptr<Ship> basecruiser = make_shared<Ship> (Ship(1, CRU, 2, 1, 1, 0, {1,0,0,0,0}));
    shared_ptr<Ship> dreadnought = make_shared<Ship> (Ship(1, DRE, 1, 2, 1, 0, {2,0,0,0,0}));
    shared_ptr<Ship> turbo_cruis = make_shared<Ship> (Ship(1, CRU, 3, 1, 1, 0, {1,0,0,0,0}));
    shared_ptr<Ship> rho_cruiser = make_shared<Ship> (Ship(1, CRU, 2, 1, 1, 1, {1,0,0,0,0}));
    shared_ptr<Ship> dice_master = make_shared<Ship> (Ship(1, DRE, 1, 0, 1, 0, {2,2,0,1,0}));
    shared_ptr<Ship> no_hull_cru = make_shared<Ship> (Ship(1, CRU, 2, 0, 1, 0, {1,0,0,0,0}));
    shared_ptr<Ship> no_hull_dre = make_shared<Ship> (Ship(1, DRE, 1, 0, 1, 0, {2,0,0,0,0}));
    shared_ptr<Ship> dummy___dre = make_shared<Ship> (Ship(1, DRE, 1, 8, 1, 0, {0,0,0,0,0})); //harmless ships to test damage allocation
    shared_ptr<Ship> dummy___int = make_shared<Ship> (Ship(1, INT, 1, 0, 1, 0, {0,0,0,0,0})); //harmless ships to test damage allocation
    shared_ptr<Ship> riftcruiser = make_shared<Ship> (Ship(1, CRU, 2, 1, 1, 0, {0,0,0,0,1}));
    shared_ptr<Ship> rift_stbase = make_shared<Ship> (Ship(2, SBA, 4, 0, 1, 0, {0,0,0,0,1}));
    

    BattleModifiers att({false, false}), def({false, false});

    //ShipBattleStates battle ({turbo_cruis}, att, {dreadnought, rho_cruiser, interceptor}, def);
    //ShipBattleStates battle ({dice_master}, att, {rho_cruiser, interceptor, interceptor}, def); //damage clock test
    //ShipBattleStates battle ({interceptor}, att, {rho_cruiser}, def);
    //ShipBattleStates battle ({dice_master, interceptor}, att, {dreadnought, interceptor, turbo_cruis}, def); //big battle test
    ShipBattleStates battle1 ({dummy___dre, basecruiser}, att, {interceptor}, def);
    BattleResult result1 = winChanceAndExpectancyCalculator (battle1);
    //cout << battle1.toString () << endl;
    cout << "normal rules " << result1.toString () << endl;

    def._is_npc = true;
    ShipBattleStates battle2 ({dummy___dre, basecruiser}, att, {interceptor}, def);
    BattleResult result2 = winChanceAndExpectancyCalculator (battle2);
    cout << "npc    rules " << result2.toString () << endl;
    def._is_npc = false;


    vector<RollUnallocated> rolls = riftcruiser->listRolls (1, CANONS, {2, 1, 0});
    for (int i=0; i< int(rolls.size()); i++) cout << rolls[i].toString () << endl;

    ShipBattleStates battle3 ({riftcruiser}, att, {interceptor}, def);
    BattleResult result3 = winChanceAndExpectancyCalculator (battle3);
    //cout << battle3.toString () << endl;
    cout << "1 rift ship  " << result3.toString () << endl;

    ShipBattleStates battle4 ({riftcruiser, rift_stbase}, att, {interceptor}, def);
    BattleResult result4 = winChanceAndExpectancyCalculator (battle4);
    //cout << battle4.toString () << endl;
    cout << "3 rift ships " << result4.toString () << endl;

    






    


    return 0;
}