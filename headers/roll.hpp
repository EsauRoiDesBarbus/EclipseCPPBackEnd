#ifndef ROLL_HPP
#define ROLL_HPP

#include <vector>
#include <array>

struct Roll {
    // roll is defined as a probability, and states it can lead to
    float _proba;
    std::vector<int> _allocations;
};

struct Damage {
    std::array <int, 4> _damage_faces; // number of *, number of **, number of ***, number of ****
    
    int& operator[](int i) {return _damage_faces[i];}
};

struct RollUnallocated {
    // unallocated roll is defined as a probability, and the amount of damage to distribute
    float _proba;
    std::vector<Damage> _damages; //first vector is total hits, (second is partial hits (third is partial partial hits)...))
    // full hits are hits that can be applied to all enemy ships, first partial hits are hits that can be applied to all ships except the one with the most shield
    int _self_hits; //for rift canon TODO
};





#endif
