#ifndef ROLL_HPP
#define ROLL_HPP

#include <vector>
#include <array>
#include <string>
#include <sstream>

struct Roll {
    // roll is defined as a probability, and states it can lead to
    float _proba;
    std::vector<int> _allocations;

    std::string toString () {
        std::stringstream output;
        output<<"p= "<<_proba<<" alloc= ";
        for (int i=0; i< int(_allocations.size());i++) {output<<_allocations[i]; if (i!=int(_allocations.size())-1) output<<","; else output<<".";}
        return output.str();
    }
};

struct Damage {
    std::array <int, 4> _damage_faces; // number of *, number of **, number of ***, number of ****
    
    int& operator[](int i) {return _damage_faces[i];}
    std::string toString () {
        std::stringstream output;
        output<<"("<<_damage_faces[0]<<","<<_damage_faces[1]<<","<<_damage_faces[2]<<","<<_damage_faces[3]<<")";
        return output.str();
    }
};

struct RollUnallocated {
    // unallocated roll is defined as a probability, and the amount of damage to distribute
    float _proba;
    std::vector<Damage> _damages; //first vector is total hits, (second is partial hits (third is partial partial hits)...))
    // full hits are hits that can be applied to all enemy ships, first partial hits are hits that can be applied to all ships except the one with the most shield
    int _self_hits; //for rift canon TODO

    std::string toString () {
        std::stringstream output;
        output<<"p= "<<_proba<<" dam= ";
        for (int i=0; i< int(_damages.size());i++) output<<_damages[i].toString();
        return output.str();
    }
};





#endif
