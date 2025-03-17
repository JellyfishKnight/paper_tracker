#pragma once

#include <any>
#include <windows.h>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <QMutexLocker>
#include <QSerialPort>
#include <QObject>
#include <QSerialPort>
#include <QWaitCondition>
#include <QQueue>
#include <QThread>
#include <QTimer>

#include "logger.hpp"

enum SerialStatus
{
    CLOSED = 0,
    OPENED,
    FAILED,
    RESTARTING
};

// 定义数据包类型
enum PacketType {
    PACKET_UNKNOWN = 0,
    PACKET_WIFI_SETUP = 1,     // WiFi配置提示
    PACKET_WIFI_SSID_PWD = 2,  // 发送WiFi SSID/密码
    PACKET_WIFI_CONFIRM = 3,   // WiFi配置成功
    PACKET_WIFI_ERROR = 4,     // WiFi配置错误
    PACKET_DEVICE_STATUS = 5,  // 设备状态
    PACKET_LIGHT_CONTROL = 6   // 补光灯控制
};

class SerialPortManager : public QObject {
public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager() override;

    template<typename Func>
    void registerCallback(PacketType packetType, Func&& callback) {
        registerCallbackImpl(packetType, std::forward<Func>(callback), &Func::operator());
    }

    void init();

    void stop();

    void write_data(const std::string& data);

    SerialStatus status() const;

    // 发送WiFi配置信息
    void sendWiFiConfig(const std::string& ssid, const std::string& pwd);
    
    // 发送补光灯控制命令
    void sendLightControl(int brightness);

    void restartESP32(QWidget* window);

    void flashESP32(QWidget* window);

    // 查找ESP32-S3设备串口
    std::string FindEsp32S3Port();

    void stop_heartbeat_timer() const
    {
        if (heartBeatTimer && heartBeatTimer->isActive())
        {
            heartBeatTimer->stop();
        }
    }

    void start_heartbeat_timer()
    {
        if (!heartBeatTimer)
        {
            heartBeatTimer = new QTimer();
        }
        if (!heartBeatTimer->isActive())
        {
            heartBeatTimer->start(20);
        }
    }
private slots:
    void onReadyRead();

    void heartBeatTimeout();

private:
    // 辅助函数，用于推导函数参数类型
    template<typename Func, typename Class, typename... Args>
    void registerCallbackImpl(PacketType packetType, Func&& callback, void (Class::*)(Args...) const) {
        callbacks[packetType] = [callback = std::forward<Func>(callback)](const std::vector<std::any>& params) {
            if (sizeof...(Args) != params.size()) {
                LOG_ERROR("回调参数数量不匹配: 期望 {} 个参数，实际 {} 个参数",
                          sizeof...(Args), params.size());
                return;
            }
            invokeCallbackHelper<0, Args...>(callback, params);
        };
    }

    // 递归解包参数的辅助函数
    template<size_t I, typename T, typename... Rest, typename F>
    static void invokeCallbackHelper(F&& callback, const std::vector<std::any>& params) {
        if constexpr (sizeof...(Rest) == 0) {
            // 最后一个参数
            callback(std::any_cast<std::remove_reference_t<T>>(params[I]));
        } else {
            // 还有更多参数，递归展开
            invokeCallbackHelper<I+1, Rest...>(
                [&callback, param = std::any_cast<std::remove_reference_t<T>>(params[I])](Rest... args) {
                    callback(param, args...);
                },
                params
            );
        }
    }

    void handlePacket(PacketType packetType, const std::vector<std::any>& params);

    // 解析接收的数据包
    PacketType parsePacket(const std::string& packet);

    // 处理接收到的数据
    void processReceivedData(std::string& receivedData);

    std::string currentPort; // 默认端口
    QSerialPort* serialPort;
    SerialStatus m_status;

    std::mutex write_lock;

    QTimer* heartBeatTimer;
    int timeout_count = 0;

    std::map<PacketType, std::function<void(const std::vector<std::any>&)>> callbacks;
};