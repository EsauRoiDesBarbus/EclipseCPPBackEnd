#include "clock_organizer.hpp"

#include <iostream>

using namespace std;

///////////////////
// ClockIterator //
///////////////////

ClockIterator::ClockIterator (const std::vector <int>& bounds, const std::vector <int>& cells_per_bound) {
    _remainder = bounds;
    _cells_per_bound = cells_per_bound;

    int nb_bounds = bounds.size ();
    int nb_cells=0;
    for (int bound=0; bound<nb_bounds; bound++) nb_cells += cells_per_bound[bound];
    
    _iteration = vector <int> (nb_cells, 0);
}

bool ClockIterator::increment () {
    int nb_cells = _iteration.size ();
    int bound=0, counter=0;
    for (int cell=0; cell <nb_cells; cell++) {
        if (counter>=_cells_per_bound[bound]){
            bound++;
            counter=0;
        } else counter++; //note that this is not robust to when cells_per_bound has multiple 0 in a row (but it shouldn't have that)

        if (_remainder[bound]>=1) {
            // transfer 1 from remainder to cell and exit
            _iteration[cell]++;
            _remainder[bound]--;
            return false; // did not go back to initial state
        } else {
            // transfer back all cell to remainder and go on
            _remainder[bound]=_iteration[cell];
            _iteration[cell]=0;
        }
    }
    return true; // went back to initial state
}

////////////////////
// ClockOrganizer //
////////////////////

int totalStatesBound (int bound, int nb_cells) {
    // returns how many combination of (a0, a1, ...) s.t. sum ai ≤ A. bound = A, nb_cells = n = number of ai
    // the answer is newton coefficient (A+n-1 ; A), that is (n+A-1)!/( A! (n-1)!) which can be computed by the for loop below
    int total_states = 1;
    for (int cell=1; cell<=nb_cells; cell++) total_states*=(bound-1+cell)/cell;
    return total_states;
}

int ClockOrganizer::totalStates () {
    int nb_bounds = _bounds.size ();
    int total_states = 1;
    for (int bound=0; bound<nb_bounds; bound++) total_states*= totalStatesBound (_bounds[bound], _cells_per_bound[bound]);
    return total_states;
}

ClockIterator ClockOrganizer::createClockIterator () {
    ClockIterator iterator(_bounds, _cells_per_bound);
    return iterator;
}

int indexOfVector (vector <int> vector, int bound) {
    // compute the index of (a0, a1, ...) with given ai and A
    return vector[0]; //works for vector of size 1 TODO : vector of higher size
}

int ClockOrganizer::iterationToIndex (vector <int> iteration) {
    //TODO
    int index = 0;
    int nb_bounds = _bounds.size ();
    int nb_cells = iteration.size ();
    int end = nb_cells;
    for (int bound=nb_bounds-1; bound>=0; bound--) {
        index*=totalStatesBound (_bounds[bound], _cells_per_bound[bound]);
        int start = end-_cells_per_bound[bound];
        vector<int> values_corresponding_to_that_bound (iteration.begin()+start, iteration.begin()+end);
        index+=indexOfVector (values_corresponding_to_that_bound, bound);
        end = start;
    }
    return index;
}

