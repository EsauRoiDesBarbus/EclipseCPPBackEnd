////////////////////////////////////
// helper class to generate graph //
////////////////////////////////////
#ifndef SHIP_HPP
#define SHIP_HPP

#include "clock_organizer.hpp"
#include "roll.hpp"
#include "weapons.hpp"

#include <array>
#include <vector>


// those values are used for NPC score computations
#define INT 1
#define CRU 500
#define DRE 10000
#define SBA 20
#define DEAD_SHIP 1000000

#define CANONS 1
#define MISSILES 0

struct StateNPCWrapper {
    int _state;
    unsigned long int _npc_score; // npc will allocate their damage to maximize that score
};

class Ship: public ClockOrganizer {
    public:
    // stats
    int _number;
    int _type; //INT CRU DRE or SBA
    int _init;
    int _hull;
    int _computer;
    int _shield;

    Weapons _canons;
    Weapons _missiles;

    //constructor
    Ship (int, int, int, int, int, int, Weapons, Weapons);
    Ship (int, int, int, int, int, int, Weapons);

    void initializeClockOrganizer ();

    std::vector<int> iterationToShipState (std::vector<int>);
    std::vector<int> shipStateToIteration (std::vector<int>);

    std::vector<int> stateToShipState (int);
    int shipStateToState (std::vector<int>);

    //function
    // int totalStates (); //return total number of states of that sip, REDUNDANT with ClockOrganizer
    int countLiveShips (int state); //return number of ships that are alive at that state
    std::vector<int> takeHits    (int, Damage); //returns all possible states that can be reached from taking that damage
    StateNPCWrapper  takeNPCHits (int, Damage); //returns the state an NPC would reach and their NPC score (used to find NPC targets)

    bool hasRift () {return (_canons.hasRift());} //return true if the ship has rift weapons
    bool hasCanons () {return (_canons.hasDice());} //return true if the ship has canons

    std::vector<RollUnallocated> listRolls (int nb_ships, int which_weapon, std::vector<int> shields, bool antimatter_splitter=false); //takes the number of ships as argument as it influences results
    // which_weapon is either CANONS or MISSILES

};

#endif