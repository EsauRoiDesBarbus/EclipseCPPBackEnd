#include "dice_organizer.hpp"

#include <cmath>
#include <iostream>

#define DEBUG false

using namespace std;

//////////////////////////////////////
// Definition of Rift canon results //
//////////////////////////////////////
array <array<int, 2>, 4> riftHits () {
    array <array<int, 2>, 4> rift_hits;
    rift_hits[0][0] = 3; rift_hits[0][1] = 1;
    rift_hits[1][0] = 2; rift_hits[1][1] = 0;
    rift_hits[2][0] = 1; rift_hits[2][1] = 0;
    rift_hits[3][0] = 0; rift_hits[3][1] = 1;
    return rift_hits;
}


/////////////////////////////////
// DiceOrganizer : Constructor //
/////////////////////////////////
DiceOrganizer::DiceOrganizer (Weapons& weapons, vector<int>& how_many_faces_hit) {
    if (DEBUG) cout << "DiceOrganizer constructor\n";
    // initialize the entire class

    _nb_shields = how_many_faces_hit.size ();

    // convert how_many_faces_hit into relevant info
    _nb_result_per_die=1; //not counting misses, there is at least 1 total hit result

    _same_chance_as_the_one_before.resize(_nb_shields);

    vector<int> chance_of_hits_and_partial_hits (_nb_shields, 0);
    int chance_of_miss= 6-how_many_faces_hit[_nb_shields-1]; //full miss
    chance_of_hits_and_partial_hits[0] = how_many_faces_hit [0]; //full hit
    _same_chance_as_the_one_before[0] = false;
    for (int shield=1; shield<_nb_shields; shield++) {
        chance_of_hits_and_partial_hits[shield] = how_many_faces_hit[shield] - how_many_faces_hit [shield-1]; //partial hit
        if (chance_of_hits_and_partial_hits[shield]!=0){
            _same_chance_as_the_one_before[shield]=false;
            _nb_result_per_die++;
        } else {
            _same_chance_as_the_one_before[shield]=true;
        }
    }

    for (int die=0; die<5; die++) _all_dice[die]=weapons[die]; //initialize _all_dice

    int max_dice = 0;
    for (int i=0; i<5; i++){
        max_dice = max(max_dice, weapons[i]);
    }

    initializeProbaTables (chance_of_miss, chance_of_hits_and_partial_hits);
    initializeFactorialLogTable (max_dice);

    initializeClockOrganizer ();
}

void DiceOrganizer::initializeClockOrganizer () {
    if (DEBUG) cout << "initializeClockOrganizer\n";
    // initialiaze arrays
    vector <int> bounds(5); //number of dice of each type
    for (int die=0; die<5; die++) bounds[die] = _all_dice[die];
    vector <int> cells_per_bound(5);
    for (int die=0; die<4; die++) cells_per_bound[die]=_nb_result_per_die;
    cells_per_bound[4]=NB_RIFT_RESULTS;

    setBounds (bounds, cells_per_bound);
}


void DiceOrganizer::initializeProbaTables (int chance_of_miss, vector<int> chance_of_hits_and_partial_hits) {
    if (DEBUG) cout << "initializeProbaTables\n";
    // compute and store log of probabilities for probability computations
    // Log and exp are a way of avoiding numerical errors from multiplying very small numbers with very large number
    _log_proba_hits.resize (4*_nb_result_per_die+NB_RIFT_RESULTS);
    // compute proba of each hit and miss for regular dice
    for (int die=0; die<4; die++) {
        _log_proba_miss[die] = log(chance_of_miss) - log(6);
        int result=0;
        for (int shield=0; shield < _nb_shields; shield++) { //we iterate on ships but skip those that have the same chance of being hit as the one before
            if (_same_chance_as_the_one_before[shield]==false) {
                _log_proba_hits[die*_nb_result_per_die+result]= log(chance_of_hits_and_partial_hits[shield]) - log(6);
                result++;
            }
        }
    }
    // compute proba of each hit and miss for rift dice
    int die = 4;
    _log_proba_miss[die] = log(2) - log(6); //the rift die has 2 chance out of 6 to miss
    for (int result=0; result<NB_RIFT_RESULTS; result++)
        _log_proba_hits[die*_nb_result_per_die+result]= log(1) - log(6); //each rift result has 1 chance out of 6
}

void DiceOrganizer::initializeFactorialLogTable (int max_dice) {
    if (DEBUG) cout << "initializeFactorialLogTable\n";
    // creates and saves a table containing ln(0!) to ln(n!) to speed up probability computations
    _factorial_log_table.resize (max_dice+1);
    _factorial_log_table[0]=0.0; _factorial_log_table[1]=0.0; //log (0!) = log(1!) = 0.0
    for (int i=2; i<max_dice+1; i++) _factorial_log_table[i] = _factorial_log_table[i-1] + log (i);
}

///////////////////////////////////////////////////
// DiceOrganizer : convertion to RollUnallocated //
///////////////////////////////////////////////////
float DiceOrganizer::computeProba (ClockIterator& clock_iterator) {
    if (DEBUG) cout << "computeProba\n";
    // compute probability of roll
    vector<int> all_hits = readData (clock_iterator, ITERATION); // yellow hits, (yellow partial hits), orange hits, (orange partial hits), ... rift 0, rift 1, rift 2, rift 3
    vector<int> all_miss = readData (clock_iterator, REMAINDER); // yellow miss, orange miss, ... rift miss
    // Each die type is independant, so the proba is the product of each die proba, hence log proba is the sum
    float log_proba = 0.0; //using exp and log to reduce numerical errors
    for (int die=0; die<5; die++) {
        log_proba +=  _factorial_log_table[_all_dice[die]]; //ln(nb_dice!)
        int misses = all_miss[die];
        log_proba += -_factorial_log_table[misses] + misses*_log_proba_miss[die];
    }
    int nb_results = all_hits.size ();
    for (int result=0; result < nb_results; result++) {
        int hits = all_hits[result];
        log_proba += -_factorial_log_table[hits] + hits*_log_proba_hits[result]; //-ln(nb_hit!) + nb_hit*ln(proba_hit)
    }
    return exp(log_proba);
}

RollUnallocated DiceOrganizer::toRollUnallocated (ClockIterator& clock_iterator) {
    if (DEBUG) cout << "toRollUnallocated\n";
    RollUnallocated output;
    // compute and store proba
    output._proba = computeProba (clock_iterator);

    vector<int> all_hits = readData (clock_iterator, ITERATION); // yellow hits, (yellow partial hits), orange hits, (orange partial hits), ... rift 0, rift 1, rift 2, rift 3
    vector<int> all_miss = readData (clock_iterator, REMAINDER); // yellow miss, orange miss, ... rift miss

    // compute and store damage allocations
    
    // do full hits, by taking normal dice and rift dice
    int die_result=0;
    Damage dam({all_hits[                     die_result],
                all_hits[_nb_result_per_die  +die_result],
                all_hits[_nb_result_per_die*2+die_result],
                all_hits[_nb_result_per_die*3+die_result]});
    // add rift hits (which are always full hits)
    array <array<int, 2>, 4> rift_hits = riftHits ();

    output._self_hits = 0;

    int before_rift = 4*_nb_result_per_die;
    for (int hit=0; hit<NB_RIFT_RESULTS; hit++) {
        dam[rift_hits[hit][0]]+= all_hits[before_rift+hit];
        output._self_hits += rift_hits[hit][1]*all_hits[before_rift+hit];
    }
    output._damages.push_back(dam);

    // partial hits with normal dice
    die_result=1; // start die_result and shield at 1 because we already did full hits
    for (int shield=1; shield<_nb_shields; shield++) {
        if (_same_chance_as_the_one_before[shield]==false) {
            Damage dam({all_hits[                     die_result],
                        all_hits[_nb_result_per_die  +die_result],
                        all_hits[_nb_result_per_die*2+die_result],
                        all_hits[_nb_result_per_die*3+die_result]});
            output._damages.push_back(dam);
            die_result++;
        } else {
            Damage dam({0,0,0,0});
            output._damages.push_back(dam);
        }
    }
    return output;
}