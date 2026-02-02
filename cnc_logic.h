#ifndef CNC_LOGIC_H
#define CNC_LOGIC_H

#include <cmath>
#include <iostream>

enum class CalculationMode {
    SolveForFz, // Input hex, calculate fz
    SolveForHex // Input fz, calculate hex
};

struct CncInputs {
    double Vc = 0.0;    // Cutting Speed (m/min)
    double Dc = 0.0;    // Nominal Tool Diameter (mm)
    double ic = 0.0;    // Insert Diameter (mm). 0 if not applicable.
    double Kr = 90.0;   // Cutting Edge Angle (Degrees).
    double ap = 0.0;    // Depth of Cut (mm)
    double ae = 0.0;    // Width of Cut (mm)
    double hex = 0.0;   // Target Chip Thickness (mm) - Used if mode is SolveForFz
    double fz_input = 0.0; // Target Feed per Tooth (mm) - Used if mode is SolveForHex
    int Z = 0;          // Number of Teeth
    CalculationMode mode = CalculationMode::SolveForFz;
};

struct CncOutputs {
    double D_cap = 0.0; // Effective Diameter
    double n = 0.0;     // Spindle Speed (RPM)
    double fz = 0.0;    // Feed per Tooth (mm) (Result or Pass-through)
    double hex = 0.0;   // Chip Thickness (mm) (Result or Pass-through)
    double Vf = 0.0;    // Table Feed (mm/min)
    double MRR = 0.0;   // Material Removal Rate (cm3/min)
};

class CncCalculator {
public:
    static CncOutputs calculate(const CncInputs& inputs);

private:
    static double toRadians(double degrees);

    // Returns the Chip Thinning Factor (K) where fz = hex * K
    static double calculate_thinning_factor(const CncInputs& inputs, double D_cap);
};

#endif // CNC_LOGIC_H
