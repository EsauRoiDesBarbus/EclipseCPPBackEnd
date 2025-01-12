/////////////////////////////////////////////////////
// Expose the C++ backend to be callable in Python //
// we use the Boost library                        //
/////////////////////////////////////////////////////
#include "ship_battle_states.hpp"
#include "bellman_algorithm.hpp"

#include <boost/python.hpp>

#include <string>
#include <vector>


using namespace boost::python;
using namespace std;

struct battleBuilder {
    // battle info
    vector<shared_ptr<Ship>> _attacker_ships;
    vector<shared_ptr<Ship>> _defender_ships;
    BattleModifiers _att;
    BattleModifiers _def;
    //ShipBattleStates _battle;
    BattleResult _result;

    battleBuilder (): _attacker_ships(0), _defender_ships(0), _att({false, false}), _def({false, false}) {}

    string help () {
        string output;
        output += "This is a helper class to enter battle info. Add ships, one at time, with:\n";
        output += "addShip (ATT/DEF, number, INT/CRU/DRE/SBA, initiative, hull, computer, shield, Weapons(y,o,b,r,p), Weapons(my,mo,mb,mr,mp))\n";
        output += "ny=number of yellow canons, o=orange canons... my=yellow missiles...\n";
        output += "Then, add modifiers (typically if one side follows NPC rules) with:\n";
        output += "addModifier (ATT/DEF, NPC/ANTIMATTER_SPLITTER)\n";
        output += "Finally, solve the battle with solveBattle ()\n";
        return output;
    }

    string addShip (string side_string, int number, string type_string, int initiative, int hull, int computer, int shield, Weapons canons, Weapons missiles) {
        // check side
        int side;
        if      (side_string=="ATT") side = ATTACKER;
        else if (side_string=="DEF") side = DEFENDER;
        else return "side (1st arg) should be ATT or DEF. No ship added.";

        //check type
        int type;
        if      (type_string=="INT") type = INT;
        else if (type_string=="CRU") type = CRU;
        else if (type_string=="DRE") type = DRE;
        else if (type_string=="SBA") type = SBA;
        else return "type (3rd arg) should be INT, CRU, DRE or SBA. No ship added.";

        //create ship
        if (side==ATTACKER) _attacker_ships.push_back (make_shared<Ship> (Ship(number, type, initiative, hull, computer, shield, canons, missiles)));
        else                _defender_ships.push_back (make_shared<Ship> (Ship(number, type, initiative, hull, computer, shield, canons, missiles)));

        return "OK";
    }

    string addModifier (string side_string, string modifier) {
        // check side
        int side;
        if      (side_string=="ATT") side = ATTACKER;
        else if (side_string=="DEF") side = DEFENDER;
        else return "side (1st arg) should be ATT or DEF. No modifier added.";

        // check modifier
        if      (modifier=="NPC") {
            if (side==ATTACKER) _att._is_npc = true;
            else                _def._is_npc = true;
        }
        else if (modifier=="ANTIMATTER_SPLITTER") {
            if (side==ATTACKER) _att._antimatter_splitter = true;
            else                _def._antimatter_splitter = true;
        }
        else return "modifier (2nd arg) should be NPC or ANTIMATTER_SPLITTER. No modifier added.";
        return "OK";
    }

    string solveBattle () {
        // check that there are at least 1 ship on each side
        if (_attacker_ships.size()==0) return "No attacker ship, use addShip";
        if (_defender_ships.size()==0) return "No defender ship, use addShip";

        //create and solve battle
        ShipBattleStates battle (_attacker_ships, _att, _defender_ships, _def);
        _result = winChanceAndExpectancyCalculator (battle);

        return _result.toString ();
    }

    float getAttackerWinChance () {
        return _result._attacker_win_chance;
    }
};

BOOST_PYTHON_MODULE(eclipseCpp) {
    class_<battleBuilder>("battleBuilder")
        .def("help"       , &battleBuilder::help)
        .def("addShip"    , &battleBuilder::addShip)
        .def("addModifier", &battleBuilder::addModifier)
        .def("solveBattle", &battleBuilder::solveBattle)
        .def("getAttackerWinChance", &battleBuilder::getAttackerWinChance)
    ;
    class_<Weapons>("Weapons", init<int, int, int, int, int>());
}











