from eclipseCpp_interface import *


int = Ship (3, "INT", 3, 0, 0, 0, [1,0,0,0,0], [0,0,0,0,0]) # base INT
cru = Ship (2, "CRU", 2, 1, 1, 0, [1,0,0,0,0], [0,0,0,0,0]) # base CRU
dre = Ship (1, "DRE", 1, 2, 1, 0, [2,0,0,0,0], [0,0,0,0,0]) # base DRE

battle = Battle ([int, dre], BattleModifier(), [cru], BattleModifier())

print (battle.solveBattle())
print (vectorToResult(battle._result_vector, battle))