#include "mainwindow.h"
#include "cnc_logic.h"
#include <QIntValidator>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi();
    applyStyles();

    // Initial Calculate to show zeros
    calculate();
}

MainWindow::~MainWindow() {}

QLineEdit* MainWindow::createDoubleInput(const QString &placeholder) {
    QLineEdit *line = new QLineEdit();
    line->setValidator(new QDoubleValidator(0.0, 100000.0, 4, this));
    line->setPlaceholderText(placeholder);
    connect(line, &QLineEdit::editingFinished, this, &MainWindow::calculate);
    return line;
}

QLineEdit* MainWindow::createIntInput(const QString &placeholder) {
    QLineEdit *line = new QLineEdit();
    line->setValidator(new QIntValidator(1, 1000, this));
    line->setPlaceholderText(placeholder);
    connect(line, &QLineEdit::editingFinished, this, &MainWindow::calculate);
    return line;
}

void MainWindow::setupUi() {
    this->setWindowTitle("Sponfortynning Kalkulator");
    this->resize(800, 600);

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
    inputHex = createDoubleInput("f.eks 0.1");
    inputZ = createIntInput("f.eks 4");

    // Defaults
    inputKr->setText("90");

    formLayout->addRow("Skjærhastighet (Vc) [m/min]:", inputVc);
    formLayout->addRow("Nominell Diameter (Dc) [mm]:", inputDc);
    formLayout->addRow("Skjærstørrelse (ic) [mm]:", inputIc);
    formLayout->addRow("Inngrepsvinkel (Kr) [°]:", inputKr);
    formLayout->addRow("Kuttdybde (ap) [mm]:", inputAp);
    formLayout->addRow("Kuttbredde (ae) [mm]:", inputAe);
    formLayout->addRow("Ønsket spontykkelse (hex) [mm]:", inputHex);
    formLayout->addRow("Antall tenner (Z):", inputZ);

    inputLayout->addLayout(formLayout);

    // Calculate Button
    QPushButton *calcBtn = new QPushButton("Beregn");
    connect(calcBtn, &QPushButton::clicked, this, &MainWindow::calculate);
    inputLayout->addStretch();
    inputLayout->addWidget(calcBtn);

    contentLayout->addWidget(inputGroup, 2);

    // --- Results Section ---
    QGroupBox *resultGroup = new QGroupBox("Resultater");
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);

    auto createResultRow = [resultLayout](const QString &label, QLabel *&valLabel) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *lbl = new QLabel(label);
        valLabel = new QLabel("-");
        valLabel->setAlignment(Qt::AlignRight);
        valLabel->setObjectName("resultValue");
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
    createResultRow("Mating pr. tann (fz) [mm/tann]:", valFz);
    createResultRow("Bordmating (Vf) [mm/min]:", valVf);
    createResultRow("Sponfjerning (MRR) [cm³/min]:", valMrr);

    resultLayout->addStretch();

    // Add logic description area
    QLabel *infoLabel = new QLabel("Valgt algoritme bestemmes automatisk basert på input (Rundt skjær, Vinklet, eller Standard).");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #666; font-style: italic; margin-top: 10px;");
    resultLayout->addWidget(infoLabel);

    contentLayout->addWidget(resultGroup, 1);
}

void MainWindow::calculate() {
    CncInputs inputs;
    inputs.Vc = inputVc->text().replace(',', '.').toDouble();
    inputs.Dc = inputDc->text().replace(',', '.').toDouble();
    inputs.ic = inputIc->text().replace(',', '.').toDouble();
    inputs.Kr = inputKr->text().replace(',', '.').toDouble();
    if (inputs.Kr == 0) inputs.Kr = 90.0; // Default safety
    inputs.ap = inputAp->text().replace(',', '.').toDouble();
    inputs.ae = inputAe->text().replace(',', '.').toDouble();
    inputs.hex = inputHex->text().replace(',', '.').toDouble();
    inputs.Z = inputZ->text().toInt();

    CncOutputs outputs = CncCalculator::calculate(inputs);

    valDcap->setText(QString::number(outputs.D_cap, 'f', 2));
    valN->setText(QString::number(outputs.n, 'f', 0));
    valFz->setText(QString::number(outputs.fz, 'f', 4));
    valVf->setText(QString::number(outputs.Vf, 'f', 0));
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
        QLineEdit {
            background-color: white;
            border: 1px solid #d1d1d1;
            border-radius: 4px;
            padding: 8px;
            font-size: 13px;
        }
        QLineEdit:focus {
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
    )";
    this->setStyleSheet(qss);
}
