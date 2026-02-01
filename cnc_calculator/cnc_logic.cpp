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

    // 1. Determine Calculation Method (The Router)
    // Priority 1: Round Insert (ic > 0)
    if (inputs.ic > 0) {
        // Step A: D_cap
        double radius = inputs.ic / 2.0;
        if (inputs.ap <= radius) {
             outputs.D_cap = (inputs.Dc - inputs.ic) + std::sqrt(std::pow(inputs.ic, 2) - std::pow(inputs.ic - 2 * inputs.ap, 2));
        } else {
             outputs.D_cap = inputs.Dc;
        }

        calculate_round_insert(inputs, outputs.D_cap, outputs);
    }
    // Priority 2: Angled Cutter (0 < Kr < 90)
    else if (inputs.Kr > 0 && inputs.Kr < 90) {
        // Step A: D_cap
        // D_cap = Dc + ((2 * ap) / tan(radians(Kr)))
        outputs.D_cap = inputs.Dc + ((2 * inputs.ap) / std::tan(toRadians(inputs.Kr)));

        calculate_angled_cutter(inputs, outputs.D_cap, outputs);
    }
    // Priority 3: Standard End Mill
    else {
        // D_cap is usually just Dc for standard mills unless tapered,
        // but prompt Step A implies D_cap logic is specific to Round/Angled.
        // Step B says "Always use D_cap if available, otherwise Dc".
        outputs.D_cap = inputs.Dc;

        calculate_standard_mill(inputs, outputs.D_cap, outputs);
    }

    // Step B: Calculate Spindle Speed (n)
    // n = (Vc * 1000) / (PI * D_cap)
    if (outputs.D_cap > 0) {
        outputs.n = (inputs.Vc * 1000.0) / (M_PI * outputs.D_cap);
    } else {
        outputs.n = 0;
    }

    // Recalculate Vf and MRR based on final n and fz
    // Step D: Calculate Outputs
    outputs.Vf = outputs.n * inputs.Z * outputs.fz;
    outputs.MRR = (inputs.ap * inputs.ae * outputs.Vf) / 1000.0;

    return outputs;
}

void CncCalculator::calculate_round_insert(const CncInputs& inputs, double D_cap, CncOutputs& outputs) {
    double radius = inputs.ic / 2.0;

    // Scenario 1: Round Insert / Button Cutter
    if (inputs.ap <= radius) {
        // Shallow depth
        // Determine if cut is ALSO radially narrow
        if (((inputs.ae + 0.01) * 2) <= D_cap) {
            // Double Thinning (Shallow Depth + Narrow Width)
            double chord_height_ap = std::sqrt(inputs.ap * inputs.ic - std::pow(inputs.ap, 2));
            double chord_height_ae = std::sqrt(D_cap * inputs.ae - std::pow(inputs.ae, 2));

            if (chord_height_ap > 0 && chord_height_ae > 0) {
                outputs.fz = (inputs.hex * inputs.ic * D_cap) / (4 * chord_height_ap * chord_height_ae);
            }
        } else {
            // Shallow Depth ONLY
            double chord_height_ap = std::sqrt(inputs.ap * inputs.ic - std::pow(inputs.ap, 2));
             if (chord_height_ap > 0) {
                outputs.fz = (inputs.hex * inputs.ic) / (2 * chord_height_ap);
             }
        }
    } else {
        // Deep Cut (Depth > Radius)
        if (((inputs.ae + 0.01) * 2) <= D_cap) {
            // Deep Depth + Narrow Width
            // sqrt(radius * ic - radius^2) is mathematically equal to radius (ic/2).
            double chord_height_radius = std::sqrt(radius * inputs.ic - std::pow(radius, 2));
            double chord_height_ae = std::sqrt(D_cap * inputs.ae - std::pow(inputs.ae, 2));
             if (chord_height_radius > 0 && chord_height_ae > 0) {
                outputs.fz = (inputs.hex * inputs.ic * D_cap) / (4 * chord_height_radius * chord_height_ae);
             }
        } else {
            // No Thinning required (Standard)
            // fz = (hex * ic) / (2 * sqrt(radius * ic - radius^2))
            // Simplified: fz = hex
            double chord_height_radius = std::sqrt(radius * inputs.ic - std::pow(radius, 2));
            if (chord_height_radius > 0) {
                outputs.fz = (inputs.hex * inputs.ic) / (2 * chord_height_radius);
            } else {
                outputs.fz = inputs.hex;
            }
        }
    }
}

void CncCalculator::calculate_angled_cutter(const CncInputs& inputs, double D_cap, CncOutputs& outputs) {
    // Scenario 2: Angled Cutter
    double Kr_rad = toRadians(inputs.Kr);

    if (inputs.ae < (D_cap / 2.0)) {
        // Radial Thinning is active
        double denominator = 2 * std::sin(Kr_rad) * std::sqrt((D_cap * inputs.ae) - std::pow(inputs.ae, 2));
        if (denominator > 0) {
            outputs.fz = (inputs.hex * D_cap) / denominator;
        }
    } else {
        // Only Lead Angle Thinning
        double sin_Kr = std::sin(Kr_rad);
        if (sin_Kr > 0) {
            outputs.fz = inputs.hex / sin_Kr;
        }
    }
}

void CncCalculator::calculate_standard_mill(const CncInputs& inputs, double D_cap, CncOutputs& outputs) {
    // Scenario 3: Standard End Mill
    if (inputs.ae <= (inputs.Dc / 2.0)) {
        // Radial Thinning Factor
        if (inputs.Dc > 0 && inputs.ae > 0) {
             double thinning_factor = 1.0 / std::sqrt(inputs.ae / inputs.Dc);
             outputs.fz = inputs.hex * thinning_factor;
        } else {
            outputs.fz = inputs.hex;
        }
    } else {
        outputs.fz = inputs.hex; // No adjustment needed
    }
}
