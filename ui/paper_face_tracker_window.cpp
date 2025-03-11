//
// Created by JellyfishKnight on 25-3-11.
//
/*
 * PaperTracker - 面部追踪应用程序
 * Copyright (C) 2025 PAPER TRACKER
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file contains code from projectbabble:
 * Copyright 2023 Sameer Suri
 * Licensed under the Apache License, Version 2.0
 */

#include <opencv2/imgproc.hpp>
#include "paper_face_tracker_window.hpp"
#include "ui_paper_face_tracker_window.h"
#include <QMessageBox>
#include <codecvt>
#include <locale>
#include <windows.h>
#include <QTimer>
#include <QProcess>
#include <QCoreApplication>
#include <roi_event.hpp>

PaperFaceTrackerWindow::PaperFaceTrackerWindow(QWidget *parent)
    : QWidget(parent), config(config)
{
    // 基本UI设置
    setFixedSize(848, 538);
    ui.setupUi(this);
    ui.LogText->setMaximumBlockCount(200);
    init_logger(ui.LogText);
    LOG_INFO("系统初始化中...");
    // 初始化串口连接状态
    ui.SerialConnectLabel->setText(tr("有线模式未连接"));
    ui.WifiConnectLabel->setText(tr("无线模式未连接"));
    // 初始化页面导航
    bound_pages();

    // 初始化亮度控制相关成员
    current_brightness = 0;
    // 连接信号和槽
    connect_callbacks();
    // 添加输入框焦点事件处理
    ui.SSIDText->installEventFilter(this);
    ui.PasswordText->installEventFilter(this);
    // 允许Tab键在输入框之间跳转
    ui.SSIDText->setTabChangesFocus(true);
    ui.PasswordText->setTabChangesFocus(true);
    // 清除所有控件的初始焦点，确保没有文本框自动获得焦点
    setFocus();

    // 添加ROI事件
    auto *roiFilter = new ROIEventFilter([this] (QRect rect, bool isEnd)
    {
        int x = rect.x ();
        int y = rect.y ();
        int width = rect.width ();
        int height = rect.height ();

        // 规范化宽度和高度为正值
        if (width < 0) {
            x += width;
            width = -width;
        }
        if (height < 0) {
            y += height;
            height = -height;
        }

        // 裁剪坐标到图像边界内
        if (x < 0) {
            width += x;  // 减少宽度
            x = 0;       // 将 x 设为 0
        }
        if (y < 0) {
            height += y; // 减少高度
            y = 0;       // 将 y 设为 0
        }

        // 确保 ROI 不超出图像边界
        if (x + width > 280) {
            width = 280 - x;
        }
        if (y + height > 280) {
            height = 280 - y;
        }
        // 确保最终的宽度和高度为正值
        width = max(0, width);
        height = max(0, height);

        // 更新 roi_rect
        roi_rect.is_roi_end = isEnd;
        roi_rect = Rect (x, y, width, height);
    },ui.ImageLabel);
    ui.ImageLabel->installEventFilter(roiFilter);
    ui.ImageLabelCal->installEventFilter(roiFilter);
    LOG_INFO("系统初始化完成, 正在启动VRCFT");

    vrcftProcess = new QProcess(this);
    // 启动VRCFT应用程序
    // 尝试方法2: PowerShell查找并启动
    QString command2 = "powershell -Command \"$pkg = Get-AppxPackage | Where-Object {$_.Name -like '*96ba052f*'}; if($pkg) { Start-Process ($pkg.InstallLocation + '\\VRCFaceTracking.exe') }\"";
    vrcftProcess->start(command2);
    // 连接信号以检测进程状态
    connect(vrcftProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit) {
            LOG_DEBUG("VRCFT已正常退出，退出码: {}", exitCode);
        } else {
            LOG_ERROR("VRCFT异常退出");
        }
    });
    inference = std::make_shared<Inference>();
    osc_manager = std::make_shared<OscManager>();
    config_writer = std::make_shared<ConfigWriter>("./face_tracker_config.json");
    set_config(config_writer->get_config<PaperFaceTrackerConfig>());
    // Load model
    LOG_INFO("正在加载推理模型...");
    try {
        inference->load_model("./model/model.onnx");
        LOG_INFO("模型加载完成");
    } catch (const std::exception& e) {
        // 使用Qt方式记录日志，而不是minilog
        LOG_ERROR("错误: 模型加载异常: {}", e.what());
    }
    // 初始化OSC管理器
    LOG_INFO("正在初始化OSC...");
    if (osc_manager->init("127.0.0.1", 8888)) {
        osc_manager->setLocationPrefix("");
        osc_manager->setMultiplier(1.0f);
        LOG_INFO("OSC初始化成功");
    } else {
        LOG_ERROR("OSC初始化失败，请检查网络连接");
    }
    // 初始化串口和wifi
    serial_port_manager = std::make_shared<SerialPortManager>();
    image_downloader = std::make_shared<ESP32VideoStream>();
    updater = std::make_shared<Updater>();
    LOG_INFO("初始化有线模式");
    serial_port_manager->init();
    // init serial port manager
    serial_port_manager->setDeviceStatusCallback([this]
                                                        (const std::string& ip, int brightness, int power, int version) {
        // 使用Qt的线程安全方式更新UI
        QMetaObject::invokeMethod(this, [ip, brightness, power, version, this]() {
            // 只在 IP 地址变化时更新显示
            if (current_ip_ != "http://" + ip)
            {
                current_ip_ = "http://" + ip;
                // 更新IP地址显示，添加 http:// 前缀
                this->setIPText(QString::fromStdString(current_ip_));
                LOG_INFO("IP地址已更新: {}", current_ip_);
                start_image_download();
            }
            firmware_version = std::to_string(version);
            // 可以添加其他状态更新的日志，如果需要的话
        }, Qt::QueuedConnection);
    });

    LOG_DEBUG("等待有线模式面捕连接");
    while (serial_port_manager->status() == SerialStatus::CLOSED) {}
    LOG_DEBUG("有线模式面捕连接完毕");

    if (serial_port_manager->status() == SerialStatus::FAILED)
    {
        setSerialStatusLabel("有线模式面捕连接失败");
        LOG_WARN("有线模式面捕未连接，尝试从配置文件中读取地址...");
        if (!config.wifi_ip.empty())
        {
            LOG_INFO("从配置文件中读取地址成功");
            current_ip_ = config.wifi_ip;
            set_config(config);
            start_image_download();
        } else
        {
            QMessageBox msgBox;
            msgBox.setWindowIcon(this->windowIcon());
            msgBox.setText(tr("未找到配置文件信息，请将面捕通过数据线连接到电脑进行首次配置"));
            msgBox.exec();
        }
    } else
    {
        LOG_INFO("有线模式面捕连接成功");
        setSerialStatusLabel("有线模式面捕连接成功");
    }
    create_sub_threads();
}

void PaperFaceTrackerWindow::setVideoImage(const cv::Mat& image)
{
    if (image.empty())
    {
        QMetaObject::invokeMethod(this, [this]()
        {
            if (ui.stackedWidget->currentIndex() == 0) {
                ui.ImageLabel->clear(); // 清除图片
                ui.ImageLabel->setText(tr("                         没有图像输入")); // 恢复默认文本
            } else if (ui.stackedWidget->currentIndex() == 1) {
                ui.ImageLabelCal->clear(); // 清除图片
                ui.ImageLabelCal->setText(tr("                         没有图像输入"));
            }
        }, Qt::QueuedConnection);
        return ;
    }
    QMetaObject::invokeMethod(this, [this, image = image.clone()]() {
        auto qimage = QImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);
        auto pix_map = QPixmap::fromImage(qimage);
        if (ui.stackedWidget->currentIndex() == 0)
        {
            ui.ImageLabel->setPixmap(pix_map);
            ui.ImageLabel->setScaledContents(true);
            ui.ImageLabel->update();
        } else if (ui.stackedWidget->currentIndex() == 1) {
            ui.ImageLabelCal->setPixmap(pix_map);
            ui.ImageLabelCal->setScaledContents(true);
            ui.ImageLabelCal->update();
        }
    }, Qt::QueuedConnection);
}

PaperFaceTrackerWindow::~PaperFaceTrackerWindow() {
    stop();
    LOG_INFO("开始自动保存");
    if (config_writer->write_config(generate_config()))
    {
        LOG_INFO("已保存配置");
    } else
    {
        LOG_ERROR("配置文件保存失败");
    }
}

void PaperFaceTrackerWindow::bound_pages() {
    // 页面导航逻辑
    connect(ui.MainPageButton, &QPushButton::clicked, [this] {
        ui.stackedWidget->setCurrentIndex(0);
    });
    connect(ui.CalibrationPageButton, &QPushButton::clicked, [this] {
        ui.stackedWidget->setCurrentIndex(1);
    });
}

// 添加事件过滤器实现
bool PaperFaceTrackerWindow::eventFilter(QObject *obj, QEvent *event)
{
    // 处理焦点获取事件
    if (event->type() == QEvent::FocusIn) {
        if (obj == ui.SSIDText) {
            if (ui.SSIDText->toPlainText() == "请输入WIFI名字（仅支持2.4ghz）") {
                ui.SSIDText->setPlainText("");
            }
        } else if (obj == ui.PasswordText) {
            if (ui.PasswordText->toPlainText() == "请输入WIFI密码") {
                ui.PasswordText->setPlainText("");
            }
        }
    }

    // 处理焦点失去事件
    if (event->type() == QEvent::FocusOut) {
        if (obj == ui.SSIDText) {
            if (ui.SSIDText->toPlainText().isEmpty()) {
                ui.SSIDText->setPlainText("请输入WIFI名字（仅支持2.4ghz）");
            }
        } else if (obj == ui.PasswordText) {
            if (ui.PasswordText->toPlainText().isEmpty()) {
                ui.PasswordText->setPlainText("请输入WIFI密码");
            }
        }
    }

    // 继续事件处理
    return QWidget::eventFilter(obj, event);
}

// 根据模型输出更新校准页面的进度条
void PaperFaceTrackerWindow::updateCalibrationProgressBars(
    const std::vector<float>& output,
    const std::unordered_map<std::string, size_t>& blendShapeIndexMap
) {
    if (output.empty() || ui.stackedWidget->currentIndex() != 1) {
        // 如果输出为空或者当前不在校准页面，则不更新
        return;
    }

    // 使用Qt的线程安全方式更新UI
    QMetaObject::invokeMethod(this, [this, output, &blendShapeIndexMap]() {
        // 将值缩放到0-100范围内用于进度条显示
        auto scaleValue = [](const float value) -> int {
            // 将值限制在0-1.0范围内，然后映射到0-100
            return static_cast<int>(value * 100);
        };

        // 更新各个进度条
        // 注意：这里假设输出数组中的索引与ARKit模型输出的顺序一致

        // 脸颊
        if (blendShapeIndexMap.contains("cheekPuffLeft") && blendShapeIndexMap.at("cheekPuffLeft") < output.size()) {
            ui.CheekPullLeftValue->setValue(scaleValue(output[blendShapeIndexMap.at("cheekPuffLeft")]));
        }
        if (blendShapeIndexMap.contains("cheekPuffRight") && blendShapeIndexMap.at("cheekPuffRight") < output.size()) {
            ui.CheekPullRightValue->setValue(scaleValue(output[blendShapeIndexMap.at("cheekPuffRight")]));
        }
        // 下巴
        if (blendShapeIndexMap.contains("jawOpen") && blendShapeIndexMap.at("jawOpen") < output.size()) {
            ui.JawOpenValue->setValue(scaleValue(output[blendShapeIndexMap.at("jawOpen")]));
        }
        if (blendShapeIndexMap.contains("jawLeft") && blendShapeIndexMap.at("jawLeft") < output.size()) {
            ui.JawLeftValue->setValue(scaleValue(output[blendShapeIndexMap.at("jawLeft")]));
        }
        if (blendShapeIndexMap.contains("jawRight") && blendShapeIndexMap.at("jawRight") < output.size()) {
            ui.JawRightValue->setValue(scaleValue(output[blendShapeIndexMap.at("jawRight")]));
        }
        // 嘴巴
        if (blendShapeIndexMap.contains("mouthLeft") && blendShapeIndexMap.at("mouthLeft") < output.size()) {
            ui.MouthLeftValue->setValue(scaleValue(output[blendShapeIndexMap.at("mouthLeft")]));
        }
        if (blendShapeIndexMap.contains("mouthRight") && blendShapeIndexMap.at("mouthRight") < output.size()) {
            ui.MouthRightValue->setValue(scaleValue(output[blendShapeIndexMap.at("mouthRight")]));
        }
        // 舌头
        if (blendShapeIndexMap.contains("tongueOut") && blendShapeIndexMap.at("tongueOut") < output.size()) {
            ui.TongueOutValue->setValue(scaleValue(output[blendShapeIndexMap.at("tongueOut")]));
        }
        if (blendShapeIndexMap.contains("tongueUp") && blendShapeIndexMap.at("tongueUp") < output.size()) {
            ui.TongueUpValue->setValue(scaleValue(output[blendShapeIndexMap.at("tongueUp")]));
        }
        if (blendShapeIndexMap.contains("tongueDown") && blendShapeIndexMap.at("tongueDown") < output.size()) {
            ui.TongueDownValue->setValue(scaleValue(output[blendShapeIndexMap.at("tongueDown")]));
        }
        if (blendShapeIndexMap.contains("tongueLeft") && blendShapeIndexMap.at("tongueLeft") < output.size()) {
            ui.TongueLeftValue->setValue(scaleValue(output[blendShapeIndexMap.at("tongueLeft")]));
        }
        if (blendShapeIndexMap.contains("tongueRight") && blendShapeIndexMap.at("tongueRight") < output.size()) {
            ui.TongueRightValue->setValue(scaleValue(output[blendShapeIndexMap.at("tongueRight")]));
        }
    }, Qt::QueuedConnection);
}

void PaperFaceTrackerWindow::connect_callbacks()
{
    brightness_timer = std::make_shared<QTimer>();
    brightness_timer->setSingleShot(true);
    connect(brightness_timer.get(), &QTimer::timeout, this, &PaperFaceTrackerWindow::onSendBrightnessValue);
    // functions
    connect(ui.BrightnessBar, &QScrollBar::valueChanged, this, &PaperFaceTrackerWindow::onBrightnessChanged);
    connect(ui.RotateImageBar, &QScrollBar::valueChanged, this, &PaperFaceTrackerWindow::onRotateAngleChanged);
    connect(ui.restart_Button, &QPushButton::clicked, this, &PaperFaceTrackerWindow::onRestartButtonClicked);
    connect(ui.FlashFirmwareButton, &QPushButton::clicked, this, &PaperFaceTrackerWindow::onFlashButtonClicked);
    connect(ui.UseFilterBox, &QCheckBox::checkStateChanged, this, &PaperFaceTrackerWindow::onUseFilterClicked);
    connect(ui.wifi_send_Button, &QPushButton::clicked, this, &PaperFaceTrackerWindow::onSendButtonClicked);
    connect(ui.EnergyModeBox, &QComboBox::currentIndexChanged, this, &PaperFaceTrackerWindow::onEnergyModeChanged);
    connect(ui.SaveParamConfigButton, &QPushButton::clicked, this, &PaperFaceTrackerWindow::onSaveConfigButtonClicked);

    connect(ui.JawOpenBar, &QScrollBar::valueChanged, this, &PaperFaceTrackerWindow::onJawOpenChanged);
    connect(ui.JawLeftBar, &QScrollBar::valueChanged, this, &PaperFaceTrackerWindow::onJawLeftChanged);
    connect(ui.JawRightBar, &QScrollBar::valueChanged, this, &PaperFaceTrackerWindow::onJawRightChanged);
    connect(ui.MouthLeftBar, &QScrollBar::valueChanged, this, &PaperFaceTrackerWindow::onMouthLeftChanged);
    connect(ui.MouthRightBar, &QScrollBar::valueChanged, this, &PaperFaceTrackerWindow::onMouthRightChanged);
    connect(ui.TongueOutBar, &QScrollBar::valueChanged, this, &PaperFaceTrackerWindow::onTongueOutChanged);
    connect(ui.TongueLeftBar, &QScrollBar::valueChanged, this, &PaperFaceTrackerWindow::onTongueLeftChanged);
    connect(ui.TongueRightBar, &QScrollBar::valueChanged, this, &PaperFaceTrackerWindow::onTongueRightChanged);
    connect(ui.TongueUpBar, &QScrollBar::valueChanged, this, &PaperFaceTrackerWindow::onTongueUpChanged);
    connect(ui.TongueDownBar, &QScrollBar::valueChanged, this, &PaperFaceTrackerWindow::onTongueDownChanged);
    connect(ui.CheekPuffLeftBar, &QScrollBar::valueChanged, this, &PaperFaceTrackerWindow::onCheeckPuffLeftChanged);
    connect(ui.CheekPuffRightBar, &QScrollBar::valueChanged, this, &PaperFaceTrackerWindow::onCheeckPuffRightChanged);

    // update
    connect(ui.CheckFirmwareVersionButton, &QPushButton::clicked, this, &PaperFaceTrackerWindow::onCheckFirmwareVersionClicked);
}

float PaperFaceTrackerWindow::getRotateAngle() const
{
    auto rotate_angle = static_cast<float>(current_rotate_angle);
    rotate_angle = rotate_angle / (static_cast<float>(ui.RotateImageBar->maximum()) -
        static_cast<float>(ui.RotateImageBar->minimum())) * 360.0f;
    return rotate_angle;
}


void PaperFaceTrackerWindow::setOnUseFilterClickedFunc(FuncWithVal func)
{
    onUseFilterClickedFunc = std::move(func);
}

void PaperFaceTrackerWindow::setSerialStatusLabel(const QString& text) const
{
    ui.SerialConnectLabel->setText(tr(text.toUtf8().constData()));
}

void PaperFaceTrackerWindow::setWifiStatusLabel(const QString& text) const
{
    ui.WifiConnectLabel->setText(text.toUtf8().constData());
}

void PaperFaceTrackerWindow::setIPText(const QString& text) const
{
    ui.textEdit->setText(tr(text.toUtf8().constData()));
}

QPlainTextEdit* PaperFaceTrackerWindow::getLogText() const
{
    return ui.LogText;
}

Rect PaperFaceTrackerWindow::getRoiRect()
{
    return roi_rect;
}

std::string PaperFaceTrackerWindow::getSSID() const
{
    return ui.SSIDText->toPlainText().toStdString();
}

std::string PaperFaceTrackerWindow::getPassword() const
{
    return ui.PasswordText->toPlainText().toStdString();
}

void PaperFaceTrackerWindow::onSendButtonClicked()
{
    // onSendButtonClickedFunc();
    // 获取SSID和密码
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
    serial_port_manager->sendWiFiConfig(ssid, password);

    QTimer::singleShot(3000, this, [this] {
        // 3秒后自动重启ESP32
        onRestartButtonClicked();
    });
}

void PaperFaceTrackerWindow::onRestartButtonClicked()
{
    serial_port_manager->stop_heartbeat_timer();
    image_downloader->stop_heartbeat_timer();
    serial_port_manager->restartESP32(this);
    serial_port_manager->start_heartbeat_timer();
    image_downloader->stop();
    image_downloader->start();
    image_downloader->start_heartbeat_timer();
}

void PaperFaceTrackerWindow::onUseFilterClicked(int value) const
{
    QTimer::singleShot(10, this, [this, value] {
        inference->set_use_filter(value);
    });
}

void PaperFaceTrackerWindow::onFlashButtonClicked()
{
    serial_port_manager->stop_heartbeat_timer();
    image_downloader->stop_heartbeat_timer();
    serial_port_manager->flashESP32(this);
    serial_port_manager->start_heartbeat_timer();
    image_downloader->stop();
    image_downloader->start();
    image_downloader->start_heartbeat_timer();
}

void PaperFaceTrackerWindow::onBrightnessChanged(int value) {
    // 更新当前亮度值
    current_brightness = value;
    // 重置定时器，如果用户500毫秒内没有再次改变值，就发送数据包
    brightness_timer->start(100);
}

void PaperFaceTrackerWindow::onRotateAngleChanged(int value)
{
    current_rotate_angle = value;
}

void PaperFaceTrackerWindow::onSendBrightnessValue() const
{
    // 发送亮度控制命令 - 确保亮度值为三位数字
    std::string brightness_str = std::to_string(current_brightness);
    // 补齐三位数字，前面加0
    while (brightness_str.length() < 3) {
        brightness_str = std::string("0") + brightness_str;
    }
    std::string packet = "A6" + brightness_str + "B6";
    serial_port_manager->write_data(packet);
    // 记录操作
    LOG_INFO("已设置亮度: {}", current_brightness);
}

bool PaperFaceTrackerWindow::is_running() const
{
    return app_is_running;
}

void PaperFaceTrackerWindow::stop()
{
    LOG_INFO("正在关闭系统...");
    app_is_running = false;
    if (update_thread.joinable())
    {
        update_thread.join();
    }
    if (inference_thread.joinable())
    {
        inference_thread.join();
    }
    if (osc_send_thread.joinable())
    {
        osc_send_thread.join();
    }
    if (brightness_timer) {
        brightness_timer->stop();
        brightness_timer.reset();
    }
    serial_port_manager->stop();
    image_downloader->stop();
    inference.reset();
    osc_manager->close();
    if (vrcftProcess) {
        if (vrcftProcess->state() == QProcess::Running) {
            vrcftProcess->terminate();
            if (!vrcftProcess->waitForFinished(3000)) {  // 等待最多3秒
                vrcftProcess->kill();  // 如果进程没有及时终止，则强制结束
            }
        }
        delete vrcftProcess;
    }
    // 其他清理工作
    LOG_INFO("系统已安全关闭");
}

void PaperFaceTrackerWindow::onEnergyModeChanged(int index)
{
    if (index == 0)
    {
        max_fps = 38;
    } else if (index == 1)
    {
        max_fps = 15;
    } else if (index == 2)
    {
        max_fps = 70;
    }
}

int PaperFaceTrackerWindow::get_max_fps() const
{
    return max_fps;
}

PaperFaceTrackerConfig PaperFaceTrackerWindow::generate_config() const
{
    PaperFaceTrackerConfig config;
    config.brightness = current_brightness;
    config.rotate_angle = current_rotate_angle;
    config.energy_mode = ui.EnergyModeBox->currentIndex();
    config.use_filter = ui.UseFilterBox->isChecked();
    config.wifi_ip = ui.textEdit->toPlainText().toStdString();
    config.amp_map = {
        {"cheekPuffLeft", ui.CheekPuffLeftBar->value()},
        {"cheekPuffRight", ui.CheekPuffRightBar->value()},
        {"jawOpen", ui.JawOpenBar->value()},
        {"jawLeft", ui.JawLeftBar->value()},
        {"jawRight", ui.JawRightBar->value()},
        {"mouthLeft", ui.MouthLeftBar->value()},
        {"mouthRight", ui.MouthRightBar->value()},
        {"tongueOut", ui.TongueOutBar->value()},
        {"tongueUp", ui.TongueUpBar->value()},
        {"tongueDown", ui.TongueDownBar->value()},
        {"tongueLeft", ui.TongueLeftBar->value()},
        {"tongueRight", ui.TongueRightBar->value()},
    };
    config.rect = roi_rect;
    return config;
}

void PaperFaceTrackerWindow::set_config(const PaperFaceTrackerConfig& config)
{
    current_brightness = config.brightness;
    current_rotate_angle = config.rotate_angle;
    ui.BrightnessBar->setValue(config.brightness);
    ui.RotateImageBar->setValue(config.rotate_angle);
    ui.EnergyModeBox->setCurrentIndex(config.energy_mode);
    ui.UseFilterBox->setChecked(config.use_filter);
    ui.textEdit->setPlainText(QString::fromStdString(config.wifi_ip));
    try
    {
        ui.CheekPuffLeftBar->setValue(config.amp_map.at("cheekPuffLeft"));
        ui.CheekPuffRightBar->setValue(config.amp_map.at("cheekPuffRight"));
        ui.JawOpenBar->setValue(config.amp_map.at("jawOpen"));
        ui.JawLeftBar->setValue(config.amp_map.at("jawLeft"));
        ui.JawRightBar->setValue(config.amp_map.at("jawRight"));
        ui.MouthLeftBar->setValue(config.amp_map.at("mouthLeft"));
        ui.MouthRightBar->setValue(config.amp_map.at("mouthRight"));
        ui.TongueOutBar->setValue(config.amp_map.at("tongueOut"));
        ui.TongueUpBar->setValue(config.amp_map.at("tongueUp"));
        ui.TongueDownBar->setValue(config.amp_map.at("tongueDown"));
        ui.TongueLeftBar->setValue(config.amp_map.at("tongueLeft"));
        ui.TongueRightBar->setValue(config.amp_map.at("tongueRight"));
    } catch (std::exception& e)
    {
        LOG_ERROR("配置文件中的振幅映射错误: {}", e.what());
    }
    roi_rect = config.rect;
}


void PaperFaceTrackerWindow::onSaveConfigButtonClicked()
{
    LOG_INFO("保存配置中...");
    config = std::move(generate_config());
    if (config_writer->write_config(config))
    {
        LOG_INFO("已保存配置");
    } else
    {
        LOG_ERROR("配置文件保存失败");
    }
}


void PaperFaceTrackerWindow::onCheeckPuffLeftChanged(int value) const
{
    inference->set_amp_map(getAmpMap());
}

void PaperFaceTrackerWindow::onCheeckPuffRightChanged(int value) const
{
    inference->set_amp_map(getAmpMap());
}

void PaperFaceTrackerWindow::onJawOpenChanged(int value)
{
    inference->set_amp_map(getAmpMap());
}

void PaperFaceTrackerWindow::onJawLeftChanged(int value)
{
    inference->set_amp_map(getAmpMap());
}

void PaperFaceTrackerWindow::onJawRightChanged(int value)
{
    inference->set_amp_map(getAmpMap());
}

void PaperFaceTrackerWindow::onMouthLeftChanged(int value) const
{
    inference->set_amp_map(getAmpMap());
}

void PaperFaceTrackerWindow::onMouthRightChanged(int value)
{
    inference->set_amp_map(getAmpMap());
}

void PaperFaceTrackerWindow::onTongueOutChanged(int value)
{
    inference->set_amp_map(getAmpMap());
}

void PaperFaceTrackerWindow::onTongueLeftChanged(int value)
{
    inference->set_amp_map(getAmpMap());
}

void PaperFaceTrackerWindow::onTongueRightChanged(int value)
{
    inference->set_amp_map(getAmpMap());
}

void PaperFaceTrackerWindow::onTongueUpChanged(int value)
{
    inference->set_amp_map(getAmpMap());
}

void PaperFaceTrackerWindow::onTongueDownChanged(int value) const
{
    inference->set_amp_map(getAmpMap());
}

std::unordered_map<std::string, int> PaperFaceTrackerWindow::getAmpMap() const
{
    return {
        {"cheekPuffLeft", ui.CheekPuffLeftBar->value()},
        {"cheekPuffRight", ui.CheekPuffRightBar->value()},
        {"jawOpen", ui.JawOpenBar->value()},
        {"jawLeft", ui.JawLeftBar->value()},
        {"jawRight", ui.JawRightBar->value()},
        {"mouthLeft", ui.MouthLeftBar->value()},
        {"mouthRight", ui.MouthRightBar->value()},
        {"tongueOut", ui.TongueOutBar->value()},
        {"tongueUp", ui.TongueUpBar->value()},
        {"tongueDown", ui.TongueDownBar->value()},
        {"tongueLeft", ui.TongueLeftBar->value()},
        {"tongueRight", ui.TongueRightBar->value()},
    };
}

void PaperFaceTrackerWindow::start_image_download() const
{
    if (image_downloader->isStreaming())
    {
        image_downloader->stop();
    }
    // 开始下载图片 - 修改为支持WebSocket协议
    // 检查URL格式
    const std::string& url = current_ip_;
    if (url.substr(0, 7) == "http://" || url.substr(0, 8) == "https://" ||
        url.substr(0, 5) == "ws://" || url.substr(0, 6) == "wss://") {
        // URL已经包含协议前缀，直接使用
        image_downloader->init(url);
    } else {
        // 添加默认ws://前缀
        image_downloader->init("ws://" + url);
    }
    image_downloader->start();
}

void PaperFaceTrackerWindow::updateWifiLabel() const
{
    if (image_downloader->isStreaming())
    {
        setWifiStatusLabel("Wifi已连接");
    } else
    {
        setWifiStatusLabel("Wifi连接失败");
    }
}

void PaperFaceTrackerWindow::updateSerialLabel() const
{
    if (serial_port_manager->status() == SerialStatus::OPENED)
    {
        setSerialStatusLabel("面捕有线模式已连接");
    } else
    {
        setSerialStatusLabel("面捕有线模式连接失败");
    }
}

cv::Mat PaperFaceTrackerWindow::getVideoImage() const
{
    return std::move(image_downloader->getLatestFrame());
}

void PaperFaceTrackerWindow::onCheckFirmwareVersionClicked()
{
    if (getSerialStatus() != SerialStatus::OPENED)
    {
        QMessageBox::information(this, tr("固件版本"), tr("有线模式面捕未连接，无法获取固件版本"));
        return ;
    }
    auto version = updater->getCurrentVersion();
    if (version.has_value())
    {
        if (version.value().version.firmware == getFirmwareVersion())
        {
            QMessageBox::information(this, tr("固件版本"), tr("固件版本已是最新"));
        } else
        {
            QMessageBox::information(this, tr("固件版本"), tr("固件版本不是最新，建议烧录最新固件"));
        }
    } else
    {
        QMessageBox::critical(this, tr("错误"), tr("无法获取最新固件版本信息"));
    }
}

std::string PaperFaceTrackerWindow::getFirmwareVersion() const
{
    return firmware_version;
}

SerialStatus PaperFaceTrackerWindow::getSerialStatus() const
{
    return serial_port_manager->status();
}

void PaperFaceTrackerWindow::set_osc_send_thead(FuncWithoutArgs func)
{
    osc_send_thread = std::thread(std::move(func));
}

void PaperFaceTrackerWindow::create_sub_threads()
{
    update_thread = std::thread([this]()
    {
        auto last_time = std::chrono::high_resolution_clock::now();
        double fps_total = 0;
        double fps_count = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        while (is_running())
        {
            updateWifiLabel();
            updateSerialLabel();
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
                cv::Mat frame = getVideoImage();
                if (!frame.empty())
                {
                    auto rotate_angle = getRotateAngle();
                    cv::resize(frame, frame, cv::Size(280, 280), cv::INTER_NEAREST);
                    int y = frame.rows / 2;
                    int x = frame.cols / 2;
                    auto rotate_matrix = cv::getRotationMatrix2D(cv::Point(x, y), rotate_angle, 1);
                    cv::warpAffine(frame, frame, rotate_matrix, frame.size(), cv::INTER_NEAREST);
                    auto roi_rect = getRoiRect();
                    // 显示图像
                    cv::rectangle(frame, roi_rect.rect, cv::Scalar(0, 255, 0), 2);
                }
                // draw rect on frame
                cv::Mat show_image;
                if (!frame.empty())
                {
                    show_image = frame;
                }
                setVideoImage(show_image);
                // 控制帧率
            } catch (const std::exception& e) {
                // 使用Qt方式记录日志，而不是minilog
                QMetaObject::invokeMethod(this, [&e]() {
                    LOG_ERROR("错误, 视频处理异常: {}", e.what());
                }, Qt::QueuedConnection);
            }
            auto end_time = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count ();
            int delay_ms = max(0, static_cast<int>(1000.0 / min(get_max_fps() + 30, 50) - elapsed));
            // LOG_DEBUG("UIFPS:" +  std::to_string(min(get_max_fps() + 30, 60)));
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }
    });

    inference_thread = std::thread([this] ()
    {
        auto last_time = std::chrono::high_resolution_clock::now();
        double fps_total = 0;
        double fps_count = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        while (is_running())
        {
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
            LOG_DEBUG("模型FPS： {}", fps);

            auto start_time = std::chrono::high_resolution_clock::now();
            // 设置时间序列
            inference->set_dt(duration.count() / 1000.0);

            auto frame = getVideoImage();
            // 推理处理
            if (!frame.empty())
            {
                auto rotate_angle = getRotateAngle();
                cv::resize(frame, frame, cv::Size(280, 280), cv::INTER_NEAREST);
                int y = frame.rows / 2;
                int x = frame.cols / 2;
                auto rotate_matrix = cv::getRotationMatrix2D(cv::Point(x, y), rotate_angle, 1);
                cv::warpAffine(frame, frame, rotate_matrix, frame.size(), cv::INTER_NEAREST);
                cv::Mat infer_frame;
                infer_frame = frame.clone();
                auto roi_rect = getRoiRect();
                if (!roi_rect.rect.empty() && roi_rect.is_roi_end)
                {
                    infer_frame = infer_frame(roi_rect.rect);
                }
                inference->inference(infer_frame);
            }
            auto end_time = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            int delay_ms = max(0, static_cast<int>(1000.0 / get_max_fps() - elapsed));
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }
    });

    osc_send_thread = std::thread([this] ()
    {
        auto last_time = std::chrono::high_resolution_clock::now();
        double fps_total = 0;
        double fps_count = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        while (is_running())
        {
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
         auto start_time = std::chrono::high_resolution_clock::now();

         inference->set_dt(duration.count() / 1000.0);
         // 发送OSC数据
         std::vector<float> output = inference->get_output();
         if (!output.empty()) {
             updateCalibrationProgressBars(output, inference->getBlendShapeIndexMap());
             osc_manager->sendModelOutput(output);
         }

         auto end_time = std::chrono::high_resolution_clock::now();
         auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
         int delay_ms = max(0, static_cast<int>(1000.0 / 66.0 - elapsed));
         std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }
    });
}