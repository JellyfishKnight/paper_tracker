//
// Created by JellyfishKnight on 25-3-11.
//
#include "eye_tracker_window.hpp"

#include <QMessageBox>

#include "ui_eye_tracker_window.h"


PaperEyeTrackerWindow::PaperEyeTrackerWindow(QWidget *parent) :
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
    config_writer = std::make_shared<ConfigWriter>("./eye_config.json");
    config = config_writer->get_config<PaperEyeTrackerConfig>();
    set_config();

    left_http_server_ = std::make_shared<HttpServer>();
    right_http_server_ = std::make_shared<HttpServer>();
    left_http_server_->start(9002);
    right_http_server_->start(9003);
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
            current_esp32_version = version;
            if (version != LEFT_TAG && version != RIGHT_TAG)
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
                if (version == LEFT_TAG)
                {
                    if (current_left_ip != "http://" + ip)
                    {
                        current_left_ip = "http://" + ip;
                        // 更新IP地址显示，添加 http:// 前缀
                        this->setIPText(LEFT_TAG, QString::fromStdString(current_left_ip));
                        LOG_INFO("IP地址已更新: {}", current_left_ip);
                        start_image_download(version);
                    }
                } else if (version == RIGHT_TAG)
                {
                    if (current_right_ip != "http://" + ip)
                    {
                        current_right_ip = "http://" + ip;
                        // 更新IP地址显示，添加 http:// 前缀
                        this->setIPText(RIGHT_TAG, QString::fromStdString(current_right_ip));
                        LOG_INFO("IP地址已更新: {}", current_right_ip);
                        start_image_download(version);
                    }
                }
                // 可以添加其他状态更新的日志，如果需要的话
            }, Qt::QueuedConnection);
        }
    );


    while (serial_port_->status() == SerialStatus::CLOSED) {}

    if (serial_port_->status() == SerialStatus::FAILED)
    {
        LOG_WARN("没有检测到眼追设备，尝试从配置文件中读取地址...");
        if (!config.left_ip.empty())
        {
            LOG_INFO("从配置文件中读取左眼地址成功");
            current_left_ip = config.left_ip;
            start_image_download(LEFT_TAG);
        } else
        {
            QMessageBox msgBox;
            msgBox.setWindowIcon(this->windowIcon());
            msgBox.setText(tr("未找到左眼配置文件信息，请将设备通过数据线连接到电脑进行首次配置"));
            msgBox.exec();
        }
        if (!config.right_ip.empty())
        {
            LOG_INFO("从配置文件中读取右眼地址成功");
            current_right_ip = config.right_ip;
            start_image_download(RIGHT_TAG);
        } else
        {
            QMessageBox msgBox;
            msgBox.setWindowIcon(this->windowIcon());
            msgBox.setText(tr("未找到右眼配置文件信息，请将设备通过数据线连接到电脑进行首次配置"));
            msgBox.exec();
        }
    } else
    {
        LOG_INFO("有线模式面捕连接成功");
        setSerialStatusLabel("有线模式面捕连接成功");
    }

    create_sub_thread();
}

void PaperEyeTrackerWindow::setVideoImage(int version, const cv::Mat& image)
{
    auto image_label = version == LEFT_TAG ? ui.LeftEyeImage : ui.RightEyeImage;
    if (image.empty())
    {
        QMetaObject::invokeMethod(this, [this, image_label]()
        {
            image_label->clear(); // 清除图片
            image_label->setText(tr("                          没有图像输入"));
        }, Qt::QueuedConnection);
        return ;
    }
    QMetaObject::invokeMethod(this, [this, image = image.clone(), image_label]() {
        auto qimage = QImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);
        auto pix_map = QPixmap::fromImage(qimage);
        image_label->setPixmap(pix_map);
        image_label->setScaledContents(true);
        image_label->update();
    }, Qt::QueuedConnection);
}

void PaperEyeTrackerWindow::create_sub_thread()
{
    update_left_ui_thread = std::thread([this] ()
    {
        auto last_time = std::chrono::high_resolution_clock::now();
        double fps_total = 0;
        double fps_count = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        while (is_running())
        {
            updateWifiLabel(LEFT_TAG);
            updateSerialLabel(current_esp32_version);
            auto start_time = std::chrono::high_resolution_clock::now();
            try {
                if (fps_total > 1000)
                {
                    fps_count = 0;
                    fps_total = 0;
                }
                // caculate fps
                auto start = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(start - last_time);
                last_time = start;
                auto fps = 1000.0 / static_cast<double>(duration.count());
                fps_total += fps;
                fps_count += 1;
                fps = fps_total/fps_count;
                cv::Mat frame = getVideoImage(LEFT_TAG);
                // draw rect on frame
                cv::Mat show_image;
                if (!frame.empty())
                {
                    left_http_server_->updateFrame(frame);
                    show_image = frame;
                }
                setVideoImage(LEFT_TAG, show_image);
                // 控制帧率
            } catch (const std::exception& e) {
                // 使用Qt方式记录日志，而不是minilog
                QMetaObject::invokeMethod(this, [&e]() {
                    LOG_ERROR("错误, 视频处理异常: {}", e.what());
                }, Qt::QueuedConnection);
            }
            auto end_time = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count ();
            int delay_ms = std::max(0, static_cast<int>(1000.0 / std::min(get_max_fps() + 30, 50) - elapsed));
            // LOG_DEBUG("UIFPS:" +  std::to_string(min(get_max_fps() + 30, 60)));
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }
    });

    update_right_ui_thread = std::thread([this] ()
    {
        auto last_time = std::chrono::high_resolution_clock::now();
        double fps_total = 0;
        double fps_count = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        while (is_running())
        {
            updateWifiLabel(RIGHT_TAG);
            updateSerialLabel(current_esp32_version);
            auto start_time = std::chrono::high_resolution_clock::now();
            try {
                if (fps_total > 1000)
                {
                    fps_count = 0;
                    fps_total = 0;
                }
                // calculate fps
                auto start = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(start - last_time);
                last_time = start;
                auto fps = 1000.0 / static_cast<double>(duration.count());
                fps_total += fps;
                fps_count += 1;
                fps = fps_total/fps_count;
                cv::Mat frame = getVideoImage(RIGHT_TAG);
                // draw rect on frame
                cv::Mat show_image;
                if (!frame.empty())
                {
                    right_http_server_->updateFrame(frame);
                    show_image = frame;
                }
                setVideoImage(RIGHT_TAG, show_image);
                // 控制帧率
            } catch (const std::exception& e) {
                // 使用Qt方式记录日志，而不是minilog
                QMetaObject::invokeMethod(this, [&e]() {
                    LOG_ERROR("错误, 视频处理异常: {}", e.what());
                }, Qt::QueuedConnection);
            }
            auto end_time = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count ();
            int delay_ms = std::max(0, static_cast<int>(1000.0 / std::min(get_max_fps() + 30, 50) - elapsed));
            // LOG_DEBUG("UIFPS:" +  std::to_string(min(get_max_fps() + 30, 60)));
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }
    });


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
    if (left_http_server_)
    {
        left_http_server_->stop();
    }
    if (right_http_server_)
    {
        right_http_server_->stop();
    }
    osc_manager->close();
    config = generate_config();
    config_writer->write_config(config);
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
    if (current_esp32_version == FACE_TAG)
    {
        QMessageBox::information(this, "错误", "插入的是面捕设备，请到面捕界面操作");
        return;
    }
    serial_port_->stop_heartbeat_timer();
    if (current_esp32_version == LEFT_TAG)
    {
        left_image_stream->stop_heartbeat_timer();
    } else if (current_esp32_version == RIGHT_TAG)
    {
        right_image_stream->stop_heartbeat_timer();
    }
    serial_port_->restartESP32(this);
    serial_port_->start_heartbeat_timer();
    if (current_esp32_version == LEFT_TAG)
    {
        left_image_stream->stop();
        left_image_stream->start();
        left_image_stream->start_heartbeat_timer();
    } else if (current_esp32_version == RIGHT_TAG)
    {
        right_image_stream->stop();
        right_image_stream->start();
        right_image_stream->start_heartbeat_timer();
    }
}

void PaperEyeTrackerWindow::onFlashButtonClicked()
{
    if (current_esp32_version == FACE_TAG)
    {
        QMessageBox::information(this, "错误", "插入的是面捕设备，请到面捕界面操作");
        return;
    }
    serial_port_->stop_heartbeat_timer();
    if (current_esp32_version == LEFT_TAG)
    {
        left_image_stream->stop_heartbeat_timer();
    } else if (current_esp32_version == RIGHT_TAG)
    {
        right_image_stream->stop_heartbeat_timer();
    }
    serial_port_->flashESP32(this);
    serial_port_->start_heartbeat_timer();
    if (current_esp32_version == LEFT_TAG)
    {
        left_image_stream->stop();
        left_image_stream->start();
        left_image_stream->start_heartbeat_timer();
    } else if (current_esp32_version == RIGHT_TAG)
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

void PaperEyeTrackerWindow::set_config() const
{
    ui.RightEyeIPAddress->setPlainText(QString::fromStdString(config.right_ip));
    ui.LeftEyeIPAddress->setPlainText(QString::fromStdString(config.left_ip));
}

void PaperEyeTrackerWindow::setIPText(int version, const QString& text) const
{
    if (version == LEFT_TAG)
    {
        ui.LeftEyeIPAddress->setPlainText(tr(text.toUtf8().constData()));
    } else
    {
        ui.RightEyeIPAddress->setPlainText(tr(text.toUtf8().constData()));
    }
}


void PaperEyeTrackerWindow::start_image_download(int version) const
{
    if (version == LEFT_TAG)
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
    } else if (version == RIGHT_TAG)
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

void PaperEyeTrackerWindow::updateWifiLabel(int version) const
{
    if (version == LEFT_TAG)
    {
        if (left_image_stream->isStreaming())
        {
            setWifiStatusLabel(version, "左眼Wifi已连接");
        } else
        {
            setWifiStatusLabel(version, "左眼Wifi连接失败");
        }
    } else if (version == RIGHT_TAG)
    {
        if (right_image_stream->isStreaming())
        {
            setWifiStatusLabel(version, "右眼Wifi已连接");
        } else
        {
            setWifiStatusLabel(version, "右眼Wifi连接失败");
        }
    }
}

void PaperEyeTrackerWindow::updateSerialLabel(int version) const
{
    if (serial_port_->status() == SerialStatus::OPENED)
    {
        if (version == LEFT_TAG)
        {
            setSerialStatusLabel("左眼设备已连接");
        } else if (version == RIGHT_TAG)
        {
            setSerialStatusLabel("右眼设备已连接");
        }
    } else
    {
        setSerialStatusLabel("没有设备连接");
    }
}

cv::Mat PaperEyeTrackerWindow::getVideoImage(int version) const
{
    if (version == LEFT_TAG)
    {
        return std::move(left_image_stream->getLatestFrame());
    } else if (version == RIGHT_TAG)
    {
        return std::move(right_image_stream->getLatestFrame());
    } else
    {
        return {};
    }
}

void PaperEyeTrackerWindow::setSerialStatusLabel(const QString& text) const
{
    ui.EyeWindowSerialStatus->setText(text);
}

void PaperEyeTrackerWindow::setWifiStatusLabel(int version, const QString& text) const
{
    if (version == LEFT_TAG)
    {
        ui.LeftEyeWifiStatus->setText(text.toUtf8().constData());
    } else if (version == RIGHT_TAG)
    {
        ui.RightEyeWifiStatus->setText(text.toUtf8().constData());
    }
}

PaperEyeTrackerConfig PaperEyeTrackerWindow::generate_config() const
{
    PaperEyeTrackerConfig res_config;
    res_config.left_ip = ui.LeftEyeIPAddress->toPlainText().toStdString();
    res_config.right_ip = ui.RightEyeIPAddress->toPlainText().toStdString();
    return res_config;
}