////////////////////////////////////
// helper class to generate graph //
////////////////////////////////////
#ifndef SHIP_HPP
#define SHIP_HPP

#include "roll.hpp" // for the Roll struct
#include <array>
#include <vector>

#define INT 1
#define CRU 10000
#define DRE 1000000
#define SBA 100

struct Weapons {
    std::array <int, 5> _dice; // number of yellow dice, orange dice, blue dice, pink dice

    int& operator[](int i) {return _dice[i];}

    std::array <int, 5> totalDice (int nb_ships) {return {nb_ships*_dice[0], nb_ships*_dice[1], nb_ships*_dice[2], nb_ships*_dice[3], nb_ships*_dice[4]};}
};

class Ship {
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

    std::vector<float> _factorial_log_table; //used to speed up probability computations

    //constructor
    Ship (int, int, int, int, int, int, Weapons, Weapons);
    Ship (int, int, int, int, int, int, Weapons);

    void initializeFactorialLogTable ();

    //function
    int totalStates (); //return total number of states of that sip (depend on model)
    int countLiveShips (int state); //return number of ships that are alive at that state
    std::vector<int> takeHits (int, Damage); //returns all possible states that can be reached from taking that damage

    std::vector<RollUnallocated> listRolls (int nb_ships, std::vector<int> shields); //takes the number of ships as argument as it influences results

};














#endif