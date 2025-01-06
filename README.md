# EclipseCPPBackEnd
Battle calculator for Eclipse with Bellman equation programming, but in C++ this time

# Compiling
This projet uses a `Makefile` with the following commands:
- `make`: builds the entire project (except python library) and creates the executable `main`, which is a sequence of tests by default.
- `make exe`: builds projetc then runs `main`.
- `make pythonmodule`: builds a python module called `eclipseCpp`. YOU NEED TO HAVE BOOST INSTALLED, you may also need to change its location in `Makefile`.
- `make clean`: deletes everything except source code.

This library can be used, either with the python module, or by editing `src/main.cpp` (and calling `make exe`).

# Architecture
This project is an algorithm to solve Eclipse battles.  
**Input**: list of ships and battle modifiers of the attacker, list of ships and battle modifiers of the defender.  
**Output**: win chance of the attacker, survival chance of each ship.  
It is assumed that stalemates are wins for the defender, hence win chance of the defender is=1-win chance of the defender.  
The survival chance of a ship type is an array, the first float is the survival chance of the last ship of that type to die, which is also the probability of 1 ship of that type or more surviving; the second float is the survival chance of the second to last ship to die, which is also the probability of 2 ships or more surviving; and so on...

Flow:
1. User uses python module eclipseCpp (or edits `src/main.cpp`) to input battle info.
2. Battle info is used to create a `ShipBattleStates` object, which is a stochastic graph modeling the battle.
3. A Bellman value function algorithm is run on that `ShipBattleStates` object to compute win chance and ship survival chances.

Here is a file by file overview of the project, which was developed in reverse order.


## Calculating win chance
The calculator part of the algorithm is almost agnostic of the context of Eclipse, to be easy to test and maintain.

### bellman_algorithm
Contains the algorithm that computes win chance and survival chance of ships.  
**Input**: `BattleStates` object, e.g. a graph of the battle.  
**Output**: `BattleResult` object, e.g. a container for attacker win chance and survival chance of each ship *(the output of the overall algorithm)*.

The Bellman algorithm works by first propagating win chance on the graph backward, that is in the reverse order of the battle, then propagating state win chance forward.

### battle_states_mother_class
Defines the `BattleStates` class.  
`BattleStates` contains the minimum required to run the bellman value function algorithm.
Each state is an index, with 0 corresponding to the initial state, and states being ordered so that graph edges almost always point forward.
Most attributes are vectors containing relevant info of each state.
The `_dice_rolls` attribute contains the stochastic edges of the graph, each corresponding to a roll of dice.

There are battle states that are interdependent, typically because a given canon round will loop on itself if every ship miss.
Those battle states form contiguous bundles in the state graph, and the `_state_bundle` attribute lists all those blocks, giving the 1st and last index of each bundle. 

BattleStates has multiple subclasses :
- test subclasses defined in battle_states_simple_class.
- `ShipBattleStates`, which transform actual Eclipse battles into usable battle states graphs.

### battle_states_simple_class
Defines two hard coded instances of `BattleStates`.  
They are used to unit test the Bellman algorithm.


## Creating the battle graph
The hardest part of the calculator is building the battle state graph from the battle info.
To avoid having `ShipBattleStates` doing all the heavy work, some of it is offloaded to the `Ship` class.
Structures like `Damage` and `Weapons` are mostly here to improve readability by grounding the code in the context of Eclipse.

### weapons
Defines the `Weapons` struct, which is an array containing the number of yellow dice, orange dice, blue dice, red dice and pink dice.

### damage
Defines the `Damage` struct, which is an array containing the number of instance of 1s, 2s, 3s, and 4s dealt.
*(Since dice cannot be split in Eclipse, each result must be kept separate.)*
`Damage` has a member function `splitAntimatter` that is called when a player has the AntimatterSplitter tech.

### roll
Defines the `Roll` and `RollUnallocated` structures.  
Both correspond to a given dice roll, and they both contain the probability of the roll they represent.  
`RollUnallocated` is the raw damage of that roll: the number of hits, partial hits, and misses. Those hits have not yet been *allocated* to opposing ships.
`Roll` is after that damage has been allocated, it lists all possible battle states that dice roll could lead to.

Since `Roll` only makes sense within a given BattleStates object, it is the `ShipBattleStates` class that can "allocate rolls", that it transform `UnallocatedRolls` into `Rolls`.

### ship
defines the `Ship` class.  
`Ship` is used to store all info of an Eclipse ship *type* *(3 interceptors should be represented as a single Ship object with _number=3)*.
`Ship` has attributes for each battle stat: number, type, initiative, hull, computer, shield, canons and missiles (those last two are Weapons objects).

`Ship` is also responsible for ordering all possible *ship states* of that particular ship.
Ship states are all possible combination of hit point values of the multiple ships of that particular ship type *(note that ship states are different from battle states)*.
To that end, it inherits from `ClockOrganizer` and has member functions to map `ClockOrganizer` iterations to ship states.

`Ship` helps `ShipBattleStates` with the following member functions:
- `listRolls`: returns a list of UnallocatedRolls corresponding to each possible rolls of their weapons.
- `takeHits`: returns all possible ship state a given `Damage` can lead to.
- `takeNPCHits`: returns *the only* ship state a given `Damage` can lead to if it were allocated by a NPC, and returns an NPC score that can be used to find the best allocation of damage among different ship types.

The last 2 functions use a `SingleDamageOrganizer` to range all possible allocation of that damage.

### ship_battle_states
Defines the `ShipBattleStates` class.
ShipBattleStates inherits from `ClockOrganizer` to map battle states (=integers) to and `ExtendedState`, which is an array (round, state of ship 1, state of ship 2...). There are two times as many rounds as there are ship types, as each ship has separate canon and missile rounds.

`ShipBattleStates` consists mostly in a list of initialization functions that are called in a sequence during construction.
1. sort ships by initiative, shield, and rift canon for later,
2. initialize `ClockOrganizer`,
3. initialize battle state info,
4. initialize graph edges.

To initialize graph edges *(arguably the most complex part of the projet)*, `ShipBattleStates` ranges all states.
At each state, it will collect the `RollsUnallocated` of the currently firing ship *(using `Ship.listRolls`)*.
For each roll, it deals self hits caused by rift canons *(using `Ship.takeNPCHits`)* and then ranges all possible allocation of that damage among opposing ships *(using a locally defined `DamageClock` struct)*.
For each allocation, it asks each Ship that has taken damage to list their new states *(using `Ship.takeHits` or `Ship.takeNPCHits`)*.
Then if a player is firing, it lists and saves all possible battle states caused by all possible allocations in a Roll object *(thus "allocating" a `RollUnallocated` object)*; if a NPC is firing, it will only save the one state that maximizes the NPC score in the Roll object.

It also creates the `BattleStates._state_bundle` attribute using Eclipse's round structure.


## Helper
To further offload complexity from `ShipBattleStates` and `Ship`, multiple helper classes are used to organize and range states.

### clock_organizer
Defines the `ClockIterator` and `ClockOrganizer` classes.

Those object are used to order all integer vectors: $$(a_0, a_1, a_2\cdots, b_0, b_1\cdots, c_0, c_1\cdots)$$ such that: $$\sum a_i \leq A,\sum b_i \leq B, \sum c_i \leq C\cdots$$  
`ClockIterator` can iterate on those integer vectors, in a clock-like manner *(hence the name)*: it increments $a_0$, unless $\sum a_i = A$, in which case it sets $a_0$ to 0 and increments $a_1$, and so on.
`ClockOrganizer` can map a given vector to its index and vice versa.

This particular problem comes up surprisingly often in this project, hence the following classes inherit from `ClockOrganizer` 
- `ShipBattleStates`,
- `Ship`,
- `DiceOrganizer`,
- `SingleDamageOrganizer`.

### single_damage_organizer
Defines the `SingleDamageOrganizer` class.

This is used to range all possible allocation of damage among ships of the same type, using `ClockOrganizer`.

### dice_organizer
Defines the `DiceOrganizer` class.  
This is used to range all possible results of dice, using `ClockOrganizer`.
The difficulty here is that the number of possible results varies in Eclipse: shield discrepancies within a player fleet leads to "partial hits", that is, hits that can be dealt to some ships but not the others, and there may be between 0 and 3 types of partial hits as a fleet contains up to 4 ships that may or may not have different shield values.

## Python Module
This calculator is meant to be used from Python rather than C++.

### expose_to_python
Exposes C++ code to Python using the BOOST library.

For now it only provides a `BattleBuilder` class, that, when instanced, allows a python user to enter ships, one at a time, as well as battle modifiers, and can then compute the battle result using the above C++ algorithm.

Call `BattleBuilder.help()` for details.

