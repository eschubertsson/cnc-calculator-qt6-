#include <QtTest>
#include "../cnc_logic.h"

class ValidationTest : public QObject {
    Q_OBJECT

private slots:
    void testCaseA();
    void testCaseB();
    void testReverseCalculation();
    void testFeedCompensation();
    void testEmptyInputs();
    void testZeroInputs();
};

void ValidationTest::testCaseA() {
    // Inputs: hex=0.2, Ap=1, Ae=100, D3=112, ic=12, Vc=100, Z=8
    CncInputs inputs;
    inputs.hex = 0.2;
    inputs.ap = 1.0;
    inputs.ae = 100.0;
    inputs.Dc = 112.0;
    inputs.ic = 12.0;
    inputs.Vc = 100.0;
    inputs.Z = 8;
    inputs.Kr = 90.0;
    inputs.mode = CalculationMode::SolveForFz;

    CncOutputs outputs = CncCalculator::calculate(inputs);

    // Expected: Dcap=106.63, Fz=0.36, Vf=864
    QVERIFY(qAbs(outputs.D_cap - 106.63) < 0.1);
    QVERIFY(qAbs(outputs.fz - 0.36) < 0.01);
    QVERIFY(qAbs(outputs.Vf - 864.0) < 1.0);
}

void ValidationTest::testCaseB() {
    // Inputs: hex=0.125, Ap=4, Ae=30, D3=80, Vc=2010.5, Z=8
    CncInputs inputs;
    inputs.hex = 0.125;
    inputs.ap = 4.0;
    inputs.ae = 30.0;
    inputs.Dc = 80.0;
    inputs.Vc = 2010.5;
    inputs.Z = 8;
    inputs.ic = 8.0;
    inputs.mode = CalculationMode::SolveForFz;

    CncOutputs outputs = CncCalculator::calculate(inputs);

    // Expected: Fz=0.129, Rpm=7999, Vf=8261
    QVERIFY(qAbs(outputs.fz - 0.129) < 0.001);
    QVERIFY(qAbs(outputs.n - 7999.0) < 5.0);
    QVERIFY(qAbs(outputs.Vf - 8261.0) < 5.0);
}

void ValidationTest::testReverseCalculation() {
    // Reverse of Case A
    CncInputs inputs;
    inputs.fz_input = 0.361814;
    inputs.ap = 1.0;
    inputs.ae = 100.0;
    inputs.Dc = 112.0;
    inputs.ic = 12.0;
    inputs.Vc = 100.0;
    inputs.Z = 8;
    inputs.mode = CalculationMode::SolveForHex;

    CncOutputs outputs = CncCalculator::calculate(inputs);

    QVERIFY(qAbs(outputs.hex - 0.2) < 0.001);
    QVERIFY(qAbs(outputs.Vf - 864.0) < 1.0);
}

void ValidationTest::testFeedCompensation() {
    CncInputs inputs;
    inputs.Vc = 100;
    inputs.Dc = 10.0;
    inputs.Z = 1;
    inputs.mode = CalculationMode::SolveForHex;
    inputs.fz_input = 1.0;
    inputs.compMode = CompensationMode::InternalHole;
    inputs.contourDiameter = 20.0;

    CncOutputs outputs = CncCalculator::calculate(inputs);

    QVERIFY(qAbs(outputs.Vf_corrected - (outputs.Vf * 0.5)) < 0.1);

    inputs.compMode = CompensationMode::ExternalBoss;
    outputs = CncCalculator::calculate(inputs);

    QVERIFY(qAbs(outputs.Vf_corrected - (outputs.Vf * 1.5)) < 0.1);
}

void ValidationTest::testEmptyInputs() {
    // Verify that calculating with default (0) inputs does not crash
    CncInputs inputs; // All defaults are 0
    CncOutputs outputs = CncCalculator::calculate(inputs);

    // Check results are safe (0 or NaN handled)
    QVERIFY(outputs.n == 0.0);
    QVERIFY(outputs.Vf == 0.0);
    QVERIFY(outputs.Vf_corrected == 0.0);
}

void ValidationTest::testZeroInputs() {
    // Specifically test potential division by zero scenarios
    CncInputs inputs;
    inputs.Vc = 100;
    inputs.Dc = 0.0; // Bad Dc
    inputs.ap = 0.0;
    inputs.ae = 0.0;
    inputs.Z = 0;

    // Compensation with 0 diameter
    inputs.compMode = CompensationMode::InternalHole;
    inputs.contourDiameter = 0.0; // Crash risk!

    CncOutputs outputs = CncCalculator::calculate(inputs);
    QVERIFY(outputs.Vf_corrected == 0.0); // Should be 0 and not crash
}

QTEST_MAIN(ValidationTest)
#include "tst_validation.moc"
