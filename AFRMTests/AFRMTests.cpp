// AFRMTests.cpp - automated unit tests for the AFRM prototype
// Microsoft Native Unit Test Framework (Visual Studio Test Explorer)
// 
// TC01-TC06 are the regression scenarios from baseline v1.0.0.
// TC07 onwards were added by the test plan and CR-002.
// Test identifiers trace to test plan documentation

#include "pch.h"
#include "CppUnitTest.h"
#include "../AFRM/afrm.h"

#include <limits>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AFRMTests
{
    // Values are compared to 2 decimal places, matching the console output
    const double TOLERANCE = 0.01;
    const double NOT_A_NUMBER = std::numeric_limits<double>::quiet_NaN();
    const double INFINITE_VALUE = std::numeric_limits<double>::infinity();

    // TC01-TC04 flight: 1,200 km at 3 kg/km with a 1,000 kg reserve.
    // Fuel required 3,600 kg, contingency margin 180 kg.
    FlightScenario flight(double initialFuelKg)
    {
        return FlightScenario{ "T", initialFuelKg, 1200.0, 3.0, 1000.0 };
    }

    void assertStatus(FuelStatus expected, const FlightScenario& s, const wchar_t* message)
    {
        Assert::IsTrue(expected == assessFlight(s).status, message);
    }

    void assertRejected(const FlightScenario& s, const char* reason)
    {
        const FuelAssessment a = assessFlight(s);
        Assert::IsTrue(a.status == FuelStatus::Invalid, L"Scenario was not rejected");
        Assert::IsTrue(a.error.find(reason) != std::string::npos, L"Wrong rejection reason");
        // Nothing may be calculated for a rejected scenario
        Assert::IsTrue(a.fuelRequiredKg == 0.0 && a.fuelRemainingKg == 0.0 && a.contingencyMarginKg == 0.0
            && a.maxRangeKm == 0.0, L"Calculated despite rejection");
    }

    TEST_CLASS(NormalConditionTests)
    {
    public:
        TEST_METHOD(TC01_SufficientFuel_IsSafe)
        {
            const FuelAssessment a = assessFlight(flight(5000.0));
            Assert::AreEqual(3600.0, a.fuelRequiredKg, TOLERANCE, L"Fuel required");
            Assert::AreEqual(1400.0, a.fuelRemainingKg, TOLERANCE, L"Fuel remaining");
            Assert::AreEqual(180.0, a.contingencyMarginKg, TOLERANCE, L"Contingency margin");
            Assert::AreEqual(1333.33, a.maxRangeKm, TOLERANCE, L"Maximum range");
            Assert::IsTrue(a.status == FuelStatus::Safe, L"Expected SAFE");
        }

        TEST_METHOD(TC02_BelowReserve_IsReserveBreach)
        {
            const FuelAssessment a = assessFlight(flight(4000.0));
            Assert::AreEqual(400.0, a.fuelRemainingKg, TOLERANCE, L"Fuel remaining");
            Assert::AreEqual(1000.0, a.maxRangeKm, TOLERANCE, L"Maximum range");
            Assert::IsTrue(a.status == FuelStatus::ReserveBreach, L"Expected RESERVE BREACH");
        }

        TEST_METHOD(TC07_InsideContingencyMargin_IsMarginal)
        {
            assertStatus(FuelStatus::Marginal, flight(4700.0), L"Expected MARGINAL");
        }
    };

    TEST_CLASS(BoundaryConditionTests)
    {
    public:
        TEST_METHOD(TC03_ExactlyAtReserve_IsMarginal)
        {
            const FuelAssessment a = assessFlight(flight(4600.0));
            Assert::AreEqual(1000.0, a.fuelRemainingKg, TOLERANCE, L"Fuel remaining");
            Assert::AreEqual(1200.0, a.maxRangeKm, TOLERANCE, L"Maximum range equals flight distance");
            Assert::IsTrue(a.status == FuelStatus::Marginal, L"Expected MARGINAL at the reserve");
        }

        TEST_METHOD(TC08_JustBelowReserve_IsReserveBreach)
        {
            assertStatus(FuelStatus::ReserveBreach, flight(4599.99), L"Expected RESERVE BREACH");
        }

        TEST_METHOD(TC09_ExactlyAtReservePlusMargin_IsSafe)
        {
            assertStatus(FuelStatus::Safe, flight(4780.0), L"Expected SAFE at the margin boundary");
        }

        TEST_METHOD(TC10_JustBelowReservePlusMargin_IsMarginal)
        {
            assertStatus(FuelStatus::Marginal, flight(4779.99), L"Expected MARGINAL");
        }

        TEST_METHOD(TC11_ArrivingWithZeroFuel_IsReserveBreach)
        {
            assertStatus(FuelStatus::ReserveBreach, flight(3600.0), L"Expected RESERVE BREACH");
        }

        TEST_METHOD(TC12_JustShortOfDestination_IsInsufficient)
        {
            assertStatus(FuelStatus::Insufficient, flight(3599.99), L"Expected INSUFFICIENT");
        }

        TEST_METHOD(TC13_DecimalInputsOnReserve_NotBelowReserve)
        {
            // 4.4 kg/km has no exact binary form. v1.0.0 calculated 499.99999999999994 kg
            // remaining and reported a false WARNING for a flight exactly on the reserve.
            const FlightScenario s{ "T", 940.0, 100.0, 4.4, 500.0 };
            Assert::IsTrue(assessFlight(s).status == FuelStatus::Marginal, L"False breach from rounding");
        }

        TEST_METHOD(TC14_StatusAgreesWithDisplayedFigures)
        {
            // 999.996 kg displays as 1000.00 kg, so it must not be reported as below 1000.00 kg
            assertStatus(FuelStatus::Marginal, flight(4599.996), L"Status contradicts the display");
        }

        TEST_METHOD(TC15_ZeroReserve_ArrivingWithZero_IsMarginal)
        {
            const FlightScenario s{ "T", 3600.0, 1200.0, 3.0, 0.0 };
            assertStatus(FuelStatus::Marginal, s, L"Expected MARGINAL");
        }

        TEST_METHOD(TC16_ZeroInitialFuel_IsAcceptedAndInsufficient)
        {
            assertStatus(FuelStatus::Insufficient, flight(0.0), L"Expected INSUFFICIENT");
        }

        TEST_METHOD(TC17_AtUpperLimits_IsAccepted)
        {
            const FlightScenario s{ "T", MAX_FUEL_KG, MAX_DISTANCE_KM, MAX_BURN_RATE_KG_PER_KM, MAX_FUEL_KG };
            const FuelAssessment a = assessFlight(s);
            Assert::IsTrue(a.error.empty(), L"Limit value was rejected");
            Assert::IsTrue(a.status == FuelStatus::Insufficient, L"Expected INSUFFICIENT");
        }

        TEST_METHOD(TC34_SmallestPositiveValues_AreAccepted)
        {
            const FlightScenario s{ "T", 5000.0, 0.01, 0.01, 1000.0 };
            Assert::IsTrue(assessFlight(s).error.empty(), L"Small positive values were rejected");
        }

        TEST_METHOD(TC18_ReserveAboveFuel_RangeIsZero)
        {
            const FlightScenario s{ "T", 500.0, 100.0, 3.0, 1000.0 };
            const FuelAssessment a = assessFlight(s);
            Assert::AreEqual(0.0, a.maxRangeKm, TOLERANCE, L"Range must not be negative");
            Assert::IsTrue(a.status == FuelStatus::ReserveBreach, L"Expected RESERVE BREACH");
        }
    };

    TEST_CLASS(AbnormalConditionTests)
    {
    public:
        TEST_METHOD(TC04_FuelShortfall_IsInsufficient)
        {
            const FuelAssessment a = assessFlight(flight(3000.0));
            Assert::AreEqual(-600.0, a.fuelRemainingKg, TOLERANCE, L"Fuel remaining");
            Assert::AreEqual(666.67, a.maxRangeKm, TOLERANCE, L"Maximum range");
            Assert::IsTrue(a.status == FuelStatus::Insufficient, L"Expected INSUFFICIENT");
        }

        TEST_METHOD(TC05_ZeroBurnRate_IsRejected)
        {
            assertRejected(FlightScenario{ "T", 5000.0, 1200.0, 0.0, 1000.0 }, "burn rate");
        }

        TEST_METHOD(TC06_NegativeDistance_IsRejected)
        {
            assertRejected(FlightScenario{ "T", 5000.0, -1200.0, 3.0, 1000.0 }, "distance");
        }

        TEST_METHOD(TC19_NegativeFuel_IsRejected)
        {
            assertRejected(flight(-0.01), "initial fuel");
        }

        TEST_METHOD(TC20_NegativeReserve_IsRejected)
        {
            assertRejected(FlightScenario{ "T", 5000.0, 1200.0, 3.0, -0.01 }, "reserve");
        }

        TEST_METHOD(TC21_ZeroDistance_IsRejected)
        {
            assertRejected(FlightScenario{ "T", 5000.0, 0.0, 3.0, 1000.0 }, "distance");
        }

        TEST_METHOD(TC22_NegativeBurnRate_IsRejected)
        {
            assertRejected(FlightScenario{ "T", 5000.0, 1200.0, -3.0, 1000.0 }, "burn rate");
        }

        // MC/DC: each field alone decides the finite-number condition
        TEST_METHOD(TC23_NonFiniteFuel_IsRejected)
        {
            assertRejected(flight(NOT_A_NUMBER), "finite");
        }

        TEST_METHOD(TC24_NonFiniteDistance_IsRejected)
        {
            assertRejected(FlightScenario{ "T", 5000.0, INFINITE_VALUE, 3.0, 1000.0 }, "finite");
        }

        TEST_METHOD(TC25_NonFiniteBurnRate_IsRejected)
        {
            assertRejected(FlightScenario{ "T", 5000.0, 1200.0, NOT_A_NUMBER, 1000.0 }, "finite");
        }

        TEST_METHOD(TC26_NonFiniteReserve_IsRejected)
        {
            assertRejected(FlightScenario{ "T", 5000.0, 1200.0, 3.0, -INFINITE_VALUE }, "finite");
        }

        TEST_METHOD(TC27_FuelAboveLimit_IsRejected)
        {
            assertRejected(flight(MAX_FUEL_KG + 0.01), "initial fuel exceeds");
        }

        TEST_METHOD(TC28_DistanceAboveLimit_IsRejected)
        {
            assertRejected(FlightScenario{ "T", 5000.0, MAX_DISTANCE_KM + 0.01, 3.0, 1000.0 }, "distance exceeds");
        }

        TEST_METHOD(TC29_BurnRateAboveLimit_IsRejected)
        {
            assertRejected(FlightScenario{ "T", 5000.0, 1200.0, MAX_BURN_RATE_KG_PER_KM + 0.01, 1000.0 }, "burn rate exceeds");
        }

        TEST_METHOD(TC30_ReserveAboveLimit_IsRejected)
        {
            assertRejected(FlightScenario{ "T", 5000.0, 1200.0, 3.0, MAX_FUEL_KG + 0.01 }, "reserve fuel exceeds");
        }

        TEST_METHOD(TC31_OverflowingInput_IsRejected)
        {
            // Overflowed to infinity and printed "inf" in v1.0.0
            assertRejected(FlightScenario{ "T", 5000.0, 1e308, 10.0, 1000.0 }, "distance exceeds");
        }
    };

    TEST_CLASS(OutputTests)
    {
    public:
        TEST_METHOD(TC32_EveryStatusHasDistinctText)
        {
            Assert::IsTrue(std::string("SAFE") == statusText(FuelStatus::Safe), L"SAFE");
            Assert::IsTrue(std::string("MARGINAL") == statusText(FuelStatus::Marginal), L"MARGINAL");
            Assert::IsTrue(std::string("RESERVE BREACH") == statusText(FuelStatus::ReserveBreach), L"RESERVE BREACH");
            Assert::IsTrue(std::string("INSUFFICIENT") == statusText(FuelStatus::Insufficient), L"INSUFFICIENT");
            Assert::IsTrue(std::string("INVALID INPUT") == statusText(FuelStatus::Invalid), L"INVALID INPUT");
        }

        TEST_METHOD(TC33_FixedPointConversionRounds)
        {
            Assert::IsTrue(toCentiKg(1000.0) == 100000, L"Whole value");
            Assert::IsTrue(toCentiKg(0.30000000000000004) == 30, L"Rounding error removed");
            Assert::IsTrue(toCentiKg(-0.01) == -1, L"Negative value");
        }
    };
}