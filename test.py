from eclipseCpp import battleBuilder


test = battleBuilder()
print (test.help())

test.addShip("ATT", 2, "INT", 3, 0, 0, 0, 1, 0, 0, 0, 0)
test.addShip("DEF", 2, "INT", 3, 0, 0, 0, 1, 0, 0, 0, 0)

print (test.solveBattle())