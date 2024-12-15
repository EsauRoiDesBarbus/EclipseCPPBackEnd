#ifndef WEAPONS_HPP
#define WEAPONS_HPP

#include <array>

struct Weapons {
    std::array <int, 5> _dice; // number of yellow dice, orange dice, blue dice, pink dice

    int& operator[](int i) {return _dice[i];}

    Weapons totalDice (int nb_ships) {return {nb_ships*_dice[0], nb_ships*_dice[1], nb_ships*_dice[2], nb_ships*_dice[3], nb_ships*_dice[4]};}

    bool hasRift () {return (_dice[4]>0);}
};

#endif