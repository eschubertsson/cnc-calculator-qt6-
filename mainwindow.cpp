#include "mainwindow.h"
#include "cnc_logic.h"
#include <QIntValidator>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi();
    applyStyles();

    // Initial Calculate to show zeros
    // Warning: On Windows, calling calculate() immediately might trigger layout issues if setup isn't fully propagated,
    // though usually fine in constructor.
    // calculate();
    // Optimization: Let the user calculate manually or wait for show().
    // But to populate initial "0" values, we can manually set text or just call it.
    // Let's rely on default labels "-" until Calculated.
}

MainWindow::~MainWindow() {}

QLineEdit* MainWindow::createDoubleInput(const QString &placeholder) {
    QLineEdit *line = new QLineEdit();
    line->setValidator(new QDoubleValidator(0.0, 100000.0, 4, this));
    line->setPlaceholderText(placeholder);
    return line;
}

QLineEdit* MainWindow::createIntInput(const QString &placeholder) {
    QLineEdit *line = new QLineEdit();
    line->setValidator(new QIntValidator(1, 1000, this));
    line->setPlaceholderText(placeholder);
    return line;
}

void MainWindow::setupUi() {
    this->setWindowTitle("Sponfortynning Kalkulator");
    this->resize(900, 800);

    QWidget *centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // Title
    QLabel *titleLabel = new QLabel("Sponfortynning Kalkulator");
    titleLabel->setObjectName("title");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    mainLayout->addLayout(contentLayout);

    // --- Inputs Section ---
    QGroupBox *inputGroup = new QGroupBox("Parametere");
    QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(10);
    formLayout->setLabelAlignment(Qt::AlignRight);

    inputVc = createDoubleInput("f.eks 100");
    inputDc = createDoubleInput("f.eks 50");
    inputIc = createDoubleInput("Valgfri (0)");
    inputKr = createDoubleInput("Standard 90");
    inputAp = createDoubleInput("f.eks 2.0");
    inputAe = createDoubleInput("f.eks 10.0");
    inputZ = createIntInput("f.eks 4");

    // Defaults
    inputKr->setText("90");

    formLayout->addRow("Skjærhastighet (Vc) [m/min]:", inputVc);
    formLayout->addRow("Nominell Diameter (Dc) [mm]:", inputDc);
    formLayout->addRow("Skjærstørrelse (ic) [mm]:", inputIc);
    formLayout->addRow("Inngrepsvinkel (Kr) [°]:", inputKr);
    formLayout->addRow("Kuttdybde (ap) [mm]:", inputAp);
    formLayout->addRow("Kuttbredde (ae) [mm]:", inputAe);
    formLayout->addRow("Antall tenner (Z):", inputZ);

    inputLayout->addLayout(formLayout);

    // --- Feed Compensation Section ---
    QGroupBox *compGroup = new QGroupBox("Mating Kompensering");
    QVBoxLayout *compLayout = new QVBoxLayout(compGroup);
    QFormLayout *compForm = new QFormLayout();

    comboCompMode = new QComboBox();
    comboCompMode->addItem("Ingen", QVariant::fromValue(CompensationMode::None));
    comboCompMode->addItem("Innvendig Hull", QVariant::fromValue(CompensationMode::InternalHole));
    comboCompMode->addItem("Utvendig Tapp", QVariant::fromValue(CompensationMode::ExternalBoss));

    inputContourDiam = createDoubleInput("f.eks 100");

    compForm->addRow("Type:", comboCompMode);
    compForm->addRow("Diameter (Hull/Tapp) [mm]:", inputContourDiam);

    compLayout->addLayout(compForm);
    inputLayout->addWidget(compGroup);

    // --- Calculation Mode Section ---
    QGroupBox *modeGroup = new QGroupBox("Beregningsmodus");
    QVBoxLayout *modeLayout = new QVBoxLayout(modeGroup);

    radioModeHex = new QRadioButton("Beregn Mating (fz) fra Spontykkelse (hex)");
    radioModeFz = new QRadioButton("Beregn Spontykkelse (hex) fra Mating (fz)");
    radioModeHex->setChecked(true); // Default

    connect(radioModeHex, &QRadioButton::toggled, this, &MainWindow::updateMode);

    modeLayout->addWidget(radioModeHex);
    modeLayout->addWidget(radioModeFz);

    // Target Inputs
    QFormLayout *targetForm = new QFormLayout();
    inputHex = createDoubleInput("f.eks 0.1");
    inputFz = createDoubleInput("f.eks 0.2");

    targetForm->addRow("Ønsket spontykkelse (hex) [mm]:", inputHex);
    targetForm->addRow("Ønsket mating (fz) [mm/tann]:", inputFz);

    modeLayout->addLayout(targetForm);
    inputLayout->addWidget(modeGroup);

    // Calculate Button
    QPushButton *calcBtn = new QPushButton("Beregn");
    calcBtn->setDefault(true);
    connect(calcBtn, &QPushButton::clicked, this, &MainWindow::calculate);
    inputLayout->addStretch();
    inputLayout->addWidget(calcBtn);

    contentLayout->addWidget(inputGroup, 2);

    // --- Results Section ---
    QGroupBox *resultGroup = new QGroupBox("Resultater");
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);

    // Use pointers to members!
    // The previous lambda captured valDcap etc, but we must ensure we assign TO the member variable.
    // The previous code: auto createResultRow = [resultLayout](..., QLabel *&valLabel) ...
    // And called as: createResultRow(..., valDcap);
    // Since valDcap is passed by reference to pointer (*&), the 'new QLabel' inside lambda assigns to valDcap.
    // This looks correct C++.

    auto createResultRow = [resultLayout](const QString &label, QLabel *&valLabel, bool bold = false) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *lbl = new QLabel(label);
        valLabel = new QLabel("-");
        valLabel->setAlignment(Qt::AlignRight);
        valLabel->setObjectName(bold ? "resultValueHighlight" : "resultValue");
        row->addWidget(lbl);
        row->addStretch();
        row->addWidget(valLabel);
        resultLayout->addLayout(row);

        // Add a separator line
        QFrame *line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        line->setStyleSheet("background-color: #e0e0e0; max-height: 1px; border: none;");
        resultLayout->addWidget(line);
    };

    createResultRow("Effektiv Diameter (D_cap) [mm]:", valDcap);
    createResultRow("Spindelhastighet (n) [o/min]:", valN);
    createResultRow("Bordmating (Vf) [mm/min]:", valVf);
    createResultRow("Korrigert Mating (Vf) [mm/min]:", valVfCorrected, true);
    createResultRow("Sponfjerning (MRR) [cm³/min]:", valMrr);

    resultLayout->addStretch();

    // Add logic description area
    QLabel *infoLabel = new QLabel("Valgt algoritme bestemmes automatisk basert på input (Rundt skjær, Vinklet, eller Standard).");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #666; font-style: italic; margin-top: 10px;");
    resultLayout->addWidget(infoLabel);

    contentLayout->addWidget(resultGroup, 1);

    updateMode(); // Set initial state
}

void MainWindow::updateMode() {
    if (radioModeHex->isChecked()) {
        // Solve for Fz (Standard)
        inputHex->setEnabled(true);
        inputFz->setEnabled(false);
        inputHex->setStyleSheet("");
        inputFz->setStyleSheet("background-color: #f0f0f0; color: #555;");
    } else {
        // Solve for Hex (Reverse)
        inputHex->setEnabled(false);
        inputFz->setEnabled(true);
        inputHex->setStyleSheet("background-color: #f0f0f0; color: #555;");
        inputFz->setStyleSheet("");
    }
}

void MainWindow::calculate() {
    // Safety Checks for Pointers
    if (!inputVc || !inputDc || !inputIc || !inputKr || !inputAp || !inputAe || !inputZ || !inputHex || !inputFz || !comboCompMode || !inputContourDiam) return;
    if (!valDcap || !valN || !valVf || !valVfCorrected || !valMrr) return;

    CncInputs inputs;

    // Robust parsing
    // On Windows, QDoubleValidator might allow comma inputs but toDouble() expects dots in C locale.
    // Replace comma with dot is standard practice.
    // Also guard against empty strings.

    auto safeDouble = [](QLineEdit *le) -> double {
        QString txt = le->text().replace(',', '.');
        if (txt.isEmpty()) return 0.0;
        bool ok;
        double val = txt.toDouble(&ok);
        return ok ? val : 0.0;
    };

    auto safeInt = [](QLineEdit *le) -> int {
        QString txt = le->text();
        if (txt.isEmpty()) return 0;
        return txt.toInt();
    };

    inputs.Vc = safeDouble(inputVc);
    inputs.Dc = safeDouble(inputDc);
    inputs.ic = safeDouble(inputIc);
    inputs.Kr = safeDouble(inputKr);
    if (inputs.Kr == 0) inputs.Kr = 90.0; // Default safety
    inputs.ap = safeDouble(inputAp);
    inputs.ae = safeDouble(inputAe);
    inputs.Z = safeInt(inputZ);

    // Mode
    if (radioModeHex->isChecked()) {
        inputs.mode = CalculationMode::SolveForFz;
        inputs.hex = safeDouble(inputHex);
    } else {
        inputs.mode = CalculationMode::SolveForHex;
        inputs.fz_input = safeDouble(inputFz);
    }

    // Compensation
    int compIndex = comboCompMode->currentIndex();
    if (compIndex == 1) inputs.compMode = CompensationMode::InternalHole;
    else if (compIndex == 2) inputs.compMode = CompensationMode::ExternalBoss;
    else inputs.compMode = CompensationMode::None;

    inputs.contourDiameter = safeDouble(inputContourDiam);

    CncOutputs outputs = CncCalculator::calculate(inputs);

    // Update the Read-Only field with the result
    if (radioModeHex->isChecked()) {
        inputFz->setText(QString::number(outputs.fz, 'f', 4));
    } else {
        inputHex->setText(QString::number(outputs.hex, 'f', 4));
    }

    valDcap->setText(QString::number(outputs.D_cap, 'f', 2));
    valN->setText(QString::number(outputs.n, 'f', 0));
    valVf->setText(QString::number(outputs.Vf, 'f', 0));
    valVfCorrected->setText(QString::number(outputs.Vf_corrected, 'f', 0));
    valMrr->setText(QString::number(outputs.MRR, 'f', 2));
}

void MainWindow::applyStyles() {
    QString qss = R"(
        QMainWindow {
            background-color: #f3f3f3;
            color: #1a1a1a;
            font-family: "Segoe UI", sans-serif;
        }
        QGroupBox {
            background-color: white;
            border-radius: 8px;
            border: 1px solid #d1d1d1;
            margin-top: 24px;
            font-weight: bold;
            font-size: 14px;
            color: #0067c0;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 10px;
            left: 10px;
        }
        QLineEdit, QComboBox {
            background-color: white;
            border: 1px solid #d1d1d1;
            border-radius: 4px;
            padding: 8px;
            font-size: 13px;
        }
        QLineEdit:focus, QComboBox:focus {
            border: 1px solid #0067c0;
        }
        QLabel {
            font-size: 13px;
            color: #333;
        }
        QLabel#title {
            font-size: 24px;
            font-weight: bold;
            color: #1a1a1a;
            margin-bottom: 20px;
        }
        QLabel#resultValue {
            font-weight: bold;
            font-size: 15px;
            color: #555;
        }
        QLabel#resultValueHighlight {
            font-weight: bold;
            font-size: 16px;
            color: #0067c0;
        }
        QPushButton {
            background-color: #0067c0;
            color: white;
            border-radius: 6px;
            padding: 10px 20px;
            font-weight: bold;
            font-size: 14px;
            border: none;
        }
        QPushButton:hover {
            background-color: #005a9e;
        }
        QPushButton:pressed {
            background-color: #004b87;
        }
        QRadioButton {
            font-size: 13px;
            color: #333;
            padding: 4px;
        }
    )";
    this->setStyleSheet(qss);
}
