# EclipseCPPBackEnd
Battle calculator for Eclipse with Bellman equation programming, but in C++ this time

# Compiling
- make: builds the entire project (except python library) and creates the executable "main", which is a sequence of tests.
- make exe: builds runs main afterward.
- make pythonmodule: builds a python module called eclipseCpp. YOU NEED TO HAVE BOOST INSTALLED, you may also need to change its location in makefile.
- make clean: deletes everything except source code.

# Architecture
This project is an algorithm to solve Eclipse battles.
Input: list of ships and battle modifiers of the attacker, list of ships and battle modifiers of the defender.
Output: win chance of the attacker *(=1-win chance of the defender)*, survival chance of each ship.
If a given ship type has multiple ships, the survival chance of that ship will be an array, the first float is the survival chance of the last ship to die, the second float is the survival chance of the second to last to die, and so on.
The first float can also be interpreted as the probability of finishing with 1 ship or more, the second float is the chance of finishing with 2 ships or more and so on...

Flow:
1. User uses python module eclipseCpp to input battle info.
2. Battle info is used to create a ShipBattleStates object, which is a graph of all possible states of the battle.
3. A Bellman value function algorithm is run on that ShipBattleStates object to compute win chance and ship survival chances.

Here is a file by file overview of the project, which was developed in reverse order.


## Calculating win chance
The calculator part of the algorithm is almost agnostic of the context of Eclipse, to reduce maintenance.

### bellman_algorithm
Contains the algorithm that computes win chance and survival chance of ships.
Input: BattleStates object, e.g. a graph of the battle.
Output: BattleResult object, e.g. a container for attacker win chance and survival chance of each ship *(the output of the overall algorithm)*.

The algorithm works by first propagating state win chance backward on that graph, then propagating state win chance forward.

### battle_states_mother_class
Defines the BattleStates class.
BattleStates contains the minimum required to run the bellman value function algorithm.
Each state is an index, with 0 corresponding to the initial state of the battle.
Most attributes are vectors containing relevant info of each state.
The _dice_rolls attribute contains the edges of the graph, each corresponding to a roll of dice.

There are battle states that are interdependent, typically because a given canon round will loop on itself if every ship misses.
Those battle states form contiguous bundles in the state graph, and the _state_bundle attribute lists all those blocks, giving the 1st and last state index of each bundle. 

BattleStates has multiple subclasses :
- test subclasses defined in battle_states_simple_class.
- ShipBattleStates, which transform actual Eclipse battles into usable battle states graphs.

### battle_states_simple_class
Defines two hard coded instances of BattleStates to test the bellman algorithm


## Creating the battle graph
The hardest part of the calculator is building the battle state graph from the battle info.
To avoid having ShipBattleStates doing all the heavy work, some of it is offloaded to the Ship class.
Structures like Damage and Weapons are mostly here to improve readability by grounding the code in the context of Eclipse.

### weapons
Defines the Weapons struct, which is an array containing the number of yellow dice, orange dice, blue dice, red dice and pink dice.

### damage
Defines the Damage struct, which is an array containing the number of instance of 1s, 2s, 3s, and 4s dealt.
*(Since dice cannot be split in Eclipse, each result must be kept separate.)*
Damage has a member function splitAntimatter for when a player has the AntimatterSplitter tech.

### roll
Defines the Roll and RollUnallocated structs.
Both correspond to a given dice roll, and they both contain the probability of the roll they represent.
RollUnallocated is the raw damage of that roll: the number of hits, partial hits, and misses. Those hits have not yet been *allocated* to opposing ships.
Roll is after that damage has been allocated, it lists all battle states that roll could lead to.

Since Roll only makes sense within a given BattleStates object, it is the ShipBattleStates class that can "allocate rolls", that it transform UnallocatedRolls into Rolls.

### ship
defines the Ship class.
Ship is used to store all info of an Eclipse ship *type* *(3 interceptors should be represented as a single Ship object with _number=3)*.
Ship has attributes for each battle stat: number, type, initiative, hull, computer, shield, canons and missiles (those last two are Weapons objects).

Ship is also responsible for ordering all possible *ship states* of that particular ship.
Ship states are all possible combination of hit points values of the multiple ships of that particular ship type *(note that ship states are different from battle states)*.
To that end, it inherits from ClockOrganizer and has member functions to map ClockOrganizer iterations to ship states.

Ship helps ShipBattleStates with the following member functions:
- listRolls: returns a list of UnallocatedRolls corresponding to each possible rolls of their weapons.
- takeHits: returns all possible ship state a given Damage can lead to.
- takeNPCHits: returns *the only* ship state a given Damage can lead to if it were allocated by a NPC, and returns an NPC score that can be used to find the best allocation of damage among different ship types.

The last 2 functions use a SingleDamageOrganizer to range all possible allocation of that damage.

### ship_battle_states
Defines the ShipBattleStates class.

ShipBattleStates inherits from ClockOrganizer to map battle states (=integers) to and ExtendedState, which is an array (round, state of ship 1, state of ship 2...).

ShipBattleStates consists mostly in a list of initialization functions that are called in a sequence during construction.
1. sort ships by initiative, shield, and rift canon for later,
2. initialize ClockOrganizer,
3. initialize battle state info,
4. initialize graph edges.

To initialize graph edges, ShipBattleStates ranges all states.
At each state, it will collect the RollsUnallocated of the currently firing ship *(using Ship.listRolls)*.
For each roll, it deals self hits caused by rift canons *(using Ship.takeNPCHits)* and then ranges all possible allocation of that damage among opposing ships *(using a locally defined DamageClock struct)*.
For each allocation, it asks each Ship that has taken damage to list their new states *(using Ship.takeHits or Ship.takeNPCHits)*.
Then if a player is firing, it lists and saves all possible battle states caused by all possible allocations in a Roll object *(thus "allocating" a RollUnallocated object)*; if a NPC is firing, it will only save the one state that maximizes the NPC score in the Roll object.

It also creates the _state_bundle attribute using round numbers.


## Helper
To further offload complexity from ShipBattleStates and Ship, multiple helper classes are used.

### clock_organizer
Defines the ClockIterator and ClockOrganizer classes.

Those object are used to order all integer vectors (a0, a1, a2..., b0, b1..., c0, c1....) such that sum ai ≤ A, sum bi ≤ B, sum ci ≤ C...
ClockIterator can iterate on those integer vectors, in a clock-like manner: it increments a0, unless sum ai = A, in which case it sets a0 to 0 and increments a1, and so on.
ClockOrganizer can map a given vector to its index and vice versa.

This particular problem comes up surprisingly often in this project, hence the following classes inherit from ClockOrganizer 
- ShipBattleStates,
- Ship,
- DiceOrganizer,
- SingleDamageOrganizer.

### single_damage_organizer
Defines the SingleDamageOrganizer class.

This is used to range all possible allocation of damage among ships of the same type, using ClockOrganizer.

### dice_organizer
Defines the DiceOrganizer class.

This is used to range all possible results of dice, using ClockOrganizer.
The difficulty here is that the number of possible results varies in Eclipse: a discrepancy of shield among ship types in a fleet leads to "partial hits", that is, hits that can be dealt to some ships but not the others, and there may be between 0 and 3 types of partial hits as a fleet contains up to 4 ships that may or may not have different shield values.

## Python Module
This calculator is meant to be used from Python rather than C++.

### expose_to_python
Exposes C++ code to Python using the BOOST library.

For now it only provides a BattleBuilder class, that, when instanced, allows a python user to enter ships, one at a time, as well as battle modifiers, and can then compute the battle result using the above C++ algorithm.

Call BattleBuilder.help() for details.

