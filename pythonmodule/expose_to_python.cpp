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

boost::python::list vectorToPythonList(const std::vector<float>& vec) {
    boost::python::list py_list;
    for (int i =0; i<int(vec.size()); ++i) py_list.append(vec[i]);
    return py_list;
}

struct BattleBuilder {
    // battle info
    vector<shared_ptr<Ship>> _attacker_ships;
    vector<shared_ptr<Ship>> _defender_ships;
    BattleModifiers _att;
    BattleModifiers _def;
    //ShipBattleStates _battle;
    BattleResult _result;

    float _timeout=60;

    BattleBuilder (): _attacker_ships(0), _defender_ships(0), _att({false, false}), _def({false, false}) {}

    string help () {
        string output;
        output += "This is a helper class to enter battle info. Add ships, one at time, with:\n";
        output += "addShip (ATT/DEF, number, INT/CRU/DRE/SBA, initiative, hull, computer, shield, [y,o,b,r,p], [my,mo,mb,mr,mp])\n";
        output += "ny=number of yellow canons, o=orange canons... my=yellow missiles...\n";
        output += "Then, add modifiers (typically if one side follows NPC rules) with:\n";
        output += "addModifier (ATT/DEF, NPC/ANTIMATTER_SPLITTER)\n";
        output += "When all info is entered, solve the battle with: solveBattle (t)\n";
        output += "where t is an optional timeout in seconds (default is 60).\n";
        output += "You may then access the result as a dictionary with: getResult()\n";
        return output;
    }

    string addShip (string side_string, int number, string type_string, int initiative, int hull, int computer, int shield, boost::python::list canons_list, boost::python::list missiles_list) {
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

        //convert lists into weapons
        if ((len(canons_list)!=5)or(len(missiles_list)!=5)) return "Weapons (8th and 9th args) should be lists with 5 elements. No ship added.";
        Weapons canons, missiles;
        for (int i=0; i<5; ++i) {
            canons[i]   = extract<int>(canons_list [i]);
            missiles[i] = extract<int>(missiles_list [i]);
        }

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

    string setTimeout (float timeout) {
        if (timeout<0) return "timeout should be positive";
        _timeout = timeout;
        return "OK";
    }

    string solveBattle (float timeout=60) {
        //set timeout
        if (timeout<0) return "timeout should be positive";
        _timeout = timeout;
        // check that there are at least 1 ship on each side
        if (_attacker_ships.size()==0) return "No attacker ship, use addShip";
        if (_defender_ships.size()==0) return "No defender ship, use addShip";

        //create and solve battle
        ShipBattleStates battle (_attacker_ships, _att, _defender_ships, _def, _timeout/2);
        _result = winChanceAndExpectancyCalculator (battle, _timeout/2);

        return _result.toString ();
    }

    float getAttackerWinChance () {
        return _result._attacker_win_chance;
    }

    boost::python::dict getResult () {
        boost::python::dict result_dict;

        result_dict["attacker_win_chance"]           = _result._attacker_win_chance;
        boost::python::list attacker_ships, defender_ships;
        for (int i=0; i<int(_result._attacker_ship_survival_chance.size()); ++i)
            attacker_ships.append (vectorToPythonList(_result._attacker_ship_survival_chance[i]));
        for (int i=0; i<int(_result._defender_ship_survival_chance.size()); ++i)
            defender_ships.append (vectorToPythonList(_result._defender_ship_survival_chance[i]));
        result_dict["attacker_ship_survival_chance"] = attacker_ships;
        result_dict["defender_ship_survival_chance"] = defender_ships;

        return result_dict;
    }

    int getTimeoutStatus () {
        return _result._timeout;
    }
};

BOOST_PYTHON_MODULE(eclipseCpp) {
    class_<BattleBuilder>("BattleBuilder")
        .def("help"       , &BattleBuilder::help)
        .def("addShip"    , &BattleBuilder::addShip)
        .def("addModifier", &BattleBuilder::addModifier)
        .def("solveBattle", &BattleBuilder::solveBattle, (boost::python::arg("timeout")=60))
        .def("getResult"  , &BattleBuilder::getResult)
        .def("getTimeoutStatus", &BattleBuilder::getTimeoutStatus)
    ;
}











