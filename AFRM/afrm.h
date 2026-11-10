// afrm.h - AFRM calculation, validation and fuel-status functions
// Aircraft Fuel and Range Management prototype
// Simulated data only
// Units: fuel in kg, distance in km, burn rate in kg/km.
//
// Version 2.0.0 (CR-002) replaces the binary SAFE/WARNING decision with a
// graded status, adds a plausibility envelope to validation, compares fuel
// at the 0.01 kg resolution shown to the user, and clamps maximum range.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

inline constexpr const char* AFRM_VERSION = "2.0.0";

// Plausibility envelope for simulated data (REQ-07). Inside these limits every
// calculation stays finite and the fixed-point conversion cannot overflow.
inline constexpr double MAX_FUEL_KG = 300000.0;
inline constexpr double MAX_DISTANCE_KM = 20000.0;
inline constexpr double MAX_BURN_RATE_KG_PER_KM = 50.0;

// Contingency margin as a fraction of the fuel required for the flight (REQ-08)
inline constexpr double CONTINGENCY_FRACTION = 0.05;

struct FlightScenario
{
    std::string id;
    double initialFuelKg;
    double distanceKm;
    double burnRateKgPerKm;
    double reserveFuelKg;
};

enum class FuelStatus
{
    Invalid,        // input rejected, nothing calculated
    Safe,           // reserve and contingency margin both protected
    Marginal,       // reserve protected, contingency margin not fully protected
    ReserveBreach,  // flight can be completed, but only by using the reserve
    Insufficient    // not enough fuel to complete the flight
};

struct FuelAssessment
{
    FuelStatus status = FuelStatus::Invalid;
    double fuelRequiredKg = 0.0;
    double fuelRemainingKg = 0.0;
    double contingencyMarginKg = 0.0;
    double maxRangeKm = 0.0;
    std::string error;
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
    if (s.initialFuelKg > MAX_FUEL_KG)              return "initial fuel exceeds the plausible limit";
    if (s.distanceKm > MAX_DISTANCE_KM)             return "distance exceeds the plausible limit";
    if (s.burnRateKgPerKm > MAX_BURN_RATE_KG_PER_KM) return "burn rate exceeds the plausible limit";
    if (s.reserveFuelKg > MAX_FUEL_KG)              return "reserve fuel exceeds the plausible limit";
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

// Furthest distance that can be flown before the reserve is used.
// Never negative: a reserve larger than the fuel on board gives zero range.
inline double calculateMaxRange(double initialFuelKg, double reserveFuelKg, double burnRateKgPerKm)
{
    return (std::max)(0.0, (initialFuelKg - reserveFuelKg) / burnRateKgPerKm);
}

// Contingency margin for the flight
inline double calculateContingencyMargin(double fuelRequiredKg)
{
    return fuelRequiredKg * CONTINGENCY_FRACTION;
}

// Fuel as a whole number of 0.01 kg units, the resolution of the display.
// Comparing these integers removes binary rounding error from the decision
// and means the status always agrees with the figures shown to the user.
inline std::int64_t toCentiKg(double kg)
{
    return std::llround(kg * 100.0);
}

// Operational criteria, most severe first
inline FuelStatus classifyFuel(double fuelRemainingKg, double reserveFuelKg, double contingencyMarginKg)
{
    const std::int64_t remaining = toCentiKg(fuelRemainingKg);
    const std::int64_t reserve = toCentiKg(reserveFuelKg);
    const std::int64_t margin = toCentiKg(contingencyMarginKg);

    if (remaining < 0)                 return FuelStatus::Insufficient;
    if (remaining < reserve)           return FuelStatus::ReserveBreach;
    if (remaining < reserve + margin)  return FuelStatus::Marginal;
    return FuelStatus::Safe;
}

// Single entry point: validates, calculates and classifies one scenario.
// Nothing is calculated for a rejected scenario.
inline FuelAssessment assessFlight(const FlightScenario& s)
{
    FuelAssessment a;
    a.error = validateScenario(s);
    if (!a.error.empty())
        return a;

    a.fuelRequiredKg = calculateFuelRequired(s.distanceKm, s.burnRateKgPerKm);
    a.fuelRemainingKg = calculateFuelRemaining(s.initialFuelKg, a.fuelRequiredKg);
    a.contingencyMarginKg = calculateContingencyMargin(a.fuelRequiredKg);
    a.maxRangeKm = calculateMaxRange(s.initialFuelKg, s.reserveFuelKg, s.burnRateKgPerKm);
    a.status = classifyFuel(a.fuelRemainingKg, s.reserveFuelKg, a.contingencyMarginKg);
    return a;
}

inline const char* statusText(FuelStatus status)
{
    switch (status)
    {
    case FuelStatus::Safe:          return "SAFE";
    case FuelStatus::Marginal:      return "MARGINAL";
    case FuelStatus::ReserveBreach: return "RESERVE BREACH";
    case FuelStatus::Insufficient:  return "INSUFFICIENT";
    case FuelStatus::Invalid:       break;
    }
    return "INVALID INPUT";
}