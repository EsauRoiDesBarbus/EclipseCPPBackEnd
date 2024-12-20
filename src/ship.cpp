//////////////////////////////////////////////
// Default implementation of the ship class //
//////////////////////////////////////////////

// in this implementation, the range all possible damage allocations between ships of the same type
// it is represented as (a0, a1..., an-1) where n is the number of ships of that type
// an-1 is the damage taken by ALL ships, an-2 is the damage taken by all ships except one and so on
// that is, the damage taken by ship k in [[0, n-1]] is an-1 + an-2.... + ak
// the order is such so that whichever way we allocate incoming damage, the clock organizer index always increases, which is necessary for Bellman
// sum ai is equal to the damage taken by the last ship, hence it is inferior or equal to hull+1

#include "ship.hpp"
#include "dice_organizer.hpp"
#include "single_damage_organizer.hpp"

#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

#define DEBUG false


Ship::Ship (int n, int t, int i, int h, int c, int s, Weapons can, Weapons mis):
    _number(n), _type(t), _init(i),_hull(h),_computer(c),_shield(s),_canons(can),_missiles(mis)         {initializeClockOrganizer();}

Ship::Ship (int n, int t, int i, int h, int c, int s, Weapons can):
    _number(n), _type(t), _init(i),_hull(h),_computer(c),_shield(s),_canons(can),_missiles({0,0,0,0,0}) {initializeClockOrganizer();}

void Ship::initializeClockOrganizer () {
    setBounds ({_hull+1}, {_number});
}

////////////////////////////////////////////////////////
// conversion between iteration, ship state and index //
////////////////////////////////////////////////////////

vector<int> Ship::iterationToShipState (vector<int> iteration) {
    vector<int> ship_state(_number, 0);
    ship_state[0] = iteration[_number-1];
    for (int ship=1; ship<_number; ship++) ship_state[ship]=ship_state[ship-1]+iteration[_number-1-ship];
    return ship_state;
}

vector<int> Ship::shipStateToIteration (vector<int> ship_state) {
    sort (ship_state.begin(), ship_state.end()); //sort in increasing order
    vector<int> iteration(_number, 0);
    iteration[_number-1] = ship_state[0];
    for (int ship=1; ship<_number; ship++) iteration[_number-1-ship]=ship_state[ship]-ship_state[ship-1];
    return iteration;
}

vector<int> Ship::stateToShipState (int state) {
    vector<int> iteration = indexToIteration (state);
    return iterationToShipState (iteration);
}

int Ship::shipStateToState (vector<int> ship_state) {
    vector<int> iteration = shipStateToIteration (ship_state);
    return iterationToIndex (iteration);
}

////////////////////////////////////
// functions for ShipBattleStates //
////////////////////////////////////

int Ship::countLiveShips (int state) {
    vector <int> ship_state = stateToShipState (state);
    int alive = 0;
    for (int ship=0; ship<_number; ship++) if (ship_state[ship]<_hull+1) alive++;
    return (alive);
}

vector<int> Ship::takeHits (int state, Damage damage) {
    vector<int> output (0);

    vector<int> initial_ship_state = stateToShipState (state);

    // create organizer that will range all damage allocation among ship of our type
    SingleDamageOrganizer damage_organizer (damage, _number);
    ClockIterator clock_iterator = damage_organizer.createClockIterator ();
    int total_states = damage_organizer.totalStates ();

    for (int _=0; _<total_states; _++) {
        vector<int> damage_taken = damage_organizer.readDamage (clock_iterator);

        vector<int> new_ship_state (_number);
        for (int ship=0; ship<_number; ship++) new_ship_state[ship] = min(initial_ship_state[ship]+damage_taken[ship],_hull+1);

        int new_state = shipStateToState (new_ship_state);

        if (DEBUG) {
            cout << "takeHits: initial state + damage = new state. hull=" <<_hull << endl;
            for (int ship=0; ship<_number; ship++) cout << initial_ship_state [ship] << " + " << damage_taken[ship] << " = " << new_ship_state[ship] <<endl;
            cout << "state transition: " << state << "->" << new_state << endl;
        }

        
        
        output.push_back (new_state);

        clock_iterator.increment ();
    }
    // clean up and remove duplicates
    sort                              (output.begin(), output.end());
    vector<int>::iterator last= unique(output.begin(), output.end());
    output.erase                      (last          , output.end());
    return output;
}

StateNPCWrapper Ship::takeNPCHits (int state, Damage damage) {
    StateNPCWrapper output;
    // find all possible states
    vector<int> possible_states = takeHits (state, damage);
    unsigned long int max_score=0;
    int best_state=-1;
    for (int i=0; i<int(possible_states.size()); i++) {
        int state = possible_states[i];
        vector<int> ship_state = stateToShipState (state);
        // NPC score is nb of dead ships times a big number, + damage taken, everything time a value that grows the bigger  the ship type
        // killing big ships > killing small ships > damaging big ships > damaging small ships
        int dead_ships = 0;
        int damage_taken = 0;
        for (int ship=0; ship<_number; ship++) {
            if (ship_state[ship]<_hull+1) damage_taken+=ship_state[ship];
            else dead_ships++;
        }
        unsigned long int score = (dead_ships*DEAD_SHIP+damage_taken)*_type;

        if (score>=max_score) {
            max_score=score;
            best_state=state; 
        }
    }
    output._npc_score = max_score;
    output._state = best_state;
    return output;
}

vector<RollUnallocated> Ship::listRolls (int nb_firing_ships, int which_weapon, std::vector<int> shields) {
    // range all possible results of dice using a clock like system
    vector<RollUnallocated> rolls; // output, should be a vector with all possible rolls

    // step 1: see which result of die hit or miss
    // shields MUST be in decreasing order 
    int nb_shields = shields.size();

    std::vector<int> how_many_faces_hit (nb_shields, 1); //there is always at least 1 chance out of 6 to hit
    for (int shield=0; shield<nb_shields; shield++) {
        for (int face=0; face<4; face++) {
            int die_result=2+face+_computer-shields[shield];
            if (die_result>=6) how_many_faces_hit[shield]+=1;
        }
    }

    Weapons weapons;
    if (which_weapon==CANONS) weapons =   _canons.totalDice(nb_firing_ships);
    else                      weapons = _missiles.totalDice(nb_firing_ships);
    
    DiceOrganizer dice_organizer (weapons, how_many_faces_hit);
    ClockIterator clock_iterator = dice_organizer.createClockIterator ();
    int total_states = dice_organizer.totalStates ();

    float total_proba =0.0; //for DEBUG, should be almost equal to 1.0

    // the following algorithm will convert those misses into all possible combination of results in a clock like manner

    bool finished = false;

    for (int _=0; _<total_states; _++) {
    //while (finished==false) {
        // the DiceClock class does all the heavy work
        RollUnallocated roll = dice_organizer.toRollUnallocated (clock_iterator);

        total_proba += roll._proba;

        rolls.push_back (roll);

        if (DEBUG) cout << roll.toString () << endl;

        // increment clock
        finished = clock_iterator.increment ();
    }
    if (DEBUG) cout << "finished="<<finished<< " Total proba= " <<total_proba <<endl;
    
    return rolls;
}