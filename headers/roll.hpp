#ifndef ROLL_HPP
#define ROLL_HPP

#include <vector>

struct Roll {
    // each roll is defined as a probability, and states it can lead to
    float _proba;
    std::vector<int> _allocations;
};

#endif
