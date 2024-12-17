/////////////////////////////////////////////////////////////
// ClockOrganizer subclass to order and range dice results //
/////////////////////////////////////////////////////////////
#ifndef SSINGLE_DAMAGE_ORGANIZER_HPP
#define SSINGLE_DAMAGE_ORGANIZER_HPP

#include "clock_organizer.hpp"
#include "weapons.hpp"
#include "roll.hpp"

#include <array>
#include <vector>


class SingleDamageOrganizer: public ClockOrganizer { //used for ships of the same type, does not deal with shield discrepancies
    private:
    // structure used to iterate between all states of the battle
    Damage _damage;
    int _nb_ships;

    void initializeClockOrganizer ();

    public:
    // constructor
    SingleDamageOrganizer (Damage, int);

    // all around function
    std::vector<int> readDamage (ClockIterator&); //return the total damage taken by each ship
};

#endif