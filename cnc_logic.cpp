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

    // Guard against invalid inputs that could cause early crashes or weird math
    // Assuming 0 values for inputs like Dc, ap, ae might be valid in "not set yet" UI state,
    // but mathematically we should be careful.

    // Step A: D_cap Logic
    if (inputs.ic > 0) {
        // Round Insert
        double radius = inputs.ic / 2.0;
        // Avoid sqrt of negative number if ic - 2*ap < 0 (meaning ap > radius)
        // But we guard with if ap <= radius.
        if (inputs.ap <= radius) {
             double term = std::pow(inputs.ic, 2) - std::pow(inputs.ic - 2 * inputs.ap, 2);
             if (term < 0) term = 0; // Safety
             outputs.D_cap = (inputs.Dc - inputs.ic) + std::sqrt(term);
        } else {
             outputs.D_cap = inputs.Dc;
        }
    }
    else if (inputs.Kr > 0 && inputs.Kr < 90) {
        // Angled Cutter
        double tan_Kr = std::tan(toRadians(inputs.Kr));
        if (std::abs(tan_Kr) > 1e-6) {
             outputs.D_cap = inputs.Dc + ((2 * inputs.ap) / tan_Kr);
        } else {
            outputs.D_cap = inputs.Dc;
        }
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
        if (K > 1e-9) { // Avoid division by zero
            outputs.hex = inputs.fz_input / K;
        } else {
            outputs.hex = 0.0;
        }
    }

    // Step B: Calculate Spindle Speed (n)
    if (outputs.D_cap > 1e-9) { // Avoid division by zero
        outputs.n = (inputs.Vc * 1000.0) / (M_PI * outputs.D_cap);
    } else {
        outputs.n = 0;
    }

    // Step D: Calculate Outputs
    outputs.Vf = outputs.n * inputs.Z * outputs.fz;

    // Feed Rate Compensation
    outputs.Vf_corrected = outputs.Vf; // Default
    if (inputs.compMode == CompensationMode::InternalHole && inputs.contourDiameter > 1e-9) {
        // Internal Hole (Reduce Feed)
        outputs.Vf_corrected = outputs.Vf * (inputs.contourDiameter - inputs.Dc) / inputs.contourDiameter;
    } else if (inputs.compMode == CompensationMode::ExternalBoss && inputs.contourDiameter > 1e-9) {
        // External Boss (Increase Feed)
        outputs.Vf_corrected = outputs.Vf * (inputs.contourDiameter + inputs.Dc) / inputs.contourDiameter;
    }

    // Safety for negative feed (if Dc > Hole Diameter)
    if (outputs.Vf_corrected < 0) outputs.Vf_corrected = 0;

    outputs.MRR = (inputs.ap * inputs.ae * outputs.Vf) / 1000.0;

    return outputs;
}

double CncCalculator::calculate_thinning_factor(const CncInputs& inputs, double D_cap) {
    // Calculates K where fz = hex * K
    double factor = 1.0;

    if (inputs.ic > 0) {
        // Round Insert Logic
        double radius = inputs.ic / 2.0;
        if (inputs.ap <= radius) {
            // Shallow
            double term_ap = inputs.ap * inputs.ic - std::pow(inputs.ap, 2);
            if (term_ap < 0) term_ap = 0;
            double chord_height_ap = std::sqrt(term_ap);

            if (((inputs.ae + 0.01) * 2) <= D_cap) {
                // Double Thinning
                double term_ae = D_cap * inputs.ae - std::pow(inputs.ae, 2);
                if (term_ae < 0) term_ae = 0;
                double chord_height_ae = std::sqrt(term_ae);

                if (chord_height_ap > 1e-9 && chord_height_ae > 1e-9) {
                    factor = (inputs.ic * D_cap) / (4 * chord_height_ap * chord_height_ae);
                }
            } else {
                // Shallow Only
                if (chord_height_ap > 1e-9) {
                    factor = inputs.ic / (2 * chord_height_ap);
                }
            }
        } else {
            // Deep
            if (((inputs.ae + 0.01) * 2) <= D_cap) {
                // Deep + Narrow
                double term_radius = radius * inputs.ic - std::pow(radius, 2);
                if (term_radius < 0) term_radius = 0;
                double chord_height_radius = std::sqrt(term_radius);

                double term_ae = D_cap * inputs.ae - std::pow(inputs.ae, 2);
                if (term_ae < 0) term_ae = 0;
                double chord_height_ae = std::sqrt(term_ae);

                if (chord_height_radius > 1e-9 && chord_height_ae > 1e-9) {
                    factor = (inputs.ic * D_cap) / (4 * chord_height_radius * chord_height_ae);
                }
            } else {
                 double term_radius = radius * inputs.ic - std::pow(radius, 2);
                 if (term_radius < 0) term_radius = 0;
                 double chord_height_radius = std::sqrt(term_radius);

                 if (chord_height_radius > 1e-9) {
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
             double term_ae = (D_cap * inputs.ae) - std::pow(inputs.ae, 2);
             if (term_ae < 0) term_ae = 0;
             double denominator = 2 * std::sin(Kr_rad) * std::sqrt(term_ae);

             if (denominator > 1e-9) {
                 factor = D_cap / denominator;
             }
        } else {
            // Lead Angle Only
             double sin_Kr = std::sin(Kr_rad);
             if (sin_Kr > 1e-9) {
                 factor = 1.0 / sin_Kr;
             }
        }
    }
    else {
        // Standard Mill Logic
        if (inputs.ae <= (inputs.Dc / 2.0)) {
             if (inputs.Dc > 1e-9 && inputs.ae > 1e-9) {
                 factor = 1.0 / std::sqrt(inputs.ae / inputs.Dc);
             }
        }
    }

    return factor;
}
