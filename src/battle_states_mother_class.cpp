#include "battle_states_simple_class.hpp"

#include <sstream>

using namespace std;


string BattleStates::toString () {
    // returns the entire battle as a string
    stringstream output;

    // Iterate through each vector in the outer vector
    for (size_t i = 0; i < _dice_rolls.size(); ++i) {
        output << "state " << to_string(i) << " rolls:";
        // Iterate through each element in the inner vector
        for (size_t j = 0; j < _dice_rolls[i].size(); ++j) {
            output << " chance " << to_string(get<0>(_dice_rolls[i][j])) << " to state ";
            for (size_t k = 0; k < get<1>(_dice_rolls[i][j]).size(); ++k) {
                output << to_string(get<1>(_dice_rolls[i][j])[k]);
                if (k < get<1>(_dice_rolls[i][j]).size() - 1) {
                    output << ",";
                }
            }
        
            if (j < _dice_rolls[i].size() - 1) {
                output << ";";
            }
        }
        if (i < _dice_rolls.size() - 1) {
            output << "\n";
        }
    }

    return output.str();
}