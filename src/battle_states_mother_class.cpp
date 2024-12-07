#include "battle_states_simple_class.hpp"

#include <sstream>

using namespace std;


string BattleStates::toString () {
    // returns the entire battle as a string
    stringstream output;

    int nb_states = _dice_rolls.size();

    // Iterate through each vector in the outer vector
    int attacker_win_it = 0;
    int defender_win_it = 0;
    for (int i = 0; i < nb_states; ++i) {
        output << "state " << to_string(i);
        if (i==_states_where_attacker_wins[attacker_win_it]) {
            attacker_win_it++;
            output << " attacker wins\n";
            continue;
        }
        if (i==_states_where_defender_wins[defender_win_it]) {
            defender_win_it++;
            output << " defender wins\n";
            continue;
        }
        output << " rolls:";
        // Iterate through each element in the inner vector
        int nb_rolls = _dice_rolls[i].size();
        for (int j = 0; j < nb_rolls; ++j) {
            output << " chance " << to_string(_dice_rolls[i][j]._proba) << " to state ";
            int nb_possible_states = _dice_rolls[i][j]._allocations.size();
            for (int k = 0; k < nb_possible_states; ++k) {
                output << to_string(_dice_rolls[i][j]._allocations[k]);
                if (k < nb_possible_states - 1) {
                    output << ",";
                }
            }
        
            if (j < nb_rolls - 1) {
                output << ";";
            }
        }
        if (i < nb_states - 1) {
            output << "\n";
        }
    }

    return output.str();
}