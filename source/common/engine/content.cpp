#include <engine/content.h>

#include <engine/terrain.h>
#include <engine/unit.h>
#include <engine/map.h>

TerrainType
    space("space"),
    nebula("nebula"),
    asteroids("asteroids"),
    debris("debris"),
    minefield("minefield"),
    planetoid("planetoid"),
    rockPlanet("rockPlanet"),
    gasPlanet("gasPlanet"),
    redDwarf("redDwarf"),
    yellowDwarf("yellowDwarf"),
    redGiant("redGiant"),
    neutronStar("neutronStar"),
    blackHole("blackHole");

UnitType
    fighter("fighter"),
    bomber("bomber"),
    cruiser("cruiser");

Map
    emptyMap("Empty Map", glm::ivec2(16, 9), 2, &space),
    kesslerSyndrome("Kessler Syndrome", glm::ivec2(17, 9), 2, &space),
    galaxy("Galaxy", glm::ivec2(17,9), 2, &space);

void populate(Map &map, TerrainType &terrain, const std::vector<glm::ivec2> &locations) {
    for(auto pos : locations)
        map.terrain.set(pos, &terrain);
}

void populateSymetric(Map &map, TerrainType &terrain, const std::vector<glm::ivec2> &locations) {
    for(auto pos : locations) {
        map.terrain.set(pos, &terrain);
        map.terrain.set(map.terrain.size() - pos, &terrain);
    }
}

void initGameContent() {


    space.movementCost = 10;
    TerrainType::registry.add(space);

    nebula.movementCost = 20;
    TerrainType::registry.add(nebula);

    asteroids.movementCost = 30;
    TerrainType::registry.add(asteroids);

    debris.movementCost = 40;
    TerrainType::registry.add(debris);

    minefield.movementCost = 50;
    TerrainType::registry.add(minefield);

    planetoid.movementCost = 15;
    TerrainType::registry.add(planetoid);

    rockPlanet.movementCost = 20;
    TerrainType::registry.add(rockPlanet);

    gasPlanet.movementCost = 25;
    TerrainType::registry.add(gasPlanet);

    redDwarf.movementCost = 30;
    TerrainType::registry.add(redDwarf);

    yellowDwarf.movementCost = 40;
    TerrainType::registry.add(yellowDwarf);

    redGiant.movementCost = 60;
    TerrainType::registry.add(redGiant);

    neutronStar.movementCost = 80;
    TerrainType::registry.add(neutronStar);

    blackHole.movementCost = 100;
    TerrainType::registry.add(blackHole);




    fighter.movementPointsPerTurn = 100;
    fighter.maxHealth = 50;
    fighter.armor = 10;
    fighter.evasion = 50;
    fighter.attackAccuracy = 100;
    fighter.attackDamage = 50;
    fighter.attackPenetration = 25;
    UnitType::registry.add(fighter);

    bomber.movementPointsPerTurn = 80;
    bomber.maxHealth = 75;
    bomber.armor = 15;
    bomber.evasion = 30;
    bomber.attackAccuracy = 25;
    bomber.attackDamage = 100;
    bomber.attackPenetration = 50;
    UnitType::registry.add(bomber);

    cruiser.movementPointsPerTurn = 60;
    cruiser.maxHealth = 250;
    cruiser.armor = 50;
    cruiser.attackDamage = 75;
    cruiser.actionPointsPerTurn = 2;
    cruiser.attackPenetration = 35;
    cruiser.attackAccuracy = 50;
    UnitType::registry.add(cruiser);



    emptyMap.startingUnits[0] = {
        Unit(fighter, 0, {0,2}),
        Unit(bomber,  0, {0,4}),
        Unit(fighter, 0, {0,6}),
        Unit(cruiser, 0, {2,4})
    };
    emptyMap.startingUnits[1] = {
        Unit(fighter, 1, {15,2}),
        Unit(bomber, 1, {15,4}),
        Unit(fighter, 1, {15,6}),
        Unit(cruiser, 1, {13,4})
    };
    Map::registry.add(emptyMap);


    kesslerSyndrome.startingUnits[0] = {
        Unit(cruiser, 0, {0,2}),
        Unit(fighter, 0, {0,4}),
        Unit(cruiser, 0, {0,6}),
        Unit(bomber, 0, {2,2}),
        Unit(bomber, 0, {2,6})
    };
    kesslerSyndrome.startingUnits[1] = {
        Unit(cruiser, 1, {16,2}),
        Unit(fighter, 1, {16,4}),
        Unit(cruiser, 1, {16,6}),
        Unit(bomber, 1, {14,2}),
        Unit(bomber, 1, {14,6})
    };
    for(int x=8-2; x<=8+2; ++x)
        for(int y=4-2; y<=4+2; ++y)
            kesslerSyndrome.terrain.set({x,y}, &debris);
    kesslerSyndrome.terrain.set({8,4}, &rockPlanet);
    Map::registry.add(kesslerSyndrome);



    populate(galaxy, blackHole, {{8,4}});
    populateSymetric(galaxy, debris, {{7,4},{8,3}});
    populateSymetric(galaxy, redGiant, {{6,5}});
    populateSymetric(galaxy, yellowDwarf, {{8,2},{8,6}});
    populateSymetric(galaxy, redDwarf, {{1,7},{3,2},{4,1},{6,3}});
    populateSymetric(galaxy, nebula, {{1,6}, {2,8}, {4,2}, {5,1}, {6,2}, {7,2}, {6,4}, {7,3}, {7,5}, {7,6}});

    galaxy.startingUnits[0] = {
        Unit(fighter, 0, {0,1}),
        Unit(fighter, 0, {1,0}),
        Unit(bomber, 0, {0,0}),
        Unit(fighter, 0, {2,5}),
        Unit(fighter, 0, {3,7}),
        Unit(cruiser, 0, {2,6})
    };

    galaxy.startingUnits[1] = {
        Unit(fighter, 1, {16,7}),
        Unit(fighter, 1, {15,8}),
        Unit(bomber, 1, {16,8}),
        Unit(fighter, 1, {14,3}),
        Unit(fighter, 1, {13,1}),
        Unit(cruiser, 1, {14,2})
    };

    Map::registry.add(galaxy);


}