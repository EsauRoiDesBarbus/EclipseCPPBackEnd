//////////////////////////////////////////////////
// functions to read battle data in vector form //
//////////////////////////////////////////////////
#ifndef VECTOR_READING_HPP
#define VECTOR_READING_HPP

#include "ship.hpp"
#include "ship_battle_states.hpp"
#include "bellman_algorithm.hpp"

#include <array>
#include <memory>

#define SHIP_VECTOR_SIZE 16
#define MODIFIER_VECTOR_SIZE 2
#define BATTLE_VECTOR_SIZE 116 // should be equal to 7*SHIP_VECTOR_SIZE + 2*MODIFIER_VECTOR_SIZE
#define RESULT_VECTOR_SIZE 33 

std::shared_ptr<Ship> vectorToShipPointer (std::array<int, SHIP_VECTOR_SIZE>, int type);

std::shared_ptr<ShipBattleStates> vectorToShipBattleStatesPointer (std::array<int, BATTLE_VECTOR_SIZE> battle_vector, time_t timeout);

std::array<float, RESULT_VECTOR_SIZE> resultToVector (BattleResult result, std::shared_ptr<ShipBattleStates> battle_states);

#endif