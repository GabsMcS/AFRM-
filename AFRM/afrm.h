// afrm.h - AFRM calculation and validation functions
// Aircraft Fuel and Range Management prototype
// Simulated data only. Not for operational use.
// Units: fuel in kg, distance in km, burn rate in kg/km.
//
// The functions are kept in this header so that both the console
// application (afrm.cpp) and the unit test project (AFRMTests.cpp)
// use exactly the same code.

#pragma once

#include <cmath>
#include <string>

struct FlightScenario
{
    std::string id;
    double initialFuelKg;
    double distanceKm;
    double burnRateKgPerKm;
    double reserveFuelKg;
};

// Input validation: returns an empty string if the scenario is valid,
// otherwise the reason it was rejected. Runs before any calculation.
inline std::string validateScenario(const FlightScenario& s)
{
    if (!std::isfinite(s.initialFuelKg) || !std::isfinite(s.distanceKm) ||
        !std::isfinite(s.burnRateKgPerKm) || !std::isfinite(s.reserveFuelKg))
        return "all values must be finite numbers";
    if (s.initialFuelKg < 0.0)      return "initial fuel cannot be negative";
    if (s.distanceKm <= 0.0)        return "distance must be greater than zero";
    if (s.burnRateKgPerKm <= 0.0)   return "burn rate must be greater than zero";
    if (s.reserveFuelKg < 0.0)      return "reserve fuel cannot be negative";
    return "";
}

// Fuel needed for the planned flight
inline double calculateFuelRequired(double distanceKm, double burnRateKgPerKm)
{
    return distanceKm * burnRateKgPerKm;
}

// Fuel left on arrival
inline double calculateFuelRemaining(double initialFuelKg, double fuelRequiredKg)
{
    return initialFuelKg - fuelRequiredKg;
}

// Furthest distance that can be flown before the reserve is used
inline double calculateMaxRange(double initialFuelKg, double reserveFuelKg, double burnRateKgPerKm)
{
    return (initialFuelKg - reserveFuelKg) / burnRateKgPerKm;
}

// Operational criterion: SAFE if fuel remaining is at least the reserve
inline bool isFuelSafe(double fuelRemainingKg, double reserveFuelKg)
{
    return fuelRemainingKg >= reserveFuelKg;
}
