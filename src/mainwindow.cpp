#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QValidator>
#include <QLabel>

const int MainWindow::CellSize = 50;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    generateGrid(9);

    // Debug
    fillGridWithTestData();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::generateGrid(int size) {
    if (size % 3 != 0) {
        qCritical() << "Grid size not divisible by 3!";
        return;
    }

    int sectionSize = size / 3;
    const QIntValidator *validator = new QIntValidator(1, size, this);

    // Initialize all grid rows
    // For later population (different order, as we add section by section)
    grid.reserve(size * size);
    for (int i = 0; i < size; ++i) {
        grid.append(GridRow());
    }

    // Add section by section and fill with cells
    // to achieve wanted styling
    for (int si = 0; si < sectionSize; ++si) {
        for (int sj = 0; sj < sectionSize; ++sj) {
            QFrame *widget = new QFrame;
            widget->setFrameShape(QFrame::Box);

            int widgetSize = CellSize * 3 + 2; // 2 for border spaces
            widget->setMinimumSize(widgetSize, widgetSize);
            widget->setMaximumSize(widgetSize, widgetSize);

            QGridLayout *gridLayout = new QGridLayout(widget);
            gridLayout->setSpacing(0);
            gridLayout->setMargin(0);

            for (int i = 0; i < sectionSize; ++i) {
                for (int j = 0; j < sectionSize; ++j) {
                    QLineEdit *cell = new QLineEdit(this);
                    cell->setAlignment(Qt::AlignCenter);
                    cell->setFont(QFont(cell->font().family(), CellSize / 2));
                    cell->setStyleSheet("QLineEdit { border: 1px solid grey; }");

                    cell->setValidator(validator);
                    cell->setMinimumSize(CellSize, CellSize);
                    cell->setMaximumSize(CellSize, CellSize);

                    gridLayout->addWidget(cell, i, j);

                    // Calculate row index from section and cell positions
                    // We add section by section and fill section as it's created
                    // but we have to add to the row in the entire grid
                    int rowIndex = i + si * sectionSize;
                    grid[rowIndex].append(cell);

                    connect(cell, &QLineEdit::textEdited, this, &MainWindow::onCellTextEdited);
                }
            }

            ui->gridLayoutSudoku->addWidget(widget, si, sj);
        }
    }
}

void MainWindow::fillGridWithTestData() {
    if (grid.size() == 9) {
        int testData[9][9] = {
            { 0,0,0,  0,0,0,  0,0,0 },
            { 0,0,0,  0,0,3,  0,8,5 },
            { 0,0,1,  0,2,0,  0,0,0 },

            { 0,0,0,  5,0,7,  0,0,0 },
            { 0,0,4,  0,0,0,  1,0,0 },
            { 0,9,0,  0,0,0,  0,0,0 },

            { 5,0,0,  0,0,0,  0,7,3 },
            { 0,0,2,  0,1,0,  0,0,0 },
            { 0,0,0,  0,4,0,  0,0,9 }
        };

        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                if (testData[i][j] != 0) {
                    int data = testData[i][j];
                    grid[i][j]->setText(QString::number(data));
                }
            }
        }
    }
}

// Slots
void MainWindow::onCellTextEdited(const QString &text) {
    // Manual low-bound validation (validator doesn't handle it)
    if (text.toInt() < 1) {
        for (auto &row : grid) {
            for (auto &cell : row) {
                if (cell->text() != "" && cell->text().toInt() < 1) {
                    cell->setText("1");
                }
            }
        }
    }
}

void MainWindow::on_pushButtonReset_clicked() {
    for (auto &row : grid) {
        for (auto &cell : row) {
            cell->setText("");
        }
    }
}

void MainWindow::on_pushButtonSolve_clicked() {
    // Convert Sudoku grid to exact cover problem
    ExactCoverBuilder ecBuilder();
    //ecBuilder->build();

    // Solve using Dancing Links / Algorithm X (DLX)
    DLX dlx();
    //dlx->solve();
}
