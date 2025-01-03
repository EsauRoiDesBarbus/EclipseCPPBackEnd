#ifndef WEAPONS_HPP
#define WEAPONS_HPP

#include <array>
#include <string>
#include <sstream>
struct Weapons {
    std::array <int, 5> _dice; // number of yellow dice, orange dice, blue dice, pink dice

    Weapons (int y, int o, int b, int r, int p): _dice({y,o,b,r,p}) {}
    Weapons (): _dice({0,0,0,0,0}) {}

    int& operator[](int i) {return _dice[i];}

    Weapons totalDice (int nb_ships) {return {nb_ships*_dice[0], nb_ships*_dice[1], nb_ships*_dice[2], nb_ships*_dice[3], nb_ships*_dice[4]};}

    bool hasRift () {return (_dice[4]>0);}

    std::string toString () {
        std::stringstream output;
        output << "["<<_dice[0]<<","<<_dice[1]<<","<<_dice[2]<<","<<_dice[3]<<","<<_dice[4]<<"]";
        return output.str();
    }
};

#endif