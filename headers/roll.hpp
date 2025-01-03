#ifndef ROLL_HPP
#define ROLL_HPP

#include "damage.hpp"

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

struct RollUnallocated {
    // unallocated roll is defined as a probability, and the amount of damage to distribute
    float _proba;
    std::vector<Damage> _damages; //first vector is total hits, (second is partial hits (third is partial partial hits)...))
    // full hits are hits that can be applied to all enemy ships, first partial hits are hits that can be applied to all ships except the one with the most shield
    int _self_hits;

    std::string toString () {
        std::stringstream output;
        output<<"p= "<<_proba<<" dam= ";
        for (int i=0; i< int(_damages.size());i++) output<<_damages[i].toString();
        output << " self hits=" << _self_hits << ".";
        return output.str();
    }

    RollUnallocated selfHitsToRollUnallocated (int ships_with_rift) {
        RollUnallocated output;
        output._proba = -1; //shouldn't be used
        output._self_hits = 0;
        output._damages.resize (ships_with_rift);
        for (int i=0; i<ships_with_rift; i++) output._damages[i] = {0,0,0,0};
        output._damages[0][0] = _self_hits;
        return output;
    }

    void splitAntimatter () {
        //split each 4 damage die hit into 4 one damage hit
        for (int i=0; i<int(_damages.size()); i++) _damages[i].splitAntimatter();
    }
};





#endif
