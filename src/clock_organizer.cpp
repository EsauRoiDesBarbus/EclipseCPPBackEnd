#include "clock_organizer.hpp"

#include <iostream>

using namespace std;

#define DEBUG false

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
        while (counter>=_cells_per_bound[bound]){
            bound++;
            counter=0;
        } // this should robust to when cells_per_bound contains multiple 0 (but it shouldn't have that)
        counter++;
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

void ClockOrganizer::setBounds (vector <int> bounds, vector <int>cells_per_bound) {
    // basically the initialization of the ClockOrganizer class
    _bounds = bounds;
    _cells_per_bound=cells_per_bound;

    initializePascalTriangle ();
}

void ClockOrganizer::initializePascalTriangle () {
    if (DEBUG) cout << "initializePascalTriangle\n";
    // find bounds on triangle
    int nb_bounds = _bounds.size ();
    int max_bound=0;
    int max_cells_per_bound=0;
    for (int bound=0; bound<nb_bounds; bound++){
        max_bound = max (max_bound, _bounds[bound]);
        max_cells_per_bound = max (max_cells_per_bound, _cells_per_bound[bound]);
    }
    //build triangle
    _pascal_triangle.resize (1+max_cells_per_bound); //first index is i, second is value of ai
    _pascal_triangle[0] = vector <int> (1+max_cells_per_bound+max_bound, 1);
    for (int cell=1; cell<1+max_cells_per_bound; cell++) {
        int bounds_to_range = 1+max_cells_per_bound+max_bound-cell;
        _pascal_triangle[cell].resize (bounds_to_range);
        _pascal_triangle[cell][0]=1;
        for (int bound=1; bound<bounds_to_range; bound++){
            _pascal_triangle[cell][bound] = _pascal_triangle[cell-1][bound]+_pascal_triangle[cell][bound-1];
        }
    }
}

int totalStatesBound (int bound, int nb_cells) {
    // returns how many combination of (a0, a1, ...) s.t. sum ai ≤ A. bound = A, nb_cells = n = number of ai
    // the answer is newton coefficient (A+n ; A), that is (A+n)!/( A! n!) which can be computed by the for loop below
    int total_states = 1;
    for (int cell=1; cell<=nb_cells; cell++) {
        total_states*=bound+cell;
        total_states/=cell; //total_states should always be a multiple of cell because of mathemagic
    }
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

int vectorToIndex (vector<int> vec, int bound, const vector<vector<int>>& pascal_triangle) {
    if (DEBUG) cout << "vectorToIndex\n"; 
    // compute the index of (a0, a1, ...) with given ai and A by riding pascal's triangle
    // if an =1, then the other indexes went over (A+n-1 ; A) combination, if an=2, they went over (A+n-1 ; A) + (A+n-2 ; A-1) and so on
    // this simplifies as index = (A+n ; A) - sum (A+i-sum ai; A-1-sum ai) - (A+1 -sum ai ; A-sum ai), the last term is equal to -(A1-sum ai +1)
    int nb_cells = vec.size(); //start at last index
    int index = pascal_triangle [nb_cells][bound]; //(A+n ; A)
    int sum_coord = 0;
    for (int cell=nb_cells-1; cell>0; cell--) {
        sum_coord += vec[cell];
        index-= pascal_triangle [cell+1][bound-1-sum_coord]; //(A+i-sum ai; A-1-sum ai)
    }
    sum_coord += vec[0];
    //index -= pascal_triangle [1][bound-sum_coord];
    index -= bound -sum_coord+1;
    if (DEBUG) {
        cout << "bound=" << bound << " vec=";
        for (int i=0; i<nb_cells; i++) cout << vec[i] <<",";
        cout << " index="<<index<<endl;
    }
    return index;
}

int ClockOrganizer::iterationToIndex (vector <int> iteration) {
    if (DEBUG) cout << "iterationToIndex\n";
    int index = 0;
    int nb_bounds = _bounds.size ();
    int nb_cells = iteration.size ();
    int end = nb_cells;
    for (int bound=nb_bounds-1; bound>=0; bound--) {
        index*=totalStatesBound (_bounds[bound], _cells_per_bound[bound]);
        int start = end-_cells_per_bound[bound];
        vector<int> values_corresponding_to_that_bound (iteration.begin()+start, iteration.begin()+end);
        index+=vectorToIndex (values_corresponding_to_that_bound, _bounds[bound], _pascal_triangle);
        end = start;
    }
    return index;
}

vector<int> indexToVector (int index, int bound_minus_sum_coord, int nb_cells, const vector<vector<int>>& pascal_triangle) {
    if (DEBUG) cout << "indexToVector\n";
    vector<int> output(nb_cells, 0); 
    // compute the index of (a0, a1, ...) with given ai and A by riding pascal's triangle
    // if an =1, then the other indexes went over (A+n-1 ; A) combination, if an=2, they went over (A+n-1 ; A) + (A+n-2 ; A-1) and so on
    int cell = nb_cells-1;
    while (index > 0) {
        if (index>=pascal_triangle[cell][bound_minus_sum_coord]) {
            index-=pascal_triangle[cell][bound_minus_sum_coord];
            output[cell]++;
            bound_minus_sum_coord--; // increased the sum of coordinates by 1
        } else cell--; //go to lower part of vector
    }
    return output;
}

vector <int> ClockOrganizer::indexToIteration (int index) {
    if (DEBUG) cout << "indexToIteration\n";
    int nb_bounds = _bounds.size ();
    vector<int> iteration (0);
    for (int bound=0; bound<nb_bounds; bound++) {
        int total_states = totalStatesBound (                _bounds[bound], _cells_per_bound[bound]);
        vector<int> vec = indexToVector (index%total_states, _bounds[bound], _cells_per_bound[bound], _pascal_triangle);
        iteration.insert (iteration.end(), vec.begin(), vec.end ());
        index/= total_states;
    }
    return iteration;
}




////////////////////////
// ClockOrganizerTest //
////////////////////////
// print iterations, in order, as well as the result of indexToIteration and iterationToIndex
void ClockOrganizerTest::iterationTest (std::vector <int> bounds, std::vector <int> cells_per_bound, bool spam_cout) {
    //initialize ClockOrganizer
    setBounds (bounds, cells_per_bound);

    ClockIterator clock_iterator = createClockIterator();
    int total_states = totalStates ();
    cout << "ClockOrganizer test\n";
    bool all_match = true;
    for (int state=0; state<total_states; state++) {
        bool match = true;
        vector <int> iteration_1 = readData (clock_iterator, ITERATION);
        vector <int> iteration_2 = indexToIteration (state);

        // test that everything matches
        if (state!=iterationToIndex (iteration_1)) match = false;
        for (int i=0; i <int(iteration_1.size()); i++) if (iteration_1[i]!=iteration_2[i]) match = false;
        // check two ways conversion of index
        if ((match==false)or(spam_cout)) {
            cout << "state=" << state << "<->" <<  iterationToIndex (iteration_1);
            // check if iteration match
            cout << " iteration =";
            for (int i=0; i <int(iteration_1.size()); i++) cout << iteration_1[i] << ",";
            cout << "<->";
            for (int i=0; i <int(iteration_2.size()); i++) cout << iteration_2[i] << ",";
            cout <<endl;
        }

        if (match==false) all_match = false;

        clock_iterator.increment ();
    }
    if (all_match) cout << "no problem in ClockOrganizer detected\n";

}
