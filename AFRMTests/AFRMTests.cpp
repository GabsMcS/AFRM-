// AFRMTests.cpp - automated unit tests for the AFRM prototype
// Microsoft Native Unit Test Framework (Visual Studio Test Explorer)
//
// One test per verification scenario in Appendix A (TC01-TC06).
// Each test checks the calculated values and the fuel status
// against the expected results in the verification table.

#include "pch.h"
#include "CppUnitTest.h"
#include "../AFRM/afrm.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AFRMTests
{
    // Values are compared to 2 decimal places, matching the console output
    const double TOLERANCE = 0.01;

    TEST_CLASS(NormalConditionTests)
    {
    public:
        TEST_METHOD(TC01_SufficientFuel_IsSafe)
        {
            const FlightScenario s{ "TC01", 5000.0, 1200.0, 3.0, 1000.0 };
            Assert::IsTrue(validateScenario(s).empty(), L"Valid input was rejected");

            const double required = calculateFuelRequired(s.distanceKm, s.burnRateKgPerKm);
            const double remaining = calculateFuelRemaining(s.initialFuelKg, required);
            const double range = calculateMaxRange(s.initialFuelKg, s.reserveFuelKg, s.burnRateKgPerKm);

            Assert::AreEqual(3600.0, required, TOLERANCE, L"Fuel required");
            Assert::AreEqual(1400.0, remaining, TOLERANCE, L"Fuel remaining");
            Assert::AreEqual(1333.33, range, TOLERANCE, L"Maximum range");
            Assert::IsTrue(isFuelSafe(remaining, s.reserveFuelKg), L"Expected SAFE");
        }

        TEST_METHOD(TC02_BelowReserve_IsWarning)
        {
            const FlightScenario s{ "TC02", 4000.0, 1200.0, 3.0, 1000.0 };
            Assert::IsTrue(validateScenario(s).empty(), L"Valid input was rejected");

            const double required = calculateFuelRequired(s.distanceKm, s.burnRateKgPerKm);
            const double remaining = calculateFuelRemaining(s.initialFuelKg, required);
            const double range = calculateMaxRange(s.initialFuelKg, s.reserveFuelKg, s.burnRateKgPerKm);

            Assert::AreEqual(400.0, remaining, TOLERANCE, L"Fuel remaining");
            Assert::AreEqual(1000.0, range, TOLERANCE, L"Maximum range");
            Assert::IsFalse(isFuelSafe(remaining, s.reserveFuelKg), L"Expected WARNING");
        }
    };

    TEST_CLASS(BoundaryConditionTests)
    {
    public:
        TEST_METHOD(TC03_ExactlyAtReserve_IsSafe)
        {
            const FlightScenario s{ "TC03", 4600.0, 1200.0, 3.0, 1000.0 };
            Assert::IsTrue(validateScenario(s).empty(), L"Valid input was rejected");

            const double required = calculateFuelRequired(s.distanceKm, s.burnRateKgPerKm);
            const double remaining = calculateFuelRemaining(s.initialFuelKg, required);
            const double range = calculateMaxRange(s.initialFuelKg, s.reserveFuelKg, s.burnRateKgPerKm);

            Assert::AreEqual(1000.0, remaining, TOLERANCE, L"Fuel remaining");
            Assert::AreEqual(1200.0, range, TOLERANCE, L"Maximum range equals flight distance");
            // Fails if the criterion were written as > instead of >=
            Assert::IsTrue(isFuelSafe(remaining, s.reserveFuelKg), L"Expected SAFE at the boundary");
        }
    };

    TEST_CLASS(AbnormalConditionTests)
    {
    public:
        TEST_METHOD(TC04_FuelShortfall_IsWarning)
        {
            const FlightScenario s{ "TC04", 3000.0, 1200.0, 3.0, 1000.0 };
            Assert::IsTrue(validateScenario(s).empty(), L"Valid input was rejected");

            const double required = calculateFuelRequired(s.distanceKm, s.burnRateKgPerKm);
            const double remaining = calculateFuelRemaining(s.initialFuelKg, required);
            const double range = calculateMaxRange(s.initialFuelKg, s.reserveFuelKg, s.burnRateKgPerKm);

            Assert::AreEqual(-600.0, remaining, TOLERANCE, L"Fuel remaining");
            Assert::AreEqual(666.67, range, TOLERANCE, L"Maximum range");
            Assert::IsFalse(isFuelSafe(remaining, s.reserveFuelKg), L"Expected WARNING");
        }

        TEST_METHOD(TC05_ZeroBurnRate_IsRejected)
        {
            const FlightScenario s{ "TC05", 5000.0, 1200.0, 0.0, 1000.0 };
            const std::string error = validateScenario(s);

            Assert::IsFalse(error.empty(), L"Zero burn rate was not rejected");
            Assert::IsTrue(error.find("burn rate") != std::string::npos, L"Wrong rejection reason");
        }

        TEST_METHOD(TC06_NegativeDistance_IsRejected)
        {
            const FlightScenario s{ "TC06", 5000.0, -1200.0, 3.0, 1000.0 };
            const std::string error = validateScenario(s);

            Assert::IsFalse(error.empty(), L"Negative distance was not rejected");
            Assert::IsTrue(error.find("distance") != std::string::npos, L"Wrong rejection reason");
        }
    };
}