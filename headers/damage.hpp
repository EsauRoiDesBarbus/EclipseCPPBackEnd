///////////////////////
// struct for damage //
///////////////////////
#ifndef DAMAGE_HPP
#define DAMAGE_HPP

#include <array>
#include <string>
#include <sstream>

struct Damage {
    std::array <int, 4> _damage_faces; // number of *, number of **, number of ***, number of ****
    //int _evaded; // 0 means full hits, 1 means hits that don't hit the ship with most shield, 2 is hits that don't hit the 2 ships with most shield
    
    int& operator[](int i) {return _damage_faces[i];}
    std::string toString () {
        std::stringstream output;
        output<<"("<<_damage_faces[0]<<","<<_damage_faces[1]<<","<<_damage_faces[2]<<","<<_damage_faces[3]<<")";
        return output.str();
    }

    void splitAntimatter () {
        //split each 4 damage die hit into 4 one damage hit
        _damage_faces[0]+= 4*_damage_faces[3];
        _damage_faces[3] = 0;
    }
};

#endif