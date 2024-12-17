#include "single_damage_organizer.hpp"

using namespace std;

SingleDamageOrganizer::SingleDamageOrganizer (Damage damage, int nb_ships): _damage(damage), _nb_ships(nb_ships) {
    initializeClockOrganizer();
}

void SingleDamageOrganizer::initializeClockOrganizer () {
    vector <int> bounds (4);
    for (int i=0; i<4; i++) bounds[i] = _damage[i];
    setBounds (bounds, vector<int>(4, _nb_ships-1));
}

vector<int> SingleDamageOrganizer::readDamage (ClockIterator& clock_iterator) {
    vector<int> output (_nb_ships);
    vector<int> iteration = readData (clock_iterator, ITERATION);
    vector<int> remainder = readData (clock_iterator, REMAINDER);
    
    output[0]=remainder[0]+2*remainder[1]+3*remainder[2]+4*remainder[3];
    for (int ship=0; ship<_nb_ships-1; ship++) {
        output[1+ship]=iteration[                ship]
                    +2*iteration[   _nb_ships-1 +ship]
                    +3*iteration[2*(_nb_ships-1)+ship]
                    +4*iteration[3*(_nb_ships-1)+ship];
    }
    return output;
}