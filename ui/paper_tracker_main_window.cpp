//
// Created by JellyfishKnight on 25-3-11.
//

// You may need to build the project (run Qt uic code generator) to get "ui_paper_tracker_main_window.h" resolved

#include "paper_tracker_main_window.hpp"

#include <QFile>

#include "ui_paper_tracker_main_window.h"


PaperTrackerMainWindow::PaperTrackerMainWindow(QWidget *parent) :
    QWidget(parent), ui(new Ui::PaperTrackerMainWindow) {
    ui->setupUi(this);
    // set logo
    QFile Logo = QFile("./resources/logo.png");
    Logo.open(QIODevice::ReadOnly);
    QPixmap pixmap;
    pixmap.loadFromData(Logo.readAll());
    Logo.close();
    ui->LOGOLabel->setPixmap(pixmap);
    ui->LOGOLabel->setScaledContents(true);
    ui->LOGOLabel->update();
    ui->LOGOLabel->setAlignment(Qt::AlignCenter);
    ui->LOGOLabel->setFixedSize(200, 200);
    ui->LOGOLabel->setGeometry(0, 0, 200, 200);
    ui->LOGOLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}

PaperTrackerMainWindow::~PaperTrackerMainWindow() {
    delete ui;
}
