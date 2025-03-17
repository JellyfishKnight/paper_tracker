//
// Created by JellyfishKnight on 25-3-11.
//

#ifndef PAPER_TRACKER_MAIN_WINDOW_HPP
#define PAPER_TRACKER_MAIN_WINDOW_HPP

#include <eye_tracker_window.hpp>
#include <face_tracker_window.hpp>
#include <QWidget>
#include <updater.hpp>

#include "ui_main_window.h"

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

    Ui::PaperTrackerMainWindow ui{};
};


#endif //PAPER_TRACKER_MAIN_WINDOW_HPP
