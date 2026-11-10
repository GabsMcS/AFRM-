// afrm.cpp - AFRM console application
// Aircraft Fuel and Range Management prototype
// Simulated data only

#include "afrm.h"

#include <iomanip>
#include <iostream>
#include <vector>

int main()
{
    // Simulated flight and fuel data (verification scenarios TC01-TC07)
    const std::vector<FlightScenario> scenarios = {
        {"TC01", 5000.0,  1200.0, 3.0, 1000.0},  // normal: sufficient fuel
        {"TC02", 4000.0,  1200.0, 3.0, 1000.0},  // normal: below reserve
        {"TC03", 4600.0,  1200.0, 3.0, 1000.0},  // boundary: exactly at reserve
        {"TC04", 3000.0,  1200.0, 3.0, 1000.0},  // abnormal: fuel shortfall
        {"TC05", 5000.0,  1200.0, 0.0, 1000.0},  // abnormal: zero burn rate
        {"TC06", 5000.0, -1200.0, 3.0, 1000.0},  // abnormal: negative distance
        {"TC07", 4700.0,  1200.0, 3.0, 1000.0}   // normal: inside contingency margin
    };

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "AFRM Fuel and Range Management Prototype v" << AFRM_VERSION << "\n";
    std::cout << "===============================================\n";

    for (const FlightScenario& s : scenarios)
    {
        std::cout << "\nScenario: " << s.id << "\n";

        const FuelAssessment a = assessFlight(s);
        if (a.status == FuelStatus::Invalid)
        {
            std::cout << "Fuel Status:        " << statusText(a.status) << " (" << a.error << ")\n";
            continue;
        }

        std::cout << "Initial Fuel:       " << s.initialFuelKg << " kg\n";
        std::cout << "Flight Distance:    " << s.distanceKm << " km\n";
        std::cout << "Fuel Burn Rate:     " << s.burnRateKgPerKm << " kg/km\n";
        std::cout << "Reserve Fuel:       " << s.reserveFuelKg << " kg\n";
        std::cout << "Fuel Required:      " << a.fuelRequiredKg << " kg\n";
        std::cout << "Fuel Remaining:     " << a.fuelRemainingKg << " kg\n";
        std::cout << "Contingency Margin: " << a.contingencyMarginKg << " kg\n";
        std::cout << "Maximum Range:      " << a.maxRangeKm << " km\n";
        std::cout << "Fuel Status:        " << statusText(a.status) << "\n";
    }

    return 0;
}