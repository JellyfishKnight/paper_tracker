//
// Created by JellyfishKnight on 25-3-11.
//

#ifndef PAPER_TRACKER_MAIN_WINDOW_HPP
#define PAPER_TRACKER_MAIN_WINDOW_HPP

#include <config_writer.hpp>
#include <paper_eye_tracker_window.hpp>
#include <paper_face_tracker_window.hpp>
#include <QWidget>
#include <updater.hpp>

#include "ui_paper_tracker_main_window.h"

struct PaperTrackerConfig
{
    PaperFaceTrackerConfig face_tracker_config;
    PaperEyeTrackerConfig eye_tracker_config;

    REFLECT(face_tracker_config, eye_tracker_config);
};

class PaperTrackerMainWindow final : public QWidget {
public:
    explicit PaperTrackerMainWindow(QWidget *parent = nullptr);

    ~PaperTrackerMainWindow() override;
private slots:
    void onFaceTrackerButtonClicked();
    void onEyeTrackerButtonClicked();
    void onUpdateButtonClicked();

private:
    void connect_callbacks();
    std::shared_ptr<Updater> updater;
    std::shared_ptr<ConfigWriter> config_writer;
    PaperTrackerConfig config;

    Ui::PaperTrackerMainWindow ui{};
};


#endif //PAPER_TRACKER_MAIN_WINDOW_HPP
