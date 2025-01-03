from eclipseCpp import battleBuilder, Weapons


test = battleBuilder()
print (test.help())

test.addShip("ATT", 1, "INT", 3, 1, 2, 0, Weapons(1,0,0,0,0), Weapons(0,0,0,0,0))
test.addShip("ATT", 1, "CRU", 2, 1, 1, 0, Weapons(1,0,0,0,0), Weapons(0,0,0,0,0))
test.addShip("DEF", 1, "CRU", 2, 1, 1, 0, Weapons(2,0,0,0,0), Weapons(0,0,0,0,0))


print (test.solveBattle())

test.addModifier ("DEF", "NPC")

print (test.solveBattle())

