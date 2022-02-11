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
    emptyMap("Empty Map", glm::ivec2(32,18), 2, &space),
    kesslerSyndrome("Kessler Syndrome", glm::ivec2(32,18), 2, &space);


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
    UnitType::registry.add(fighter);

    bomber.movementPointsPerTurn = 80;
    UnitType::registry.add(bomber);

    cruiser.movementPointsPerTurn = 60;
    UnitType::registry.add(cruiser);




    Map::registry.add(emptyMap);
    Map::registry.add(kesslerSyndrome);
}