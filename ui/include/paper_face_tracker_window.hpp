//
// Created by JellyfishKnight on 25-3-11.
//

#ifndef PAPER_FACE_TRACKER_WINDOW_HPP
#define PAPER_FACE_TRACKER_WINDOW_HPP

#include <QProcess>
#include <thread>
#include <future>
#include <atomic>
#include <algorithm>
#include <config_writer.hpp>
#include <image_downloader.hpp>
#include <osc.hpp>
#include <QTimer>

#include "ui_paper_face_tracker_window.h"
#include "inference.hpp"
#include "serial.hpp"
#include "logger.hpp"
#include "reflect.hpp"
#include "updater.hpp"

struct Rect
{
    Rect() = default;

    Rect(int x, int y, int width, int height) :
        rect(x, y, width, height), x(x), y(y), width(width), height(height) {}

    // delete operate = to avoid copy
    Rect& operator=(const Rect& other)
    {
        this->rect = cv::Rect(other.x, other.y, other.width, other.height);
        this->x = other.x;
        this->y = other.y;
        this->width = other.width;
        this->height = other.height;
        return *this;
    }

    explicit Rect(cv::Rect rect) : rect(rect), x(rect.x), y(rect.y), width(rect.width), height(rect.height) {}

    cv::Rect rect;
    bool is_roi_end = true;

    int x{0};
    int y{0};
    int width{0};
    int height{0};

    REFLECT(x, y, width, height);
};

struct PaperFaceTrackerConfig
{
    int brightness;
    int rotate_angle;
    int energy_mode;
    std::string wifi_ip;
    bool use_filter;
    std::unordered_map<std::string, int> amp_map;
    Rect rect;

    REFLECT(brightness, rotate_angle, energy_mode, wifi_ip, use_filter, amp_map, rect);
};

class PaperFaceTrackMainWindow : public QWidget {
private:
    // UI组件
    Ui::PaperFaceTrackerMainWindow ui{};
public:
    explicit PaperFaceTrackMainWindow(QWidget *parent = nullptr);
    ~PaperFaceTrackMainWindow() override;

    void setSerialStatusLabel(const QString& text) const;
    void setWifiStatusLabel(const QString& text) const;
    void setIPText(const QString& text) const;

    QPlainTextEdit* getLogText() const;
    Rect getRoiRect();
    float getRotateAngle() const;
    std::string getSSID() const;
    std::string getPassword() const;

    void setVideoImage(const cv::Mat& image);
    // 根据模型输出更新校准页面的进度条
    void updateCalibrationProgressBars(
        const std::vector<float>& output,
        const std::unordered_map<std::string, size_t>& blendShapeIndexMap
    );

    using FuncWithoutArgs = std::function<void()>;
    using FuncWithVal = std::function<void(int)>;
    // let user decide what to do with these action
    void setOnUseFilterClickedFunc(FuncWithVal func);
    void setOnSaveConfigButtonClickedFunc(FuncWithoutArgs func);
    void setOnAmpMapChangedFunc(FuncWithoutArgs func);
    void set_update_thread(FuncWithoutArgs func);
    void set_inference_thread(FuncWithoutArgs func);
    void set_osc_send_thead(FuncWithoutArgs func);

    bool is_running() const;

    void stop();
    int get_max_fps() const;

    PaperFaceTrackerConfig generate_config() const;

    void set_config(const PaperFaceTrackerConfig& config);

    std::unordered_map<std::string, int> getAmpMap() const;

    void updateWifiLabel() const;
    void updateSerialLabel() const;

    cv::Mat getVideoImage() const;
    std::string getFirmwareVersion() const;
    SerialStatus getSerialStatus() const;

private slots:
    void onBrightnessChanged(int value);
    void onSendBrightnessValue() const;
    void onRotateAngleChanged(int value);
    void onSendButtonClicked();
    void onRestartButtonClicked();
    void onUseFilterClicked(int value) const;
    void onFlashButtonClicked();
    void onEnergyModeChanged(int value);
    void onSaveConfigButtonClicked();

    void onCheeckPuffLeftChanged(int value) const;
    void onCheeckPuffRightChanged(int value) const;
    void onJawOpenChanged(int value);
    void onJawLeftChanged(int value);
    void onJawRightChanged(int value);
    void onMouthLeftChanged(int value) const;
    void onMouthRightChanged(int value);
    void onTongueOutChanged(int value);
    void onTongueLeftChanged(int value);
    void onTongueRightChanged(int value);
    void onTongueUpChanged(int value);
    void onTongueDownChanged(int value) const;

    void onCheckFirmwareVersionClicked();
    void onCheckClientVersionClicked();
private:
    void start_image_download() const;

    QProcess* vrcftProcess;
    FuncWithVal onRotateAngleChangedFunc;
    FuncWithVal onUseFilterClickedFunc;
    FuncWithoutArgs onSaveConfigButtonClickedFunc;
    FuncWithoutArgs onAmpMapChangedFunc;
    std::shared_ptr<QTimer> brightness_timer;

    int current_brightness;
    int current_rotate_angle = 0;

    std::string current_ip_;
    void bound_pages();

    void connect_callbacks();

    void create_sub_threads();

    Rect roi_rect;

    std::thread update_thread;
    std::thread inference_thread;
    std::thread osc_send_thread;
    bool app_is_running = true;
    int max_fps = 38;

    std::shared_ptr<SerialPortManager> serial_port_manager;
    std::shared_ptr<ESP32VideoStream> image_downloader;
    std::shared_ptr<Updater> updater;
    std::shared_ptr<Inference> inference;
    std::shared_ptr<OscManager> osc_manager;
    std::shared_ptr<ConfigWriter> config_writer;

    PaperFaceTrackerConfig config;

    std::string firmware_version;
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif //PAPER_FACE_TRACKER_WINDOW_HPP
