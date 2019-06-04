#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QValidator>
#include <QLabel>

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

    ui->gridLayoutSudoku->setSpacing(0);

    const QIntValidator *validator = new QIntValidator(1, size, this);

/*    for(int mr = 0; mr < 3; mr++)
    {
        for(int mc = 0; mc < 3; mc++)
        {
            QFrame *widget = new QFrame;
            widget->setFrameStyle(QFrame::Plain);
            widget->setFrameShape(QFrame::Box);

            QGridLayout *gridLayout = new QGridLayout(widget);
            gridLayout->setSpacing(0);
            gridLayout->setMargin(0);

            for(int r = 0; r < 3; r++)
            {
                for(int c = 0; c < 3; c++)
                {
                    QLineEdit *item = new QLineEdit(this);

                    gridLayout->addWidget(item, r, c, 1, 1, Qt::AlignCenter);
                }
            }

            ui->gridLayoutSudoku->addWidget(widget, mr, mc, 1, 1, Qt::AlignCenter);
        }
    }*/

    /*for (int i = 0; i < size; ++i) {
        grid.append(Cell());

        for (int j = 0; j < size; ++j) {
            QLineEdit *item = new QLineEdit(this);
            item->setValidator(validator);

            ui->gridLayoutSudoku->addWidget(item, i, j);

            grid[i].append(item);
        }
    }*/
}
