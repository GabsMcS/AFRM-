AFRM Assignment 1.

# AFRM – Aircraft Fuel and Range Management Prototype

A C++ console application that calculates fuel requirements, remaining fuel and maximum range for simulated flights, and classifies each flight as **SAFE** or **WARNING** against a reserve-fuel threshold.

## Features

- Validates every scenario before calculation, rejecting negative fuel and zero or negative distance or burn rate
- Calculates fuel required, fuel remaining and maximum range
- Classifies each flight as SAFE or WARNING
- Runs six built-in verification scenarios covering normal, boundary and abnormal conditions
- Includes six automated unit tests using the Microsoft Native Unit Test Framework

## Calculations

| Output | Formula |
| --- | --- |
| Fuel required (kg) | Distance (km) × Burn rate (kg/km) |
| Fuel remaining (kg) | Initial fuel (kg) − Fuel required (kg) |
| Maximum range (km) | (Initial fuel − Reserve fuel) ÷ Burn rate |
| Fuel status | SAFE if Fuel remaining ≥ Reserve fuel, otherwise WARNING |

The reserve represents the 30-minute final reserve fuel requirement for turbine-engined aeroplanes as a fixed mass. The `>=` operator is used because the reserve is a minimum to be protected, so arriving with exactly the reserve is compliant.

## Requirements

| Item | Version |
| --- | --- |
| Operating system | Windows 10 or 11, x64 |
| IDE | Visual Studio 2026 (Community, Professional or Enterprise) |
| Workload | Desktop development with C++ |
| Compiler | MSVC v143 [CONFIRM: version from `cl /Bv`] |
| Windows SDK | [CONFIRM: 10.0.x] |
| Language standard | ISO C++20 (`/std:c++20`) |

There are no third-party dependencies. The application uses only the C++ standard library.

## Project structure

```text
AFRM/
├── AFRM.sln                  Visual Studio solution
├── AFRM/                     Console application project
│   ├── AFRM.vcxproj
│   ├── afrm.h                Validation and calculation functions
│   └── AFRM.cpp              main(): runs the six scenarios
├── AFRMTests/                Unit test project
│   ├── AFRMTests.vcxproj
│   ├── AFRMTests.cpp         Six unit tests (TC01–TC06)
│   ├── pch.h
│   └── pch.cpp
├── .gitignore
└── README.md
```

The functions live in `afrm.h`, which is shared by the application and the test project, so both use exactly the same code.

## Build and run

1. Clone the repository and open `AFRM.sln` in Visual Studio 2022.
2. Set the solution configuration to **Release** and the platform to **x64**.
3. Select **Build → Build Solution** (`Ctrl+Shift+B`).
4. Press **Ctrl+F5** to run without debugging.

The console shows the results for all six scenarios.

## Run the unit tests

1. Build the solution as above.
2. Open **Test → Test Explorer** (`Ctrl+E`, `T`).
3. Click **Run All**.

All six tests should pass. They are grouped as `NormalConditionTests`, `BoundaryConditionTests` and `AbnormalConditionTests`.

## Verification scenarios

TC01–TC04 use a distance of 1,200 km, a burn rate of 3.0 kg/km and a reserve of 1,000 kg.

| Test | Type | Initial fuel (kg) | Expected remaining (kg) | Expected range (km) | Expected status |
| --- | --- | --- | --- | --- | --- |
| TC01 | Normal | 5,000 | 1,400.00 | 1,333.33 | SAFE |
| TC02 | Normal | 4,000 | 400.00 | 1,000.00 | WARNING |
| TC03 | Boundary | 4,600 | 1,000.00 | 1,200.00 | SAFE |
| TC04 | Abnormal (valid data) | 3,000 | −600.00 | 666.67 | WARNING |
| TC05 | Abnormal (burn rate 0 kg/km) | 5,000 | Not calculated | Not calculated | INVALID INPUT |
| TC06 | Abnormal (distance −1,200 km) | 5,000 | Not calculated | Not calculated | INVALID INPUT |

## Configuration

| Setting | Value | Reason |
| --- | --- | --- |
| Warning level | `/W4`, `/WX` in Release | Treats warnings as defects |
| Optimisation | `/Od` Debug, `/O2` Release | Debugging visibility vs deployment build |
| Floating point | `/fp:precise` | Keeps boundary results predictable |
| Runtime library | `/MT` static (Release) | No Visual C++ Redistributable needed |
| Security | `/GS`, `/sdl` | Buffer-security and additional checks |
| Static analysis | Code Analysis (`/analyze`) | Detects defects without running the code |

## Assumptions and limitations

- Constant fuel burn rate; no variation with mass, altitude or flight phase
- Fixed reserve mass; no taxi, contingency or alternate fuel
- Validation rejects impossible values but not unrealistic ones (for example, 1,000,000 kg of fuel is accepted)
- Binary SAFE/WARNING status does not distinguish a marginal shortfall from insufficient fuel for the flight
- The build and tests depend on Visual Studio 2022, whose mainstream support ends in January 2027

## Maintenance

After each Visual Studio update, rebuild the solution and run all unit tests before accepting the update. Record the new compiler version with `cl /Bv` from the Developer Command Prompt and update the Requirements table above.
