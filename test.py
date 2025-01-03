from eclipseCpp import battleBuilder, Weapons


test = battleBuilder()
print (test.help())

test.addShip("ATT", 2, "INT", 3, 0, 0, 0, Weapons(1,0,0,0,0), Weapons(0,0,0,0,0))
test.addShip("DEF", 2, "INT", 3, 0, 0, 0, Weapons(1,0,0,0,0), Weapons(0,0,0,0,0))

print (test.solveBattle())