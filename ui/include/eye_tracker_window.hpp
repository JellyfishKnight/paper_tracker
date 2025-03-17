//
// Created by JellyfishKnight on 25-3-11.
//

#ifndef PAPER_EYE_TRACKER_WINDOW_HPP
#define PAPER_EYE_TRACKER_WINDOW_HPP

#include "ui_eye_tracker_window.h"
#include "serial.hpp"
#include "image_downloader.hpp"
#include "osc.hpp"
#include "logger.hpp"
#include <QTimer>
#include "config_writer.hpp"

#define FACE_TAG 1
#define LEFT_TAG 2
#define RIGHT_TAG 3

struct PaperEyeTrackerConfig
{
    std::string left_ip;
    std::string right_ip;
    REFLECT(left_ip, right_ip);
};

class PaperEyeTrackerWindow : public QWidget {
public:
    explicit PaperEyeTrackerWindow(QWidget *parent = nullptr);
    ~PaperEyeTrackerWindow() override;

    std::string getSSID() const;
    std::string getPassword() const;

    int get_max_fps() const;
    bool is_running() const;

    void set_config() const;
    void setIPText(int version, const QString& text) const;
    void start_image_download(int version) const;
    void setSerialStatusLabel(const QString& text) const;
    void setWifiStatusLabel(int version, const QString& text) const;

    void setVideoImage(int version, const cv::Mat& image);
    void updateWifiLabel(int version) const;
    void updateSerialLabel(int version) const;
    cv::Mat getVideoImage(int version) const;

    PaperEyeTrackerConfig generate_config() const;
private slots:
    void onSendButtonClicked();
    void onRestartButtonClicked();
    void onFlashButtonClicked();

private:
    void connect_callbacks();

    void create_sub_thread();

    Ui::PaperEyeTrackerWindow ui{};

    std::shared_ptr<ESP32VideoStream> left_image_stream, right_image_stream;
    std::shared_ptr<SerialPortManager> serial_port_;
    std::shared_ptr<OscManager> osc_manager;

    // 2 is left, 3 is right
    int current_esp32_version = 0;

    inline static PaperEyeTrackerWindow* instance = nullptr;

    std::thread update_left_ui_thread;
    std::thread update_right_ui_thread;
    bool app_is_running = true;
    int max_fps = 38;

    std::string current_left_ip;
    std::string current_right_ip;

    std::shared_ptr<ConfigWriter> config_writer;
    PaperEyeTrackerConfig config;
};


#endif //PAPER_EYE_TRACKER_WINDOW_HPP
