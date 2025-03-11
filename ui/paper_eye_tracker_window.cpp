//
// Created by JellyfishKnight on 25-3-11.
//

// You may need to build the project (run Qt uic code generator) to get "ui_paper_eye_tracker_window.h" resolved

#include "paper_eye_tracker_window.hpp"
#include "ui_paper_eye_tracker_window.h"


PaperEyeTrackerWindow::PaperEyeTrackerWindow(QWidget *parent) :
    QWidget(parent) {
    ui.setupUi(this);
    setFixedSize(671, 442);
}

PaperEyeTrackerWindow::~PaperEyeTrackerWindow() = default;
