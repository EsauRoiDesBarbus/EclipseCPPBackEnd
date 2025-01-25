
#include "battle_states_mother_class.hpp"
#include "battle_states_simple_class.hpp"
#include "bellman_algorithm.hpp"
#include "ship.hpp"
#include "ship_battle_states.hpp"
#include "vector_reading.hpp"

using namespace std;
#include <iostream>
#include <fstream>
#include <chrono>
#include <sstream>
#include <string>


extern "C" {
    void solveEclipseBattle (int* input_array, int input_size, float* output_array, int* output_size, int timeout) {
        // solves an Eclipse battle
        // the input needs to be an array of integers as defined in README.md TODO
        // the output will be an array of floats as defined in README.md
        if (input_size != BATTLE_VECTOR_SIZE) {
            std::cerr << "Input battle vector should have size " << BATTLE_VECTOR_SIZE << std::endl;
            *output_size = 0;
            return;
        }

        std::array<int, BATTLE_VECTOR_SIZE> battle_vec;
        for (int i = 0; i < BATTLE_VECTOR_SIZE; i++) {
            battle_vec[i] = input_array[i];
        }

        std::shared_ptr<ShipBattleStates> battle_states = vectorToShipBattleStatesPointer(battle_vec, timeout);
        BattleResult result = winChanceAndExpectancyCalculator(*battle_states);

        std::array<float, RESULT_VECTOR_SIZE> result_vec = resultToVector(result, battle_states);
        *output_size = result_vec.size();
        for (int i = 0; i < *output_size; i++) {
            output_array[i] = result_vec[i];
        }
    }
}