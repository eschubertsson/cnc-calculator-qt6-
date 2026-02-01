#include "cnc_logic.h"
#include <iostream>
#include <iomanip>

int main() {
    // Case A (45 Facemill but ic present -> Round Insert Logic)
    // Inputs: hex=0.2, Ap=1, Ae=100, D3=112, ic=12, Vc=100, Z=8
    CncInputs caseA;
    caseA.hex = 0.2;
    caseA.ap = 1.0;
    caseA.ae = 100.0;
    caseA.Dc = 112.0;
    caseA.ic = 12.0;
    caseA.Vc = 100.0;
    caseA.Z = 8;
    // Kr not specified in validation data, but logic uses ic if present.

    CncOutputs outA = CncCalculator::calculate(caseA);
    std::cout << "Case A:" << std::endl;
    std::cout << "D_cap: " << outA.D_cap << " (Exp: 106.63)" << std::endl;
    std::cout << "fz: " << outA.fz << " (Exp: 0.36)" << std::endl;
    std::cout << "Vf: " << outA.Vf << " (Exp: 864)" << std::endl;
    std::cout << "n: " << outA.n << std::endl;

    // Case B (High Feed / Round Insert)
    // Inputs: hex=0.125, Ap=4, Ae=30, D3=80, Vc=2010.5, Z=8
    // I inferred ic=8
    CncInputs caseB;
    caseB.hex = 0.125;
    caseB.ap = 4.0;
    caseB.ae = 30.0;
    caseB.Dc = 80.0;
    caseB.Vc = 2010.5;
    caseB.Z = 8;
    caseB.ic = 8.0; // Inferred

    CncOutputs outB = CncCalculator::calculate(caseB);
    std::cout << "\nCase B:" << std::endl;
    std::cout << "D_cap: " << outB.D_cap << std::endl;
    std::cout << "n: " << outB.n << " (Exp: 7999)" << std::endl;
    std::cout << "fz: " << outB.fz << " (Exp: 0.129)" << std::endl;
    std::cout << "Vf: " << outB.Vf << " (Exp: 8261)" << std::endl;

    return 0;
}
