#include "cnc_logic.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double CncCalculator::toRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

CncOutputs CncCalculator::calculate(const CncInputs& inputs) {
    CncOutputs outputs;

    // Step A: D_cap Logic
    if (inputs.ic > 0) {
        // Round Insert
        double radius = inputs.ic / 2.0;
        if (inputs.ap <= radius) {
             outputs.D_cap = (inputs.Dc - inputs.ic) + std::sqrt(std::pow(inputs.ic, 2) - std::pow(inputs.ic - 2 * inputs.ap, 2));
        } else {
             outputs.D_cap = inputs.Dc;
        }
    }
    else if (inputs.Kr > 0 && inputs.Kr < 90) {
        // Angled Cutter
        outputs.D_cap = inputs.Dc + ((2 * inputs.ap) / std::tan(toRadians(inputs.Kr)));
    }
    else {
        // Standard
        outputs.D_cap = inputs.Dc;
    }

    // Calculate Thinning Factor (K)
    double K = calculate_thinning_factor(inputs, outputs.D_cap);

    // Solve based on Mode
    if (inputs.mode == CalculationMode::SolveForFz) {
        outputs.hex = inputs.hex;
        outputs.fz = inputs.hex * K;
    } else {
        outputs.fz = inputs.fz_input;
        if (K > 0) {
            outputs.hex = inputs.fz_input / K;
        } else {
            outputs.hex = 0.0;
        }
    }

    // Step B: Calculate Spindle Speed (n)
    if (outputs.D_cap > 0) {
        outputs.n = (inputs.Vc * 1000.0) / (M_PI * outputs.D_cap);
    } else {
        outputs.n = 0;
    }

    // Step D: Calculate Outputs
    outputs.Vf = outputs.n * inputs.Z * outputs.fz;
    outputs.MRR = (inputs.ap * inputs.ae * outputs.Vf) / 1000.0;

    return outputs;
}

double CncCalculator::calculate_thinning_factor(const CncInputs& inputs, double D_cap) {
    // Calculates K where fz = hex * K
    // We reuse the logic by assuming hex = 1.0 and seeing what fz we get.
    // However, we must implement the logic cleanly.

    double factor = 1.0;

    if (inputs.ic > 0) {
        // Round Insert Logic
        double radius = inputs.ic / 2.0;
        if (inputs.ap <= radius) {
            // Shallow
            double chord_height_ap = std::sqrt(inputs.ap * inputs.ic - std::pow(inputs.ap, 2));

            if (((inputs.ae + 0.01) * 2) <= D_cap) {
                // Double Thinning
                double chord_height_ae = std::sqrt(D_cap * inputs.ae - std::pow(inputs.ae, 2));
                if (chord_height_ap > 0 && chord_height_ae > 0) {
                    factor = (inputs.ic * D_cap) / (4 * chord_height_ap * chord_height_ae);
                }
            } else {
                // Shallow Only
                if (chord_height_ap > 0) {
                    factor = inputs.ic / (2 * chord_height_ap);
                }
            }
        } else {
            // Deep
            if (((inputs.ae + 0.01) * 2) <= D_cap) {
                // Deep + Narrow
                double chord_height_radius = std::sqrt(radius * inputs.ic - std::pow(radius, 2)); // == radius
                double chord_height_ae = std::sqrt(D_cap * inputs.ae - std::pow(inputs.ae, 2));
                if (chord_height_radius > 0 && chord_height_ae > 0) {
                    factor = (inputs.ic * D_cap) / (4 * chord_height_radius * chord_height_ae);
                }
            } else {
                 // Standard Round Insert Deep (Should be 1/sin(90) effectively 1 if we ignore radial thinning logic for simple round insert?)
                 // The prompt formula: fz = (hex * ic) / (2 * sqrt(radius * ic - radius**2))
                 // sqrt(radius*ic - radius^2) = sqrt(ic^2/2 - ic^2/4) ?? No.
                 // radius = ic/2.
                 // radius*ic - radius^2 = (ic/2)*ic - (ic/2)^2 = ic^2/2 - ic^2/4 = ic^2/4.
                 // sqrt(ic^2/4) = ic/2.
                 // So denominator = 2 * (ic/2) = ic.
                 // fz = (hex * ic) / ic = hex.
                 // So factor is 1.0. Correct.
                 double chord_height_radius = std::sqrt(radius * inputs.ic - std::pow(radius, 2));
                 if (chord_height_radius > 0) {
                     factor = inputs.ic / (2 * chord_height_radius);
                 }
            }
        }
    }
    else if (inputs.Kr > 0 && inputs.Kr < 90) {
        // Angled Logic
        double Kr_rad = toRadians(inputs.Kr);
        if (inputs.ae < (D_cap / 2.0)) {
            // Radial
             double denominator = 2 * std::sin(Kr_rad) * std::sqrt((D_cap * inputs.ae) - std::pow(inputs.ae, 2));
             if (denominator > 0) {
                 factor = D_cap / denominator;
             }
        } else {
            // Lead Angle Only
             double sin_Kr = std::sin(Kr_rad);
             if (sin_Kr > 0) {
                 factor = 1.0 / sin_Kr;
             }
        }
    }
    else {
        // Standard Mill Logic
        if (inputs.ae <= (inputs.Dc / 2.0)) {
             if (inputs.Dc > 0 && inputs.ae > 0) {
                 factor = 1.0 / std::sqrt(inputs.ae / inputs.Dc);
             }
        }
    }

    return factor;
}
