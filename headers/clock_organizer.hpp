//////////////////////////////////////////////////////////////////////////////
// class to order and range states, dice, damages, everything really        //
// it's called clock because it works like a clock, nothing to do with time //
//////////////////////////////////////////////////////////////////////////////

// Range all possible solutions of the problem

// given integers A, B, C
// range all integer vectors (a0, a1, a2..., b0, b1... , c0, c1....) such that sum ai ≤ A, sum bi ≤ B, sum ci ≤ C...
// use case 1 : A is the number of yellow dice, a0 are the number of hits, a1 the number of partial hits
// use case 2 : A is the hull of a ship type, a0 is how much damage the 1st ship took, a1 is how much MORE damage the second ship took...
// note that in all cases, we don't know how many ai there are, and there might be a different number of bi


#ifndef CLOCK_ORGANIZER_HPP
#define CLOCK_ORGANIZER_HPP

#include <vector>

#define ITERATION 1 //to access ClockIterator data
#define REMAINDER 0

class ClockIterator {
    // a state of our clock is 
    private: //this is private because any modification to one vector would break the array
    std::vector <int> _iteration; // (a0, a1...., b0, b1...)
    std::vector <int> _remainder; // (A - sum ai, B - sum bi...)
    std::vector <int> _cells_per_bound; // first value is how many ai there, second is how many bi...

    // constructor
    ClockIterator (const std::vector <int>& bounds, const std::vector <int>& cells_per_bound);

    bool increment (); //go to next iteration, output is whether we went back to initial state

    // all operations on ClockIterator are handled by ClockOrganizer
    friend class ClockOrganizer;
};

class ClockOrganizer {
    private:
    std::vector <int> _bounds;
    std::vector <int> _cells_per_bound;

    protected:
    void setBounds (std::vector <int> bounds, std::vector <int>cells_per_bound) {_bounds = bounds; _cells_per_bound=cells_per_bound;}

    public:
    int totalStates ();
    
    ClockIterator& createClockIterator ();

    int iterationToIndex (std::vector <int>); //returns the index number corresponding to a given iteration vector

    std::vector <int> readData (const ClockIterator& clock_iterator, int b) const { //returns a copy of data
        if (b==ITERATION) return clock_iterator._iteration;
        else              return clock_iterator._remainder;
    }
};

#endif