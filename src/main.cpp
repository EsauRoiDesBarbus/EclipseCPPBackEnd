
#include "battle_states_mother_class.hpp"
#include "battle_states_simple_class.hpp"
#include "bellman_algorithm.hpp"
#include "ship.hpp"
#include "ship_battle_states.hpp"

using namespace std;
#include <iostream>
#include <chrono>

int main(){
    BattleResult result;
    // use battlestates sub class to test bellman algorithm
    cout << "Using battlestates sub class to test bellman algorithm.\nShould return 17.03%, 97.57% and 91.83%" << endl; 
    BattleStates simple_example = CruiserVSAncientBattleStates();
    result = winChanceAndExpectancyCalculator (simple_example);
    cout << result.toString () << endl;

    simple_example = CruiserAndIntVSIntBattleStates (1); // if arg=1, then win chance is 97.57%, if arg=0, then win chance = 91.83%
    result = winChanceAndExpectancyCalculator (simple_example);
    cout << result.toString () << endl;

    simple_example = CruiserAndIntVSIntBattleStates (0); // if arg=1, then win chance is 97.57%, if arg=0, then win chance = 91.83%
    result = winChanceAndExpectancyCalculator (simple_example);
    cout << result.toString () << endl;

    //Ship two_dreads_with_plasma_turret (2, DRE, 1, 2, 1, 2, {2,2,0,0,0});
    //Ship interceptor (1, INT, 3, 0, 0, 0, {1,0,0,0,0});
    //Ship cruiser (1, CRU, 2, 1, 1, 0, {1,0,0,0,0});
    //vector<RollUnallocated> rolls = cruiser.listRolls (1, CANONS, {2,0});
    //rolls = two_dreads_with_plasma_turret.listRolls (1, CANONS, {1,0});
    //rolls = two_dreads_with_plasma_turret.listRolls (2, CANONS, {2,1,0,0});

    ClockOrganizerTest test;
    test.iterationTest ({4, 5, 2}, {3, 4, 1}, false);




    // base ships for simple battle test
    shared_ptr<Ship> interceptor = make_shared<Ship> (Ship(1, INT, 3, 0, 0, 0, {1,0,0,0,0}));
    shared_ptr<Ship> basecruiser = make_shared<Ship> (Ship(1, CRU, 2, 1, 1, 0, {1,0,0,0,0}));
    shared_ptr<Ship> dreadnought = make_shared<Ship> (Ship(1, DRE, 1, 2, 1, 0, {2,0,0,0,0}));
    shared_ptr<Ship> baseancient = make_shared<Ship> (Ship(1, DRE, 2, 1, 1, 0, {2,0,0,0,0}));

    BattleModifiers att({false, false}), def({false, false});

    ShipBattleStates battle ({basecruiser, interceptor}, att, {baseancient}, def);
    result = winChanceAndExpectancyCalculator (battle);
    //cout << battle.toString () << endl;
    cout << "simple battle test " << result.toString () << endl;



    shared_ptr<Ship> turbo_cruis = make_shared<Ship> (Ship(1, CRU, 3, 1, 1, 0, {1,0,0,0,0}));
    shared_ptr<Ship> rho_cruiser = make_shared<Ship> (Ship(1, CRU, 2, 1, 1, 1, {1,0,0,0,0}));
    shared_ptr<Ship> dice_master = make_shared<Ship> (Ship(1, DRE, 1, 0, 1, 0, {2,2,0,1,0}));
    shared_ptr<Ship> no_hull_cru = make_shared<Ship> (Ship(1, CRU, 2, 0, 1, 0, {1,0,0,0,0}));
    shared_ptr<Ship> no_hull_dre = make_shared<Ship> (Ship(1, DRE, 1, 0, 1, 0, {2,0,0,0,0}));
    
    
    

    

    //ShipBattleStates battle ({turbo_cruis}, att, {dreadnought, rho_cruiser, interceptor}, def);
    //ShipBattleStates battle ({dice_master}, att, {rho_cruiser, interceptor, interceptor}, def); //damage clock test
    //ShipBattleStates battle ({dice_master, interceptor}, att, {dreadnought, interceptor, turbo_cruis}, def); //big battle test

    // optimal targeting test
    shared_ptr<Ship> missile_int = make_shared<Ship> (Ship(1, INT, 3, 0, 2, 0, {0,0,0,0,0}, {2,0,0,0,0}));
    shared_ptr<Ship> missile_cru = make_shared<Ship> (Ship(1, CRU, 2, 1, 2, 0, {0,0,0,0,0}, {0,0,0,2,0}));
    shared_ptr<Ship> space_brick = make_shared<Ship> (Ship(2, DRE, 1, 4, 1, 0, {2,0,0,0,0}));

    battle = ShipBattleStates ({missile_int, missile_cru}, att, {space_brick}, def);
    result = winChanceAndExpectancyCalculator (battle);
    //cout << battle.toString () << endl;
    cout << "optimal targeting test (should be 6.25%) " << result.toString () << endl;

    // NPC rule test
    shared_ptr<Ship> dummy___dre = make_shared<Ship> (Ship(1, DRE, 1, 4, 1, 0, {0,0,0,0,0})); //harmless ships to test damage allocation
    shared_ptr<Ship> dummy___int = make_shared<Ship> (Ship(1, INT, 1, 0, 1, 0, {0,0,0,0,0})); //harmless ships to test damage allocation

    battle = ShipBattleStates ({dummy___dre, basecruiser}, att, {basecruiser}, def);
    result = winChanceAndExpectancyCalculator (battle);
    //cout << battle.toString () << endl;
    cout << "normal targeting test (should be 44.8%) " << result.toString () << endl;

    def._is_npc = true;
    battle = ShipBattleStates({dummy___dre, basecruiser}, att, {basecruiser}, def);
    result = winChanceAndExpectancyCalculator (battle);
    //cout << battle.toString () << endl;
    cout << "npc targeting test (should be 98.33%) " << result.toString () << endl;
    def._is_npc = false;




    if (true) {

        shared_ptr<Ship> riftcruiser = make_shared<Ship> (Ship(1, CRU, 2, 1, 1, 0, {0,0,0,0,1}));
        shared_ptr<Ship> rift_stbase = make_shared<Ship> (Ship(2, SBA, 4, 0, 1, 0, {0,0,0,0,1}));
        


        vector<RollUnallocated> rolls = riftcruiser->listRolls (1, CANONS, {2, 1, 0});
        for (int i=0; i< int(rolls.size()); i++) cout << rolls[i].toString () << endl;

        battle = ShipBattleStates ({riftcruiser}, att, {interceptor}, def);
        result = winChanceAndExpectancyCalculator (battle);
        //cout << battle.toString () << endl;
        cout << "1 rift ship  " << result.toString () << endl;

        battle = ShipBattleStates({riftcruiser, rift_stbase}, att, {interceptor}, def);
        //cout << battle.toString () << endl;
        result = winChanceAndExpectancyCalculator (battle);
        cout << "3 rift ships " << result.toString () << endl;
    }
    if (true) {

        shared_ptr<Ship> rift_cru_x2 = make_shared<Ship> (Ship(2, CRU, 0, 8, 0, 0, {0,0,0,0,1}));
        shared_ptr<Ship> rift_int_x2 = make_shared<Ship> (Ship(2, CRU, 0, 2, 0, 0, {0,0,0,0,1}));
        shared_ptr<Ship> gretech_dre = make_shared<Ship> (Ship(1, DRE, 3, 4, 4, 0, {0,0,0,2,0}));
        battle = ShipBattleStates({gretech_dre}, att, {rift_cru_x2}, def);
        result = winChanceAndExpectancyCalculator (battle);
        //cout << battle1.toString () << endl;
        cout << "rift cruiser test " << result.toString () << endl;

        //ShipBattleStates battle2 ({gretech_dre}, att, {rift_cru_x2, rift_int_x2}, def);
        //BattleResult result2 = winChanceAndExpectancyCalculator (battle2);
        //cout << battle2.toString () << endl;
        //cout << "rift cruiser test " << result2.toString () << endl;

        //perf test
        shared_ptr<Ship> elliot_ints = make_shared<Ship> (Ship(4, INT, 4, 0, 2, 0, {0,1,0,0,0}));
        shared_ptr<Ship> elliot_crus = make_shared<Ship> (Ship(3, CRU, 4, 2, 4, 0, {0,1,0,0,0}));
        shared_ptr<Ship> etienn_dres = make_shared<Ship> (Ship(2, DRE, 2, 8, 0, 2, {0,0,0,0,1}));
        shared_ptr<Ship> etienn_ints = make_shared<Ship> (Ship(3, INT, 3, 0, 0, 0, {2,0,0,0,0}));

        clock_t start = clock();

        battle = ShipBattleStates ({elliot_ints, elliot_crus}, att, {etienn_dres, etienn_ints}, def);
        //battle = ShipBattleStates ({elliot_ints, elliot_crus}, att, {etienn_dres}, def);
        result = winChanceAndExpectancyCalculator (battle);
        //cout << battle.toString () << endl;
        clock_t end = clock();
        cout << "fatefull battle Etienne Elliot " << result.toString () << endl;
        cout << "perf test " << double(end-start)/CLOCKS_PER_SEC << "s" << endl;

    }
    


    return 0;
}