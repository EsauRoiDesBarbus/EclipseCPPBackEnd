import ctypes
import numpy as np

####################
# LOAD C++ LIBRARY #
####################
lib = ctypes.CDLL('./eclipseCpp.so')

# Define the argument and return types of the function
lib.solveEclipseBattle.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_int), ctypes.c_int]
lib.solveEclipseBattle.restype = None

def solveEclipseBattle(input_array, timeout):
    # Prepare output array
    output_array = np.zeros(34, dtype=np.float32)
    output_size = ctypes.c_int()

    input_size = 116

    # Call the function
    lib.solveEclipseBattle(input_array.ctypes.data_as(ctypes.POINTER(ctypes.c_int)), input_size, output_array.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), ctypes.byref(output_size), timeout)

    # Get the result
    return output_array[:output_size.value]


#####################
# Ship python class #
#####################
class Ship:
    def __init__ (self, number, type, initiative, hull, computer, shield, canons, missiles):
        self._number = number
        self._type = type
        self._initiative = initiative
        self._hull = hull
        self._computer = computer
        self._shield = shield
        self._canons = canons
        self._missiles = missiles
        self._regen = 0

    def toVector (self):
        # writes the ship as a vector for the neural network
        # type is not written here as it is implicit as the position in the battle vector
        vec = [self._number, self._initiative, self._hull, self._computer, self._shield, self._regen]
        vec+= [self._canons[0]  ,self._canons[1]  ,self._canons[2]  ,self._canons[3]  ,self._canons[4]]
        vec+= [self._missiles[0],self._missiles[1],self._missiles[2],self._missiles[3],self._missiles[4]]
        return vec
    
    def toString (self):
        response = str(self._number)+" "
        if   (self._type=="INT"):
            response+="interceptor"
        elif (self._type=="CRU"):
            response+="cruiser"
        elif (self._type=="DRE"):
            response+="dreadnought"
        elif (self._type=="SBA"):
            response+="starbase"
        else:
            response+="ship"
        response+= (self._number>1)*"s" + " with "+str(self._initiative)+" initiative, "
        if (self._hull>0):
            response +=     str(self._hull)+" hull, "
        if (self._computer>0):
            response += '+'+str(self._computer)+" computer, "
        if (self._shield>0):
            response += '-'+str(self._shield)+" shield, "
        colors = ["yellow", "orange", "blue", "red", "pink"]
        for i in range (5):
            if self._canons[i]>0:
                response += str(self._canons[i])+' '+ colors[i] + " canon"    +(self._canons[i]>1)*"s" +", "
        for i in range (5):
            if self._missiles[i]>0:
                response += str(self._missiles[i])+' '+ colors[i] + " missile"  +(self._missiles[i]>1)*"s" +", "
        return (response[:-2]) #remove the last space and ,

def emptyShip ():
    return Ship(0, "INT", 0, 0, 0, 0, [0,0,0,0,0], [0,0,0,0,0])

def vectorToShip (vec, type="SHIP"):
    # converts a vector to a ship
    ship = Ship(vec[0], type, vec[1], vec[2], vec[3], vec[4], vec[6:11], vec[11:16])
    ship._regen = vec[5]
    return ship


#########################
# Modifier python class #
#########################
class BattleModifier:
    def __init__ (self, is_npc=0, antimatter_splitter=0):
        # saves battle modifiers as 0 (False) and 1 (True)
        if (is_npc == True)or(is_npc == 1):
            self._is_npc = 1
        else:
            self._is_npc = 0
        if (antimatter_splitter == True)or(antimatter_splitter == 1):
            self._antimatter_splitter = 1
        else:
            self._antimatter_splitter = 0

    def toVector (self):
        # writes the bonus as a vector for the neural network
        return [self._is_npc, self._antimatter_splitter]

#######################
# Battle python class #
#######################
class Battle:
    def __init__ (self, attacker_ships, attacker_bonus, defender_ships, defender_bonus):
        # mirrors the constructor of ShipBattleStates in eclipseCpp
        self._attacker_ships = attacker_ships
        self._defender_ships = defender_ships

        self._attacker_bonus = attacker_bonus
        self._defender_bonus = defender_bonus

    def toVector (self):
        # writes the battle as a vector for the neural network
        vec = []
        for type in ["INT", "CRU", "DRE"]:
            no_ship=True
            for ship in self._attacker_ships:
                if (ship._type == type)and(no_ship):
                    vec+= ship.toVector()
                    no_ship=False
            if (no_ship):
                vec+=emptyShip().toVector()
            
        vec+= self._attacker_bonus.toVector()


        for type in ["INT", "CRU", "DRE", "SBA"]:
            no_ship=True
            for ship in self._defender_ships:
                if (ship._type == type)and(no_ship):
                    vec+= ship.toVector()
                    no_ship=False
            if (no_ship):
                vec+=emptyShip().toVector()

        vec+= self._defender_bonus.toVector()

        return vec
    
    def resultToVector (self):
        # writes the battle result as a vector for the neural network
        #  0   : calculation time,
        #  1   : attacker win chance,
        #  2- 9: attacker INT survival chance,
        # 10-13: attacker CRU survival chance,
        # 14-15: attacker DRE survival chance,
        # 16-23: defender INT survival chance,
        # 24-27: defender CRU survival chance,
        # 28-29: defender DRE survival chance,
        # 30-33: defender SBA survival chance.
        attacker_place_in_vec ={
            "INT": 2,
            "CRU": 10,
            "DRE": 14
        }
        defender_place_in_vec ={
            "INT": 16,
            "CRU": 24,
            "DRE": 28,
            "SBA": 30
        }
        vec = [0 for i in range (34)]
        vec[0] = self._calculation_time
        vec[1] = self._result["attacker_win_chance"]
        
        for ship_id in range(len(self._attacker_ships)):
            place_in_vec = attacker_place_in_vec[self._attacker_ships[ship_id]._type]
            survival_chance = self._result["attacker_ship_survival_chance"][ship_id]
            for i in range (len(survival_chance)):
                vec[place_in_vec+i] = survival_chance[i]

        for ship_id in range(len(self._defender_ships)):
            place_in_vec = defender_place_in_vec[self._defender_ships[ship_id]._type]
            survival_chance = self._result["defender_ship_survival_chance"][ship_id]
            for i in range (len(survival_chance)):
                vec[place_in_vec+i] = survival_chance[i]

        return vec
    
    def toString (self):
        response = "Attacker:\n"
        for ship in self._attacker_ships:
            response += ship.toString() + "\n"
        response+= "Defender:\n"
        for ship in self._defender_ships:
            response += ship.toString() + "\n"
        return response
    
    def signature (self):
        # use injective hash function on toVector to detect and remove duplicates
        return hash(tuple(self.toVector()))
    
    def solveBattle (self, timeout=60):
        # check for inconsistencies that would cause the C function to fail
        if self.consistencyCheck()!="OK":
            return self.consistencyCheck()
        
        input_array = np.array(self.toVector(), dtype=np.int32)
        self._result_vector = solveEclipseBattle (input_array, timeout)

        # check for timeout TODO
        if self._result_vector[0]<0:
            return 'Timeout'
        else:
            return 'OK'
    
    def consistencyCheck (self):
        # check that there is no duplicate ship in the battle, not too many ships of a type and no unknown type
        max_number = {
            "INT": 8,
            "CRU": 4,
            "DRE": 2,
            "SBA": 4
        }
        # check attacker
        other_types = []
        for ship in self._attacker_ships:
            if   (ship._type not in ["INT", "CRU", "DRE"]): # check for other types
                return "Attacker has unknown ship type"
            elif (ship._type in other_types): # check for duplicates
                return "Attacker has multiple instances of the same ship type"
            else:
                other_types.append(ship._type)
                if (ship._number>max_number[ship._type]): # check for too many ships
                    return "A ship type has too many ships"
        # check defender
        other_types = []
        for ship in self._attacker_ships:
            if   (ship._type not in ["INT", "CRU", "DRE", "SBA"]): # check for other types
                return "Defender has unknown ship type"
            elif (ship._type in other_types): # check for duplicates
                return "Defender has multiple instances of the same ship type"
            else:
                other_types.append(ship._type)
                if (ship._number>max_number[ship._type]): # check for too many ships
                    return "A ship type has too many ships"

        return "OK"
    
    def errorCheck (self):
        # check if the battle was solved correctly
        if isinstance(self._result["attacker_win_chance"], float):
            if (self._result["attacker_win_chance"]>1.000001)or(self._result["attacker_win_chance"]<-0.000001):
                return True
            else:
                return False
        else:
            return True
    
def vectorToResult (vec, battle):
    # converts a vector to a battle result
    # need a battle as argument to know which ship types are in the battle
    result = {}
    result["attacker_win_chance"] = vec[0]
    result["attacker_ship_survival_chance"] = []
    result["defender_ship_survival_chance"] = []
    for ship in battle._attacker_ships:
        if ship._type=="INT":
            result["attacker_ship_survival_chance"].append(vec[ 1: 1+ship._number])
        if ship._type=="CRU":
            result["attacker_ship_survival_chance"].append(vec[ 9: 9+ship._number])
        if ship._type=="DRE":
            result["attacker_ship_survival_chance"].append(vec[13:13+ship._number])
    for ship in battle._defender_ships:
        if ship._type=="INT":
            result["defender_ship_survival_chance"].append(vec[15:15+ship._number])
        if ship._type=="CRU":
            result["defender_ship_survival_chance"].append(vec[23:23+ship._number])
        if ship._type=="DRE":
            result["defender_ship_survival_chance"].append(vec[27:27+ship._number])
        if ship._type=="SBA":
            result["defender_ship_survival_chance"].append(vec[29:29+ship._number])
    return result

def vectorToBattle (vec):
    # converts a vector to a ship
    attacker_ships = []
    defender_ships = []
    if vec[0]>0:
        attacker_ships.append(vectorToShip(vec[ 0:16], "INT"))
    if vec[16]>0:
        attacker_ships.append(vectorToShip(vec[16:32], "CRU"))
    if vec[32]>0:
        attacker_ships.append(vectorToShip(vec[32:48], "DRE"))
    attacker_bonus = BattleModifier(vec[48], vec[49])
    if vec[50]>0:
        defender_ships.append(vectorToShip(vec[50:66], "INT"))
    if vec[66]>0:
        defender_ships.append(vectorToShip(vec[66:82], "CRU"))
    if vec[82]>0:
        defender_ships.append(vectorToShip(vec[82:98], "DRE"))
    if vec[98]>0:
        defender_ships.append(vectorToShip(vec[98:114], "SBA"))
    defender_bonus = BattleModifier(vec[114], vec[115])
    return Battle(attacker_ships, attacker_bonus, defender_ships, defender_bonus)

