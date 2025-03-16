//
// Created by JellyfishKnight on 25-3-11.
//
#include "paper_eye_tracker_window.hpp"
#include "ui_paper_eye_tracker_window.h"


PaperEyeTrackerWindow::PaperEyeTrackerWindow(PaperEyeTrackerConfig* config, QWidget *parent) :
    QWidget(parent) {
    if (instance == nullptr)
        instance = this;
    else
        throw std::exception("当前已经打开了眼追窗口，请不要重复打开");
    ui.setupUi(this);
    setFixedSize(883, 596);
}

PaperEyeTrackerWindow::~PaperEyeTrackerWindow() {
    instance = nullptr;
}
