#pragma once

#include <QMainWindow>
#include <QLineEdit>

#include <QDebug>

#include "exactcoverbuilder.h"
#include "dlx.h"

using GridRow = QList<QLineEdit *>;
using Grid = QList<GridRow>;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    static const int CellSize;

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void generateGrid(int size);
    void fillGridWithTestData();

private:
    Ui::MainWindow *ui;

    Grid grid;

private slots:
    void onCellTextEdited(const QString &text);
    void on_pushButtonReset_clicked();
    void on_pushButtonSolve_clicked();
};
