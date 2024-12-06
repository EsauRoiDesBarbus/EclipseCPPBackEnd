#include "bellman_algorithm.hpp"

#include <vector>
#include <iostream>
#include <cmath>
#include <sstream>

#define DEBUG false

using namespace std;

BattleResult winChanceAndExpectancyCalculator (BattleStates& battle_states) {
    // this algorithm first compute the value (= win chance of the attacker) going backward
    // as well as the optimal damage allocation of each roll
    // then it uses that information to compute expectancy forward

    BattleResult results;
    int nb_states = battle_states._who_is_firing.size(); //total number of states

    //store optimal allocation of each roll in a vector
    std::vector< //for each state, 
        std::vector< //we roll dice and for each possible result,
            int //we have ONE optimal state to go to
        >
    > optimal_allocations (battle_states._dice_rolls.size ());
    for (int state =0; state<nb_states;state++) optimal_allocations [state].resize (battle_states._dice_rolls[state].size());

    


    // backward win chance calculator
    vector<float> win_chance(nb_states, -1.0); //-1 means uninitialized


    int nb_attacker_wins = battle_states._states_where_attacker_wins.size();
    int nb_defender_wins = battle_states._states_where_defender_wins.size();
    int nb_bundles = battle_states._state_bundles.size ();

    int attacker_win_it = nb_attacker_wins-1;
    int defender_win_it = nb_defender_wins-1;
    int bundle_it = nb_bundles-1;

    int state = nb_states-1;
    while (state>=0) { //backward loop
        //if attacker wins, win chance is 1 if defender wins, win chance is 0
        if        ((attacker_win_it>=0)&&(state==battle_states._states_where_attacker_wins[attacker_win_it])) {
            attacker_win_it--;
            win_chance [state] = 1.0;
            state--;
        } else if ((defender_win_it>=0)&&(state==battle_states._states_where_defender_wins[defender_win_it])) {
            defender_win_it--;
            win_chance [state] = 0.0;
            state--;
        } else if ((bundle_it>=0)&&(state==get<1>(battle_states._state_bundles[bundle_it]))) {
            // contiguous bundle of states that need to be computed together
            int start_state = get<0>(battle_states._state_bundles[bundle_it]);
            int close_state = get<1>(battle_states._state_bundles[bundle_it]);
            int bundle_size = close_state - start_state+1;

            //write problem as LP Ax = b
            vector<vector<float>> A (bundle_size, vector<float>(bundle_size, 0.0));
            vector<float> b(bundle_size, 0.0);

            for (int i=0; i<bundle_size; ++i) A[i][i] = 1.0; //identity diagonal

            for (int i=0; i<bundle_size; ++i) {
                int nb_rolls = battle_states._dice_rolls[start_state+i].size();
                for (int roll=0;roll<nb_rolls; roll++){
                    float proba = get<0>(battle_states._dice_rolls[start_state+i][roll]);
                    if (get<1>(battle_states._dice_rolls[start_state+i][roll])[0]<=close_state){
                        // this state loops to another state of the bundle, must be written in A
                        int j = get<1>(battle_states._dice_rolls[start_state+i][roll])[0]-start_state;
                        A[i][j] = A[i][j] - proba;

                        optimal_allocations [start_state+i][roll] = get<1>(battle_states._dice_rolls[start_state+i][roll])[0]; //there should only be one state anyway

                    } else {
                        //range all possible states to find max win chance
                        int best_allocation = findBestAllocation (battle_states._who_is_firing[start_state+i], get<1>(battle_states._dice_rolls[start_state+i][roll]), win_chance);
                        float max_win_chance = win_chance[best_allocation];

                        b[i] = b[i] + proba*max_win_chance;

                        optimal_allocations [start_state+i][roll] = best_allocation; // store best allocation for expectancy
                    }
                }
            }

            if (DEBUG) {
                cout << "state = "<< state << endl;

                cout << "b =";
                for (int i = 0; i < bundle_size; ++i) cout << b[i] << ",";
                cout << endl;

                cout << "A =";
                for (int i = 0; i < bundle_size; ++i) for (int j = 0; j < bundle_size; ++j) cout << A[i][j] << ",";
                cout << endl;
            }

            vector<float> x = solveLP (A, b);
            for (int i=0; i<bundle_size; ++i) {
                win_chance [start_state+i] = x[i];
            }

            // go over all states in the bundle
            state = start_state-1;
            bundle_it--;
        } else {
            // state that can be computed alone (e.g. round of missiles)
            float state_win_chance = 0.0;
            int nb_rolls = battle_states._dice_rolls[state].size();
            for (int roll=0;roll<nb_rolls; roll++){
                float proba = get<0>(battle_states._dice_rolls[state][roll]);

                int best_allocation = findBestAllocation (battle_states._who_is_firing[state], get<1>(battle_states._dice_rolls[state][roll]), win_chance);
                float max_win_chance = win_chance[best_allocation];
                
                state_win_chance = state_win_chance + proba*max_win_chance;

                optimal_allocations [state][roll] = best_allocation; // store best allocation for expectancy
            }
            win_chance [state] = state_win_chance;
            state--;
        }
    }
    if (DEBUG) {
        cout << "win chance =";
        for (int i = 0; i < nb_states; ++i) cout << win_chance[i] << ",";
        cout << endl;
    }

    results._attacker_win_chance = win_chance[0];

    


    // forward expectancy calculator
    int nb_type_ships = battle_states._live_ships[0].size();
    results._ship_survival_chance.resize (nb_type_ships);
    for (int i=0; i<nb_type_ships; ++i) results._ship_survival_chance[i].resize (battle_states._live_ships[0][i]); //give the initial number of ships


    vector<float> expectancy(nb_states, 0.0);
    expectancy[0] = 1.0; // battle start at state 0

    attacker_win_it = 0;
    defender_win_it = 0;
    bundle_it = 0;
    state = 0;
    while (state<nb_states) {
        //if attacker wins, win chance is 1 if defender wins, win chance is 0
        if        ((attacker_win_it<nb_attacker_wins)&&(state==battle_states._states_where_attacker_wins[attacker_win_it])) {
            attacker_win_it++;
            // add surviving ships to results
            for (int i=0; i<nb_type_ships; ++i) {
                int ships_of_that_type_alive = battle_states._live_ships[state][i];
                for (int j=0; j<ships_of_that_type_alive; ++j) results._ship_survival_chance[i][j] += expectancy[state];
            }
            state++;
        } else if ((defender_win_it<nb_defender_wins)&&(state==battle_states._states_where_defender_wins[defender_win_it])) {
            defender_win_it++;
            // add surviving ships to probability
            for (int i=0; i<nb_type_ships; ++i) {
                int ships_of_that_type_alive = battle_states._live_ships[state][i];
                for (int j=0; j<ships_of_that_type_alive; ++j) results._ship_survival_chance[i][j] += expectancy[state];
            }
            state++;
        } else if ((bundle_it<nb_bundles)&&(state==get<0>(battle_states._state_bundles[bundle_it]))) {
            // contiguous bundle of states that need to be computed together
            int start_state = get<0>(battle_states._state_bundles[bundle_it]);
            int close_state = get<1>(battle_states._state_bundles[bundle_it]);
            int bundle_size = close_state - start_state+1;

            // step 1 compute expectancy in the bundle by writing it as LP Ax = b
            vector<vector<float>> A (bundle_size, vector<float>(bundle_size, 0.0));
            vector<float> b(bundle_size, 0.0);

            for (int i=0; i<bundle_size; ++i) {
                A[i][i] = 1.0; //identity diagonal
                b[i] = expectancy [start_state+i];
            }

            for (int i=0; i<bundle_size; ++i) {
                int nb_rolls = battle_states._dice_rolls[start_state+i].size();
                for (int roll=0;roll<nb_rolls; roll++){
                    int best_allocation = optimal_allocations [start_state+i][roll];
                    if (best_allocation<=close_state){
                        // this state loops to another state of the bundle, must be written in A
                        float proba = get<0>(battle_states._dice_rolls[start_state+i][roll]);
                        int j = best_allocation-start_state;
                        A[j][i]+= -proba; //the formula of expectancy is the other way around 
                    } 
                }
            }

            if (DEBUG) {
                cout << "state = "<< state << endl;
                
                cout << "b =";
                for (int i = 0; i < bundle_size; ++i) cout << b[i] << ",";
                cout << endl;

                cout << "A =";
                for (int i = 0; i < bundle_size; ++i) for (int j = 0; j < bundle_size; ++j) cout << A[i][j] << ",";
                cout << endl;
            }

            vector<float> x = solveLP (A, b);
            for (int i=0; i<bundle_size; ++i) {
                expectancy [start_state+i] = x[i];
            }

            // propagate expectancy outside the bundle
            for (int i=0; i<bundle_size; ++i) {
                int nb_rolls = battle_states._dice_rolls[start_state+i].size();
                for (int roll=0;roll<nb_rolls; roll++){
                    int best_allocation = optimal_allocations [start_state+i][roll];
                    if (best_allocation>close_state){
                        float proba = get<0>(battle_states._dice_rolls[start_state+i][roll]);
                        
                        expectancy[best_allocation]+= proba*expectancy[start_state+i];
                    } 
                }
            }
            state = close_state+1;
            bundle_it++;
        } else {
            // propagate expectancy of states
            int nb_rolls = battle_states._dice_rolls[state].size();
            for (int roll=0;roll<nb_rolls; roll++){
                float proba = get<0>(battle_states._dice_rolls[state][roll]);

                int best_allocation = optimal_allocations [state][roll];
                
                expectancy[best_allocation]+= proba*expectancy[state];
            }
            state++;
        }
    }
    if (DEBUG) {
        cout << "expectancy =";
        for (int i = 0; i < nb_states; ++i) cout << expectancy[i] << ",";
        cout << endl;
    }


    return results;
}

int findBestAllocation (int sign, std::vector<int>& allocations, std::vector<float>& win_chance) {
    
    //range all possible states to find max win chance. if sign = 1, then we maximize win chance for the attacker, if sign = -1 we minimize it
    int best_allocation = allocations[0];
    float max_win_chance = sign*win_chance [best_allocation];
    int nb_allocations = allocations.size ();
    for (int r=1; r<nb_allocations; r++) {
        int allocation = allocations[r];
        float allocation_win_chance = sign*win_chance [allocation];
        if (allocation_win_chance>max_win_chance) {
            max_win_chance = allocation_win_chance;
            best_allocation = allocation;
        }
    }
    return best_allocation;

}

string BattleResult::toString () {
    stringstream output;

    output << "Attacker win chance = " << _attacker_win_chance << ", surviving ship probability= ";
    for (int i=0; i<_ship_survival_chance.size (); ++i){
        for (int j=0; j<_ship_survival_chance[i].size (); ++j) output << _ship_survival_chance[i][j] << ", ";
    }
    
    return output.str ();
}


// Function to perform Gaussian Elimination by ChatGPT
vector<float> solveLP(vector<vector<float>>& A, vector<float>& b) {
    int n = A.size();
    
    // Augment the matrix A with vector b (forming an augmented matrix [A|b])
    for (int i = 0; i < n; i++) {
        A[i].push_back(b[i]);
    }
    
    // Perform Forward Elimination
    for (int i = 0; i < n; i++) {
        // Find the pivot (the largest element in the current column)
        float maxEl = fabs(A[i][i]);
        int maxRow = i;
        for (int k = i + 1; k < n; k++) {
            if (fabs(A[k][i]) > maxEl) {
                maxEl = fabs(A[k][i]);
                maxRow = k;
            }
        }

        // Swap the maximum row with the current row
        if (maxRow != i) {
            swap(A[maxRow], A[i]);
        }

        // Eliminate the entries below the pivot element
        for (int j = i + 1; j < n; j++) {
            float factor = A[j][i] / A[i][i];
            for (int k = i; k < n + 1; k++) {
                A[j][k] -= A[i][k] * factor;
            }
        }
    }

    // Back Substitution
    vector<float> x(n, 0);
    for (int i = n - 1; i >= 0; i--) {
        x[i] = A[i][n] / A[i][i];
        for (int j = i - 1; j >= 0; j--) {
            A[j][n] -= A[j][i] * x[i];
        }
    }

    return x;
}




