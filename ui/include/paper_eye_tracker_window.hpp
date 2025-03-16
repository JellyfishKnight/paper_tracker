//
// Created by JellyfishKnight on 25-3-11.
//

#ifndef PAPER_EYE_TRACKER_WINDOW_HPP
#define PAPER_EYE_TRACKER_WINDOW_HPP

#include "ui_paper_eye_tracker_window.h"
#include "serial.hpp"
#include "image_downloader.hpp"
#include "osc.hpp"
#include "logger.hpp"
#include <QTimer>
#include "config_writer.hpp"

struct PaperEyeTrackerConfig
{

    REFLECT();
};

class PaperEyeTrackerWindow : public QWidget {
public:
    explicit PaperEyeTrackerWindow(PaperEyeTrackerConfig* config, QWidget *parent = nullptr);
    ~PaperEyeTrackerWindow() override;

private:
    Ui::PaperEyeTrackerWindow ui{};

    inline static PaperEyeTrackerWindow* instance = nullptr;
};


#endif //PAPER_EYE_TRACKER_WINDOW_HPP
