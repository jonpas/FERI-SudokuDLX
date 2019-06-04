#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QValidator>
#include <QLabel>

const QSize MainWindow::CellSize = {50, 50};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    generateGrid(9);
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

    const QIntValidator *validator = new QIntValidator(1, size, this); // TODO Check 0 input

    for (int mr = 0; mr < sectionSize; mr++) {
        for (int mc = 0; mc < sectionSize; mc++) {
            QFrame *widget = new QFrame;
            widget->setFrameShape(QFrame::Box);
            widget->setMinimumSize(CellSize * 3);
            widget->setMaximumSize(CellSize * 3);

            QGridLayout *gridLayout = new QGridLayout(widget);
            gridLayout->setSpacing(0);
            gridLayout->setMargin(0);

            for (int r = 0; r < sectionSize; r++) {
                grid.append(Cell());

                for (int c = 0; c < sectionSize; c++) {
                    QLineEdit *item = new QLineEdit(this);
                    item->setAlignment(Qt::AlignCenter);
                    item->setFont(QFont(item->font().family(), CellSize.height() / 2));
                    item->setValidator(validator);
                    item->setMinimumSize(CellSize);

                    gridLayout->addWidget(item, r, c);

                    grid[r].append(item);
                    connect(item, &QLineEdit::textEdited, this, &MainWindow::on_cell_textEdited);
                }
            }

            ui->gridLayoutSudoku->addWidget(widget, mr, mc);
        }
    }
}

// Slots
void MainWindow::on_cell_textEdited(const QString &text) {
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
