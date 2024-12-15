/////////////////////////////////////////////////////////////
// ClockOrganizer subclass to order and range dice results //
/////////////////////////////////////////////////////////////
#ifndef DICE_ORGANIZER_HPP
#define DICE_ORGANIZER_HPP

#include "clock_organizer.hpp"
#include "weapons.hpp"
#include "roll.hpp"

#include <array>
#include <vector>

#define NB_RIFT_RESULTS 4
std::array <std::array<int, 2>, 4> riftHits (); //returns results of rift dice

class DiceOrganizer: public ClockOrganizer {
    private:
    // structure used to iterate between all states of the battle
    std::array <int, 5> _all_dice; // yellow miss, orange miss, ... rift miss

    int _nb_shields;
    int _nb_result_per_die;
    std::vector<bool> _same_chance_as_the_one_before; // says wether a ship has the same chance of being hit as the next ship with more shield

    void initializeClockOrganizer ();

    // probability
    std::vector<float>    _log_proba_hits;
    std::array <float, 5> _log_proba_miss;
    std::vector<float>    _factorial_log_table;
    void initializeProbaTables (int, std::vector<int>); //takes the number of miss results, and the number of hit results, (and partial hit results)
    void initializeFactorialLogTable (int max_dice);

    float computeProba (ClockIterator&);

    public:
    // constructor
    DiceOrganizer (Weapons& weapons, std::vector<int>& how_many_faces_hit);

    // all around function
    RollUnallocated toRollUnallocated (ClockIterator&);
};

#endif