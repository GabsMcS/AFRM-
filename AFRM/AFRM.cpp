// afrm.cpp - AFRM console application
// Aircraft Fuel and Range Management prototype
// Simulated data only. Not for operational use.

#include "afrm.h"

#include <iomanip>
#include <iostream>
#include <vector>

int main()
{
    // Simulated flight and fuel data (verification scenarios TC01-TC06)
    const std::vector<FlightScenario> scenarios = {
        {"TC01", 5000.0,  1200.0, 3.0, 1000.0},  // normal: sufficient fuel
        {"TC02", 4000.0,  1200.0, 3.0, 1000.0},  // normal: below reserve
        {"TC03", 4600.0,  1200.0, 3.0, 1000.0},  // boundary: exactly at reserve
        {"TC04", 3000.0,  1200.0, 3.0, 1000.0},  // abnormal: fuel shortfall
        {"TC05", 5000.0,  1200.0, 0.0, 1000.0},  // abnormal: zero burn rate
        {"TC06", 5000.0, -1200.0, 3.0, 1000.0}   // abnormal: negative distance
    };

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "AFRM Fuel and Range Management Prototype\n";
    std::cout << "========================================\n";

    for (const FlightScenario& s : scenarios)
    {
        std::cout << "\nScenario: " << s.id << "\n";

        const std::string error = validateScenario(s);
        if (!error.empty())
        {
            std::cout << "Fuel Status:     INVALID INPUT (" << error << ")\n";
            continue;
        }

        const double fuelRequired = calculateFuelRequired(s.distanceKm, s.burnRateKgPerKm);
        const double fuelRemaining = calculateFuelRemaining(s.initialFuelKg, fuelRequired);
        const double maxRange = calculateMaxRange(s.initialFuelKg, s.reserveFuelKg, s.burnRateKgPerKm);
        const bool safe = isFuelSafe(fuelRemaining, s.reserveFuelKg);

        std::cout << "Initial Fuel:    " << s.initialFuelKg << " kg\n";
        std::cout << "Flight Distance: " << s.distanceKm << " km\n";
        std::cout << "Fuel Burn Rate:  " << s.burnRateKgPerKm << " kg/km\n";
        std::cout << "Reserve Fuel:    " << s.reserveFuelKg << " kg\n";
        std::cout << "Fuel Required:   " << fuelRequired << " kg\n";
        std::cout << "Fuel Remaining:  " << fuelRemaining << " kg\n";
        std::cout << "Maximum Range:   " << maxRange << " km\n";
        std::cout << "Fuel Status:     " << (safe ? "SAFE" : "WARNING") << "\n";
    }

    return 0;
}