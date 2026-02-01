#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QDoubleValidator>
#include <QRadioButton>
#include <QButtonGroup>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void calculate();
    void updateMode();

private:
    void setupUi();
    void applyStyles();

    // Input Fields
    QLineEdit *inputVc;
    QLineEdit *inputDc;
    QLineEdit *inputIc;
    QLineEdit *inputKr;
    QLineEdit *inputAp;
    QLineEdit *inputAe;
    QLineEdit *inputHex; // Input for hex
    QLineEdit *inputFz;  // Input for fz (was label)
    QLineEdit *inputZ;

    // Mode Selection
    QRadioButton *radioModeHex; // Solve for fz (Lock hex)
    QRadioButton *radioModeFz;  // Solve for hex (Lock fz)

    // Output Labels
    QLabel *valDcap;
    QLabel *valN;
    QLabel *valVf;
    QLabel *valMrr;

    // Helper to create inputs
    QLineEdit* createDoubleInput(const QString &placeholder = "");
    QLineEdit* createIntInput(const QString &placeholder = "");
};

#endif // MAINWINDOW_H
