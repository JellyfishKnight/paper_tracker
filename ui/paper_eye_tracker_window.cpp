//
// Created by JellyfishKnight on 25-3-11.
//
#include "paper_eye_tracker_window.hpp"

#include <QMessageBox>

#include "ui_paper_eye_tracker_window.h"


PaperEyeTrackerWindow::PaperEyeTrackerWindow(PaperEyeTrackerConfig* config, QWidget *parent) :
    QWidget(parent) {
    if (instance == nullptr)
        instance = this;
    else
        throw std::exception("当前已经打开了眼追窗口，请不要重复打开");
    ui.setupUi(this);
    setFixedSize(883, 596);
    append_log_window(ui.LogText);

    connect_callbacks();
    // 添加输入框焦点事件处理
    ui.SSIDInput->installEventFilter(this);
    ui.PassWordInput->installEventFilter(this);
    // 允许Tab键在输入框之间跳转
    ui.SSIDInput->setTabChangesFocus(true);
    ui.PassWordInput->setTabChangesFocus(true);
    setFocus();

    osc_manager = std::make_shared<OscManager>();
    set_config();
    // LOG_INFO("正在初始化OSC...");
    // if (osc_manager->init("127.0.0.1", 8888)) {
    //     osc_manager->setLocationPrefix("");
    //     osc_manager->setMultiplier(1.0f);
    //     LOG_INFO("OSC初始化成功");
    // } else {
    //     LOG_ERROR("OSC初始化失败，请检查网络连接");
    // }
    // 初始化串口和wifi
    serial_port_ = std::make_shared<SerialPortManager>();
    left_image_stream = std::make_shared<ESP32VideoStream>();
    right_image_stream = std::make_shared<ESP32VideoStream>();

    serial_port_->init();
    // init serial port manager
    serial_port_->registerCallback(
        PACKET_DEVICE_STATUS,
        [this](const std::string& ip, int brightness, int power, int version) {
            if (version != 2 && version != 3)
            {
                static bool version_warning = false;
                if (!version_warning)
                {
                    QMessageBox msgBox;
                    msgBox.setWindowIcon(this->windowIcon());
                    msgBox.setText(tr("检测到面捕设备，请打开面捕界面进行设置"));
                    msgBox.exec();
                    version_warning = true;
                }
                return ;
            }
            // 使用Qt的线程安全方式更新UI
            QMetaObject::invokeMethod(this, [ip, brightness, power, version, this]() {
                if (version == 2)
                {
                    if (current_left_ip != "http://" + ip)
                    {
                        current_left_ip = "http://" + ip;
                        // 更新IP地址显示，添加 http:// 前缀
                        this->setLeftIPText(QString::fromStdString(current_left_ip));
                        LOG_INFO("IP地址已更新: {}", current_left_ip);
                        start_image_download(version);
                    }
                } else if (version == 3)
                {
                    if (current_right_ip != "http://" + ip)
                    {
                        current_right_ip = "http://" + ip;
                        // 更新IP地址显示，添加 http:// 前缀
                        this->setLeftIPText(QString::fromStdString(current_right_ip));
                        LOG_INFO("IP地址已更新: {}", current_right_ip);
                        start_image_download(version);
                    }
                }
                // 可以添加其他状态更新的日志，如果需要的话
            }, Qt::QueuedConnection);
        }
    );
}

PaperEyeTrackerWindow::~PaperEyeTrackerWindow() {
    LOG_INFO("正在关闭系统...");
    instance = nullptr;
    app_is_running = false;
    if (update_left_ui_thread.joinable())
    {
        update_left_ui_thread.join();
    }
    if (update_right_ui_thread.joinable())
    {
        update_right_ui_thread.join();
    }
    if (serial_port_->status() == SerialStatus::OPENED)
    {
        serial_port_->stop();
    }
    if (left_image_stream->isStreaming())
    {
        left_image_stream->stop();
    }
    if (right_image_stream->isStreaming())
    {
        right_image_stream->stop();
    }
    osc_manager->close();
    LOG_INFO("系统已安全关闭");
    remove_log_window(ui.LogText);
    instance = nullptr;
}

void PaperEyeTrackerWindow::onSendButtonClicked()
{
    auto ssid = getSSID();
    auto password = getPassword();

    // 输入验证
    if (ssid == "请输入WIFI名字（仅支持2.4ghz）" || ssid.empty()) {
        QMessageBox::warning(this, tr("输入错误"), tr("请输入有效的WIFI名字"));
        return;
    }

    if (password == "请输入WIFI密码" || password.empty()) {
        QMessageBox::warning(this, tr("输入错误"), tr("请输入有效的密码"));
        return;
    }

    // 构建并发送数据包
    LOG_INFO("已发送WiFi配置: SSID = {}, PWD = {}", ssid, password);
    LOG_INFO("等待数据被发送后开始自动重启ESP32...");
    serial_port_->sendWiFiConfig(ssid, password);

    QTimer::singleShot(3000, this, [this] {
        // 3秒后自动重启ESP32
        onRestartButtonClicked();
    });
}

void PaperEyeTrackerWindow::onRestartButtonClicked()
{
    if (current_esp32_version == 1)
    {
        QMessageBox::information(this, "错误", "插入的是面捕设备，请到面捕界面操作");
        return;
    }
    serial_port_->stop_heartbeat_timer();
    if (current_esp32_version == 2)
    {
        left_image_stream->stop_heartbeat_timer();
    } else if (current_esp32_version == 3)
    {
        right_image_stream->stop_heartbeat_timer();
    }
    serial_port_->restartESP32(this);
    serial_port_->start_heartbeat_timer();
    if (current_esp32_version == 2)
    {
        left_image_stream->stop();
        left_image_stream->start();
        left_image_stream->start_heartbeat_timer();
    } else if (current_esp32_version == 3)
    {
        right_image_stream->stop();
        right_image_stream->start();
        right_image_stream->start_heartbeat_timer();
    }
}

void PaperEyeTrackerWindow::onFlashButtonClicked()
{
    if (current_esp32_version == 1)
    {
        QMessageBox::information(this, "错误", "插入的是面捕设备，请到面捕界面操作");
        return;
    }
    serial_port_->stop_heartbeat_timer();
    if (current_esp32_version == 2)
    {
        left_image_stream->stop_heartbeat_timer();
    } else if (current_esp32_version == 3)
    {
        right_image_stream->stop_heartbeat_timer();
    }
    serial_port_->flashESP32(this);
    serial_port_->start_heartbeat_timer();
    if (current_esp32_version == 2)
    {
        left_image_stream->stop();
        left_image_stream->start();
        left_image_stream->start_heartbeat_timer();
    } else if (current_esp32_version == 3)
    {
        right_image_stream->stop();
        right_image_stream->start();
        right_image_stream->start_heartbeat_timer();
    }
}

void PaperEyeTrackerWindow::connect_callbacks()
{
    connect(ui.SendButton, &QPushButton::clicked, this, &PaperEyeTrackerWindow::onSendButtonClicked);
    connect(ui.RestartButton, &QPushButton::clicked, this, &PaperEyeTrackerWindow::onRestartButtonClicked);
    connect(ui.FlashButton, &QPushButton::clicked, this, &PaperEyeTrackerWindow::onFlashButtonClicked);
}


std::string PaperEyeTrackerWindow::getSSID() const
{
    return ui.SSIDInput->toPlainText().toStdString();
}

std::string PaperEyeTrackerWindow::getPassword() const
{
    return ui.PassWordInput->toPlainText().toStdString();
}

int PaperEyeTrackerWindow::get_max_fps() const
{
    return max_fps;
}

bool PaperEyeTrackerWindow::is_running() const
{
    return app_is_running;
}

void PaperEyeTrackerWindow::set_config()
{

}

void PaperEyeTrackerWindow::setLeftIPText(const QString& text) const
{
    ui.LeftEyeIPAddress->setPlainText(tr(text.toUtf8().constData()));
}

void PaperEyeTrackerWindow::setRightIPText(const QString& text) const
{
    ui.RightEyeIPAddress->setPlainText(tr(text.toUtf8().constData()));
}

void PaperEyeTrackerWindow::start_image_download(int version) const
{
    if (version == 2)
    {
        if (left_image_stream->isStreaming())
        {
            left_image_stream->stop();
        }
        // 开始下载图片 - 修改为支持WebSocket协议
        // 检查URL格式
        const std::string& url = current_left_ip;
        if (url.substr(0, 7) == "http://" || url.substr(0, 8) == "https://" ||
            url.substr(0, 5) == "ws://" || url.substr(0, 6) == "wss://") {
            // URL已经包含协议前缀，直接使用
            left_image_stream->init(url);
            } else {
                // 添加默认ws://前缀
                left_image_stream->init("ws://" + url);
            }
        left_image_stream->start();
    } else if (version == 3)
    {
        if (right_image_stream->isStreaming())
        {
            right_image_stream->stop();
        }
        // 开始下载图片 - 修改为支持WebSocket协议
        // 检查URL格式
        const std::string& url = current_right_ip;
        if (url.substr(0, 7) == "http://" || url.substr(0, 8) == "https://" ||
            url.substr(0, 5) == "ws://" || url.substr(0, 6) == "wss://") {
            // URL已经包含协议前缀，直接使用
            right_image_stream->init(url);
            } else {
                // 添加默认ws://前缀
                right_image_stream->init("ws://" + url);
            }
        right_image_stream->start();
    }
}
