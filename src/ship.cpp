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

#define RIFT_RESULTS 4

struct DiceClock {
    // structure used to iterate between all states of the battle
    vector <int>   _all_hits; // yellow hits, (yellow partial hits), orange hits, (orange partial hits), ... rift +3-1, rift +2, rift +1, rif -1
    array <int, 5> _all_miss; // yellow miss, orange miss, ... rift miss
    array <int, 5> _all_dice; // yellow miss, orange miss, ... rift miss

    vector<bool> _same_chance_as_the_one_before; // says wether a ship has the same chance of being hit as the next ship with more shield

    vector<float>    _log_proba_hits;
    array <float, 5> _log_proba_miss;
    vector<float>    _factorial_log_table;


    DiceClock (Weapons& weapons, vector<int>& how_many_faces_hit) {
        // initialize damage clock, and does ground work to later export to RollUnallocated

        int nb_shields = how_many_faces_hit.size ();

        int nb_result_per_die=1; //not counting misses, there is at least 1 total hit result

        _same_chance_as_the_one_before.resize(nb_shields);

        vector<int> chance_of_hits_and_partial_hits (nb_shields, 0);
        int chance_of_miss= 6-how_many_faces_hit[nb_shields-1]; //full miss
        chance_of_hits_and_partial_hits[0] = how_many_faces_hit [0]; //full hit
        _same_chance_as_the_one_before[0] = false;
        for (int shield=1; shield<nb_shields; shield++) {
            chance_of_hits_and_partial_hits[shield] = how_many_faces_hit[shield] - how_many_faces_hit [shield-1]; //partial hit
            if (chance_of_hits_and_partial_hits[shield]!=0){
                _same_chance_as_the_one_before[shield]=false;
                nb_result_per_die++;
            } else {
                _same_chance_as_the_one_before[shield]=true;
            }
        }

        // initialiaze arrays
        int nb_results = 4*nb_result_per_die+RIFT_RESULTS; //each of the 4 damage die has nb_result_per_die possible results (depends on enemy shields), and rift canon has 4 possible results
        _all_hits.resize       (nb_results, 0); 
        _log_proba_hits.resize (nb_results, 0); 
        for (int die=0; die<5; die++) _all_dice[die]=weapons[die];
        _all_miss = _all_dice; //by default all dice are misses

        // compute and store log of probabilities for probability computations
        // Log and exp are a way of avoiding numerical errors from multiplying very small numbers with very large number

        // compute proba of each hit and miss for regular dice
        for (int die=0; die<4; die++) {
            _log_proba_miss[die] = log(chance_of_miss) - log(6);
            int result=0;
            for (int shield=0; shield < nb_shields; shield++) { //we iterate on ships but skip those that have the same chance of being hit as the one before
                if (_same_chance_as_the_one_before[shield]==false) {
                    _log_proba_hits[die*nb_result_per_die+result]= log(chance_of_hits_and_partial_hits[shield]) - log(6);
                    result++;
                }
            }
        }
        // compute proba of each hit and miss for rift dice
        int die = 4;
        _log_proba_miss[die] = log(2) - log(6); //the rift die has 2 chance out of 6 to miss
        for (int result=0; result<RIFT_RESULTS; result++)
            _log_proba_hits[die*nb_result_per_die+result]= log(1) - log(6); //each rift result has 1 chance out of 6

        // creates and saves a table containing ln(0!) to ln(n!) to speed up probability computations
        // step 1 : find highest number of dice as it the size of the table we need
        int max_dice = 0;
        for (int i=0; i<5; i++){
            max_dice = max(max_dice, weapons[i]);
        }
        // step 2 : create table
        _factorial_log_table.resize (max_dice+1);
        _factorial_log_table[0]=0.0; _factorial_log_table[1]=0.0; //log (0!) = log(1!) = 0.0
        for (int i=2; i<max_dice+1; i++) _factorial_log_table[i] = _factorial_log_table[i-1] + log (i);
    }

    float computeProba () {
        // compute probability of roll
        // Each die type is independant, so the proba is the product of each die proba, hence log proba is the sum
        float log_proba = 0.0; //using exp and log to reduce numerical errors
        for (int die=0; die<5; die++) {
            log_proba +=  _factorial_log_table[_all_dice[die]]; //ln(nb_dice!)
            int misses = _all_miss[die];
            log_proba += -_factorial_log_table[misses] + misses*_log_proba_miss[die];
        }
        int nb_results = _all_hits.size ();
        for (int result=0; result < nb_results; result++) {
            int hits = _all_hits[result];
            log_proba += -_factorial_log_table[hits] + hits*_log_proba_hits[result]; //-ln(nb_hit!) + nb_hit*ln(proba_hit)
        }
        return exp(log_proba);
    }

    RollUnallocated toRollUnallocated () {
        RollUnallocated output;
        // compute and store proba
        output._proba = computeProba ();

        // compute and store damage allocations
        int nb_shields = _same_chance_as_the_one_before.size ();
        int nb_results = _all_hits.size ();
        int nb_result_per_die = (nb_results-RIFT_RESULTS)/4;
        
        // do full hits, by taking normal dice and rift dice
        int die_result=0;
        Damage dam({_all_hits[                    die_result],
                    _all_hits[nb_result_per_die  +die_result],
                    _all_hits[nb_result_per_die*2+die_result],
                    _all_hits[nb_result_per_die*3+die_result]});
        // 1st cell of rift dice is +3 -1, 2nd is +2, 3rd is +1, 4th is -1
        int before_rift = 4*nb_result_per_die;
        dam[2]+= _all_hits[before_rift  ];
        dam[1]+= _all_hits[before_rift+1];
        dam[0]+= _all_hits[before_rift+2];
        output._self_hits = _all_hits[before_rift  ]+_all_hits[before_rift+3]; //the two results of rift that do self hits

        output._damages.push_back(dam);

        // partial hits with normal dice
        die_result=1; // start die_result and shield at 1 because we already did full hits
        for (int shield=1; shield<nb_shields; shield++) {
            if (_same_chance_as_the_one_before[shield]==false) {
                Damage dam({_all_hits[                    die_result],
                            _all_hits[nb_result_per_die  +die_result],
                            _all_hits[nb_result_per_die*2+die_result],
                            _all_hits[nb_result_per_die*3+die_result]});
                output._damages.push_back(dam);
                die_result++;
            } else {
                Damage dam({0,0,0,0});
                output._damages.push_back(dam);
            }
        }

        return output;
    }

    bool increment () {
        int nb_results = _all_hits.size ();
        int nb_result_per_die = (nb_results-RIFT_RESULTS)/4;
        for (int result=0; result <nb_results; result++) {
            int die = min(result/nb_result_per_die, 4); // this is a quick and dirty way that works even with rift canon fixed 4 results
            if (_all_miss[die] >= 1) {
                // transfer one miss to that result
                _all_hits[result]++;
                _all_miss[die]--;
                return false; //we haven't done a full clock turn
            } else {
                // transfer back all those outcomes to misses (and go to next outcome)
                _all_miss[die]=_all_hits[result];
                _all_hits[result]=0;
            }
        }
        return true; //we have done a full clock turn
    }
};


Ship::Ship (int n, int t, int i, int h, int c, int s, Weapons can, Weapons mis):
    _number(n), _type(t), _init(i),_hull(h),_computer(c),_shield(s),_canons(can),_missiles(mis) {}

Ship::Ship (int n, int t, int i, int h, int c, int s, Weapons can):
    _number(n), _type(t), _init(i),_hull(h),_computer(c),_shield(s),_canons(can),_missiles({0,0,0,0,0}) {}

int Ship::totalStates () {
    return (_number*(_hull+1)+1);
}

int Ship::countLiveShips (int state) {
    return (_number - state/(_hull+1));
}

vector<int> Ship::takeHits (int state, Damage damage) {
    // in this implementation, we retrun only one state
    // TODO: take overkill into account
    // idea : check if overkill, then check for attribution with 0 overflow, then allocation with 1 overflow etc...
    return {min(state +damage[0] +damage[1]*2 +damage[2]*3 +damage[3]*4, _number*(_hull+1))}; //it's a vector of 1 element
}

StateNPCWrapper Ship::takeNPCHits (int state, Damage damage) {
    StateNPCWrapper output;
    output._state = takeHits (state, damage)[0];
    // NPC score is nb of dead ships times a big number, + damage taken, everything time a value that grows the bigger  the ship type
    // killing big ships > killing small ships > damaging big ships > damaging small ships
    int dead_ships = _number - countLiveShips (output._state);
    int damage_taken = output._state - dead_ships*(_hull+1);
    output._npc_score = (dead_ships*DEAD_SHIP+damage_taken)*_type;
    return output;
}

vector<RollUnallocated> Ship::listRolls (int nb_firing_ships, int which_weapon, std::vector<int> shields) {
    // range all possible results of dice using a clock like system
    vector<RollUnallocated> rolls; // output, should be a vector with all possible rolls

    // step 1: see which result of die hit or miss
    // shields MUST be in decreasing order 
    int nb_shields = shields.size();

    std::vector<int> how_many_faces_hit (nb_shields, 1); //there is always at least 1 chance out of 6 to hit
    for (int shield=0; shield<nb_shields; shield++) {
        for (int face=0; face<4; face++) {
            int die_result=2+face+_computer-shields[shield];
            if (die_result>=6) how_many_faces_hit[shield]+=1;
        }
    }

    Weapons weapons;
    if (which_weapon==CANONS) weapons =   _canons.totalDice(nb_firing_ships);
    else                      weapons = _missiles.totalDice(nb_firing_ships);
    
    DiceClock dice_clock (weapons, how_many_faces_hit);

    float total_proba =0.0; //for DEBUG, should be almost equal to 1.0

    // the following algorithm will convert those misses into all possible combination of results in a clock like manner
    bool finished = false;
    while (finished==false) {
        // the DiceClock class does all the heavy work
        RollUnallocated roll = dice_clock.toRollUnallocated ();

        total_proba += roll._proba;

        rolls.push_back (roll);

        if (DEBUG) cout << roll.toString () << endl;

        // increment clock
        finished = dice_clock.increment ();
    }
    if (DEBUG) cout << "Total proba= " <<total_proba <<endl;
    
    return rolls;

}