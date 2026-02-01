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

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void calculate();

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
    QLineEdit *inputHex;
    QLineEdit *inputZ;

    // Output Labels
    QLabel *valDcap;
    QLabel *valN;
    QLabel *valFz;
    QLabel *valVf;
    QLabel *valMrr;

    // Helper to create inputs
    QLineEdit* createDoubleInput(const QString &placeholder = "");
    QLineEdit* createIntInput(const QString &placeholder = "");
};

#endif // MAINWINDOW_H
