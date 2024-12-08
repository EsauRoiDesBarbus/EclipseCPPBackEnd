//////////////////////////////////////////////
// Default implementation of the ship class //
//////////////////////////////////////////////

// in this implementation, the index is equal to the total number of damage taken by that ship type
// we assume that the opponents will finish a ship before firing on another one of the same type
// this means that damage can't be split between ships of the same type TODO : make a class that can model that


#include "ship.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

#define DEBUG true

Ship::Ship (int n, int t, int i, int h, int c, int s, Weapons can, Weapons mis):
    _number(n), _type(t), _init(i),_hull(h),_computer(c),_shield(s),_canons(can),_missiles(mis) {
    initializeFactorialLogTable();
}

Ship::Ship (int n, int t, int i, int h, int c, int s, Weapons can):
    _number(n), _type(t), _init(i),_hull(h),_computer(c),_shield(s),_canons(can),_missiles({0,0,0,0,0}) {
    initializeFactorialLogTable();
}

int Ship::totalStates () {
    return (_number*(_hull+2));
}

int Ship::countLiveShips (int state) {
    return (_number - state/(_hull+1));
}

vector<int> Ship::takeHits (int state, Damage damage) {
    // in this implementation, we retrun only one state
    // TODO: take overkill into account
    // idea : check if overkill, then check for attribution with 0 overflow, then allocation with 1 overflow etc...
    return {state - damage[0] - damage[1]*2 - damage[2]*3- damage[3]*4}; //it's a vector of 1 element
}



vector<RollUnallocated> Ship::listRolls (int nb_ships, std::vector<int> shields) {
    // range all possible results of dice using a clock like system

    // step 1: see which result of die hit or miss

    // shields MUST be in decreasing order 
    int nb_shields = shields.size();
    int nb_die_results = 1; //not counting misses, there is at least 1 total hit result

    std::vector<int> how_many_faces_hit (nb_shields, 1); //there is always at least 1 chance out of 6 to hit
    for (int shield=0; shield<nb_shields; shield++) {
        for (int face=0; face<4; face++) {
            int die_result=2+face+_computer-shields[shield];
            if (die_result>=6) how_many_faces_hit[shield]+=1;
        }
    }
    std::vector<int> chance_of_hits_and_partial_hits (nb_shields+1, 0);
    std::vector<bool> same_chance_as_the_one_before (nb_shields);
    chance_of_hits_and_partial_hits[0] = 6-how_many_faces_hit[nb_shields-1]; //full miss
    chance_of_hits_and_partial_hits[1] = how_many_faces_hit [0]; //full hit
    for (int shield=1; shield<nb_shields; shield++) {
        chance_of_hits_and_partial_hits[shield+1] = how_many_faces_hit[shield] - how_many_faces_hit [shield-1]; //partial hit
        if (chance_of_hits_and_partial_hits[shield+1]!=0){
            same_chance_as_the_one_before[shield]=false;
            nb_die_results++;
        } else {
            same_chance_as_the_one_before[shield]=true;
        }
    }
    

    vector<RollUnallocated> rolls; // output, should be a vector with all possible rolls

    
    int nb_outcomes_4_dice = 4*nb_die_results;

    vector<int> all_outcomes (nb_outcomes_4_dice, 0); // * hits, (* partial hits), ** hits, (** partial hits), *** hits etc...

    array <int, 5> total_dice = _canons.totalDice (nb_ships); //by default we consider each die a miss
    array <int, 5> total_misses = total_dice;

    

    // compute the log of the probability of each outcome
    vector<float> proba_result_log(1+nb_die_results);//first element is miss
    
    proba_result_log[0]= log(chance_of_hits_and_partial_hits[0]) - log(6);
    int result =1;
    for (int shield=0; shield < nb_shields; shield++) {
        if (same_chance_as_the_one_before[shield]==false) {
            proba_result_log[result]= log(chance_of_hits_and_partial_hits[shield+1]) - log(6);
            result++;
        }
    }

    float total_proba =0.0; //for DEBUG, should be almost equal to 1.0

    // the following algorithm will convert those misses into all possible combination of results in a clock like manner
    bool finished = false;
    while (finished==false) {
        // compute probability of roll
        // Each die type is independant, so the proba is the product of each die proba, hence log proba is the sum
        float log_proba = 0.0; //using exp and log to reduce numerical errors
        for (int die=0; die<4; die++) {
            log_proba += _factorial_log_table[total_dice[die]]; //ln(nb_dice!)
            for (int result=0; result < nb_die_results; result++) {
                int hits = all_outcomes[nb_die_results*die+result];
                log_proba += -_factorial_log_table[hits] + hits*proba_result_log[1+result]; //-ln(nb_hit!) + nb_hit*ln(proba_hit)
            }
            int misses = total_misses[die];
            log_proba += -_factorial_log_table[misses] + misses*proba_result_log[0];
        }
            
        float proba = exp(log_proba);

        RollUnallocated roll;
        roll._proba = proba;

        total_proba += proba;

        int die_result=0;
        for (int shield=0; shield<nb_shields; shield++) {
            if (same_chance_as_the_one_before[shield]==false) {
                Damage dam({all_outcomes[                 die_result],
                            all_outcomes[nb_die_results  +die_result],
                            all_outcomes[nb_die_results*2+die_result],
                            all_outcomes[nb_die_results*3+die_result]});
                roll._damages.push_back(dam);
                die_result++;
            } else {
                Damage dam({0,0,0,0});
                roll._damages.push_back(dam);
            }
        }

        rolls.push_back (roll);


        if (DEBUG) { // print current outcomes
            cout << "(";
            for (int outcome=0; outcome<nb_outcomes_4_dice; outcome++) cout << all_outcomes[outcome] << ",";
            cout << ")";
            cout << " roll damage=";
            int nb_damages = roll._damages.size ();
            for (int i=0; i<nb_damages; i++) cout<<" ("<<roll._damages[i][0]<<","<<roll._damages[i][1]<<","<<roll._damages[i][2]<<","<<roll._damages[i][3]<<")";
            cout << " proba= " << roll._proba << endl;
        }


        // increment clock
        finished = true;
        for (int outcome=0; outcome <nb_outcomes_4_dice; outcome++) {
            int die = outcome/nb_die_results;
            if (total_misses[die] >= 1) {
                // transfer one miss to that outcome
                all_outcomes[outcome]++;
                total_misses[die]--;
                finished = false;
                break;
            } else {
                // transfer back all those outcomes to misses (and go to next outcome)
                total_misses[die]=all_outcomes[outcome];
                all_outcomes[outcome]=0;
            }
        }
        
        
    }
    if (DEBUG) cout << "Total proba= " <<total_proba <<endl;
    
    return rolls;

}


void Ship::initializeFactorialLogTable () {
    // creates and saves a table containing ln(0!) to ln(n!) to speed up probability computations
    // Log and exp are a way of avoiding numerical errors from multiplying very small numbers with very large number
    // step 1 : find highest number of dice as it the size of the table we need
    int max_dice = 0;
    for (int i=0; i<5; i++){
        max_dice = max(max_dice,   _canons[i]);
        max_dice = max(max_dice, _missiles[i]);
    }
    max_dice*=_number;
    // step 2 : crate table
    _factorial_log_table.resize (max_dice+1); //log (0!) = log(1!) = 0.0
    for (int i=2; i<max_dice+1; i++) _factorial_log_table[i] = _factorial_log_table[i-1] + log (i);
}