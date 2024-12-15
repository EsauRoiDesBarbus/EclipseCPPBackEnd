//////////////////////////////////////////////
// Default implementation of the ship class //
//////////////////////////////////////////////

// in this implementation, the index is equal to the total number of damage taken by that ship type
// we assume that the opponents will finish a ship before firing on another one of the same type
// this means that damage can't be split between ships of the same type TODO : make a class that can model that

#include "ship.hpp"
#include "dice_organizer.hpp"

#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

#define DEBUG false


Ship::Ship (int n, int t, int i, int h, int c, int s, Weapons can, Weapons mis):
    _number(n), _type(t), _init(i),_hull(h),_computer(c),_shield(s),_canons(can),_missiles(mis) {}

Ship::Ship (int n, int t, int i, int h, int c, int s, Weapons can):
    _number(n), _type(t), _init(i),_hull(h),_computer(c),_shield(s),_canons(can),_missiles({0,0,0,0,0}) {}

int Ship::totalStates () {
    return (_number*(_hull+1)+1);
}

int Ship::countLiveShips (int state) {
    return (_number - state/(_hull+1));
}

vector<int> Ship::takeHits (int state, Damage damage) {
    // in this implementation, we retrun only one state
    // TODO: take overkill into account
    // idea : check if overkill, then check for attribution with 0 overflow, then allocation with 1 overflow etc...
    return {min(state +damage[0] +damage[1]*2 +damage[2]*3 +damage[3]*4, _number*(_hull+1))}; //it's a vector of 1 element
}

StateNPCWrapper Ship::takeNPCHits (int state, Damage damage) {
    StateNPCWrapper output;
    output._state = takeHits (state, damage)[0];
    // NPC score is nb of dead ships times a big number, + damage taken, everything time a value that grows the bigger  the ship type
    // killing big ships > killing small ships > damaging big ships > damaging small ships
    int dead_ships = _number - countLiveShips (output._state);
    int damage_taken = output._state - dead_ships*(_hull+1);
    output._npc_score = (dead_ships*DEAD_SHIP+damage_taken)*_type;
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