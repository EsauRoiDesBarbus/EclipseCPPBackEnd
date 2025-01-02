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

#define ATT 1
#define DEF 0

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
        return "This is a helper class to enter battle info.\nAdd ships, one at time, with addShip (ATT/DEF, number, INT/CRU/DRE/SBA, initiative, hull, computer, shield, y, o, b, r, p).\nThen, call solveBattle ()\n";
    }

    string addShip (string side_string, int number, string type_string, int initiative, int hull, int computer, int shield, int y, int o, int b, int r, int p) {
        // check side
        int side;
        if      (side_string=="ATT") side = ATT;
        else if (side_string=="DEF") side = DEF;
        else return "side (1st arg) should be ATT or DEF";

        //check type
        int type;
        if      (type_string=="INT") type = INT;
        else if (type_string=="CRU") type = CRU;
        else if (type_string=="DRE") type = DRE;
        else if (type_string=="SBA") type = SBA;
        else return "type (3rd arg) should be INT, CRU, DRE or SBA";

        //create ship
        Weapons canons ({y,o,b,r,p});
        if (side==ATT) _attacker_ships.push_back (make_shared<Ship> (Ship(number, type, initiative, hull, computer, shield, canons)));
        else           _defender_ships.push_back (make_shared<Ship> (Ship(number, type, initiative, hull, computer, shield, canons)));

        return "OK";
    } //TODO add missiles

    string solveBattle () {
        ShipBattleStates battle (_attacker_ships, _att, _defender_ships, _def);
        _result = winChanceAndExpectancyCalculator (battle);

        return _result.toString ();
    } //TODO check number of ships on each side
};

BOOST_PYTHON_MODULE(eclipseCpp) {
    class_<battleBuilder>("battleBuilder")
        .def("help"       , &battleBuilder::help)
        .def("addShip"    , &battleBuilder::addShip)
        .def("solveBattle", &battleBuilder::solveBattle)
    ;
}











