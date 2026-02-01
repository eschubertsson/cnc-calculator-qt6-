#include <QtTest>
#include "../cnc_logic.h"

class ValidationTest : public QObject {
    Q_OBJECT

private slots:
    void testCaseA();
    void testCaseB();
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
    inputs.Kr = 90.0; // Not used but good practice

    CncOutputs outputs = CncCalculator::calculate(inputs);

    // Expected: Dcap=106.63, Fz=0.36, Vf=864
    QVERIFY(qAbs(outputs.D_cap - 106.63) < 0.1);
    QVERIFY(qAbs(outputs.fz - 0.36) < 0.01);
    QVERIFY(qAbs(outputs.Vf - 864.0) < 1.0);
}

void ValidationTest::testCaseB() {
    // Inputs: hex=0.125, Ap=4, Ae=30, D3=80, Vc=2010.5, Z=8
    // Inferred ic=8
    CncInputs inputs;
    inputs.hex = 0.125;
    inputs.ap = 4.0;
    inputs.ae = 30.0;
    inputs.Dc = 80.0;
    inputs.Vc = 2010.5;
    inputs.Z = 8;
    inputs.ic = 8.0;

    CncOutputs outputs = CncCalculator::calculate(inputs);

    // Expected: Fz=0.129, Rpm=7999, Vf=8261
    QVERIFY(qAbs(outputs.fz - 0.129) < 0.001);
    QVERIFY(qAbs(outputs.n - 7999.0) < 5.0); // 7999.5 vs 7999
    QVERIFY(qAbs(outputs.Vf - 8261.0) < 5.0);
}

QTEST_MAIN(ValidationTest)
#include "tst_validation.moc"
