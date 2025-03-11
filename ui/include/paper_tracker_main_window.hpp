//
// Created by JellyfishKnight on 25-3-11.
//

#ifndef PAPER_TRACKER_MAIN_WINDOW_HPP
#define PAPER_TRACKER_MAIN_WINDOW_HPP

#include <QWidget>
#include "ui_paper_tracker_main_window.h"

class PaperTrackerMainWindow : public QWidget {
public:
    explicit PaperTrackerMainWindow(QWidget *parent = nullptr);

    ~PaperTrackerMainWindow() override;

private:
    Ui::PaperTrackerMainWindow *ui;
};


#endif //PAPER_TRACKER_MAIN_WINDOW_HPP
