from eclipseCpp import BattleBuilder


test = BattleBuilder()
print (test.help())

test.addShip("ATT", 3, "INT", 3, 1, 2, 0, [1,0,0,0,0], [0,0,0,0,0])
test.addShip("ATT", 2, "CRU", 2, 1, 1, 0, [1,0,0,0,0], [0,0,0,0,0])
test.addShip("DEF", 4, "CRU", 2, 1, 1, 0, [2,0,0,0,0], [0,0,0,0,0])


print (test.solveBattle())

test.addModifier ("DEF", "NPC")

print (test.solveBattle())
print (test.getResult())

