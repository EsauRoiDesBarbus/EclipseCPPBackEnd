#include "bellman_algorithm.hpp"

#include <vector>
#include <iostream>
#include <cmath>

#define DEBUG true

using namespace std;

BattleResult winChanceAndExpectancyCalculator (BattleStates& battle_states) {
    // this algorithm first compute the value (= win chance of the attacker) going backward
    // as well as the optimal damage allocation of each roll
    // then it uses that information to compute expectancy forward

    BattleResult results;

    
    std::vector< //for each state, 
        std::vector< //we roll dice and for each possible result,
            std::tuple<float, int> //we have a probability of the result, plus ONE optimal state to go to
        >
    > optimal_allocations;

    int nb_states = battle_states._who_is_firing.size(); //total number of states


    // backward win chance calculator
    vector<float> win_chance(nb_states, -1.0); //-1 means uninitialized

    int attacker_win_it = battle_states._states_where_attacker_wins.size()-1;
    int defender_win_it = battle_states._states_where_defender_wins.size()-1;
    int bundle_it = battle_states._state_bundles.size ()-1;

    int state = nb_states-1;
    while (state>=0) { //backward loop
    cout << state << endl;
        //if attacker wins, win chance is 1 if defender wins, win chance is 0
        if ((attacker_win_it>=0)&&(state==battle_states._states_where_attacker_wins[attacker_win_it])) {
            attacker_win_it--;
            win_chance [state] = 1.0;
            state--;
            continue;
        }
        if ((defender_win_it>=0)&&(state==battle_states._states_where_defender_wins[defender_win_it])) {
            defender_win_it--;
            win_chance [state] = 0.0;
            state--;
            continue;
        }
        // check if we have a contiguous bundle of states that need to be computed together
        if ((bundle_it>=0)&&(state==get<1>(battle_states._state_bundles[bundle_it]))) {
            // TODO
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

                    } else {
                        //TODO : range all possible states to find max win chance
                        float max_win_chance = win_chance [get<1>(battle_states._dice_rolls[start_state+i][roll])[0]];
                        b[i] = b[i] + proba*max_win_chance;

                    }
                }
            }

            if (DEBUG) {
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
            continue;
        } //TODO : lone state
    }
    if (DEBUG) {
        cout << "win chance =";
        for (int i = 0; i < nb_states; ++i) cout << win_chance[i] << ",";
        cout << endl;
    }

    


    // forward expectancy calculator
    vector<float> expectancy(nb_states, 0.0);




    return results;



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




