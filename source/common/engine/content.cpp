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
    kesslerSyndrome("Kessler Syndrome", glm::ivec2(17, 9), 2, &space);


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
    UnitType::registry.add(fighter);

    bomber.movementPointsPerTurn = 80;
    UnitType::registry.add(bomber);

    cruiser.movementPointsPerTurn = 60;
    UnitType::registry.add(cruiser);



    emptyMap.startingUnits[0] = {
        Unit(fighter, 0, {0,2}),
        Unit(fighter, 0, {0,4}),
        Unit(fighter, 0, {0,6})
    };
    emptyMap.startingUnits[1] = {
        Unit(fighter, 1, {15,2}),
        Unit(fighter, 1, {15,4}),
        Unit(fighter, 1, {15,6})
    };
    Map::registry.add(emptyMap);


    kesslerSyndrome.startingUnits[0] = {
        Unit(fighter, 0, {0,2}),
        Unit(fighter, 0, {0,4}),
        Unit(fighter, 0, {0,6})
    };
    kesslerSyndrome.startingUnits[1] = {
        Unit(fighter, 1, {16,2}),
        Unit(fighter, 1, {16,4}),
        Unit(fighter, 1, {16,6})
    };
    for(int x=8-2; x<=8+2; ++x)
        for(int y=4-2; y<=4+2; ++y)
            kesslerSyndrome.terrain.set({x,y}, &debris);
    kesslerSyndrome.terrain.set({8,4}, &rockPlanet);
    Map::registry.add(kesslerSyndrome);
}