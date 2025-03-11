//
// Created by JellyfishKnight on 25-3-11.
//

#ifndef PAPER_EYE_TRACKER_WINDOW_HPP
#define PAPER_EYE_TRACKER_WINDOW_HPP

#include <QWidget>
#include "ui_paper_eye_tracker_window.h"

class PaperEyeTrackerWindow : public QWidget {
public:
    explicit PaperEyeTrackerWindow(QWidget *parent = nullptr);
    ~PaperEyeTrackerWindow() override;

private:
    Ui::PaperEyeTrackerWindow ui{};
};


#endif //PAPER_EYE_TRACKER_WINDOW_HPP
