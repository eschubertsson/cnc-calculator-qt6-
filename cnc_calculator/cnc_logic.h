#ifndef CNC_LOGIC_H
#define CNC_LOGIC_H

#include <cmath>
#include <iostream>

struct CncInputs {
    double Vc = 0.0;    // Cutting Speed (m/min)
    double Dc = 0.0;    // Nominal Tool Diameter (mm)
    double ic = 0.0;    // Insert Diameter (mm). 0 if not applicable.
    double Kr = 90.0;   // Cutting Edge Angle (Degrees).
    double ap = 0.0;    // Depth of Cut (mm)
    double ae = 0.0;    // Width of Cut (mm)
    double hex = 0.0;   // Target Chip Thickness (mm)
    int Z = 0;          // Number of Teeth
};

struct CncOutputs {
    double D_cap = 0.0; // Effective Diameter
    double n = 0.0;     // Spindle Speed (RPM)
    double fz = 0.0;    // Feed per Tooth (mm)
    double Vf = 0.0;    // Table Feed (mm/min)
    double MRR = 0.0;   // Material Removal Rate (cm3/min)
};

class CncCalculator {
public:
    static CncOutputs calculate(const CncInputs& inputs);

private:
    static double toRadians(double degrees);
    static double calculate_dcap_round(double Dc, double ic, double ap);
    static double calculate_dcap_angled(double Dc, double Kr, double ap);

    // Logic Implementations
    static void calculate_round_insert(const CncInputs& inputs, double D_cap, CncOutputs& outputs);
    static void calculate_angled_cutter(const CncInputs& inputs, double D_cap, CncOutputs& outputs);
    static void calculate_standard_mill(const CncInputs& inputs, double D_cap, CncOutputs& outputs);
};

#endif // CNC_LOGIC_H
