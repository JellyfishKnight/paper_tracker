#ifndef UPDATER_HPP
#define UPDATER_HPP

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QMessageBox>
#include <QProgressDialog>
#include <QProcess>
#include <QEventLoop>
#include <optional>

// 定义版本相关结构体
struct ClientVersion {
    QString tag;
    QString description;
    QString firmware;
};

struct Version {
    ClientVersion version;
};

class Updater : public QObject {
public:
    explicit Updater(QObject *parent = nullptr) : QObject(parent) {}

    // 同步获取远程版本信息
    std::optional<Version> getClientVersionSync(QWidget* window) {
        QNetworkAccessManager manager;
        QNetworkRequest request(QUrl("http://47.116.163.1/version.json"));
        QNetworkReply* reply = manager.get(request);

        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(window, "错误", "获取版本信息失败: " + reply->errorString());
            reply->deleteLater();
            return std::nullopt;
        }

        QByteArray responseData = reply->readAll();
        reply->deleteLater();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        if (!doc.isObject()) {
            QMessageBox::critical(window, "错误", "版本信息格式错误");
            return std::nullopt;
        }

        QJsonObject obj = doc.object();
        QJsonObject versionObj = obj["version"].toObject();
        ClientVersion cv;
        cv.tag = versionObj["tag"].toString();
        cv.description = versionObj["description"].toString();
        cv.firmware = versionObj["firmware"].toString();

        return Version{cv};
    }

    // 同步读取本地版本信息（使用 QFile 和 QJsonDocument）
    std::optional<Version> getCurrentVersion() {
        QFile file("version.json");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning("无法打开文件: version.json");
            return std::nullopt;
        }
        QByteArray fileData = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(fileData);
        if (!doc.isObject()) {
            qWarning("JSON 格式错误");
            return std::nullopt;
        }

        QJsonObject obj = doc.object();
        QJsonObject versionObj = obj["version"].toObject();
        ClientVersion cv;
        cv.tag = versionObj["tag"].toString();
        cv.description = versionObj["description"].toString();
        cv.firmware = versionObj["firmware"].toString();

        return Version{cv};
    }

    // 同步下载更新文件，阻塞直到完成
    // 参数 remoteVersion 用于构造下载 URL 和输出文件名
    bool downloadAppToLocalSync(QWidget* window, const Version& remoteVersion) {
        QString url = "http://47.116.163.1/downloads/" + remoteVersion.version.tag;
        QString outputFilename = remoteVersion.version.tag + ".exe";

        QFile file(outputFilename);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(window, "错误", "无法创建文件: " + outputFilename);
            return false;
        }

        QNetworkAccessManager manager;
        QNetworkRequest request{QUrl(url)};
        QNetworkReply* reply = manager.get(request);

        // 创建进度对话框
        QProgressDialog progress("正在下载安装包，请稍候...", "取消", 0, 100, window);
        progress.setWindowTitle("下载");
        progress.setWindowModality(Qt::WindowModal);
        progress.setMinimumDuration(0);

        // 更新进度
        connect(reply, &QNetworkReply::downloadProgress, [&](qint64 bytesReceived, qint64 bytesTotal) {
            if (bytesTotal > 0) {
                progress.setMaximum(bytesTotal);
                progress.setValue(bytesReceived);
            }
            // 处理事件，保证进度条更新
            QCoreApplication::processEvents();
        });

        // 若用户取消下载，终止操作
        connect(&progress, &QProgressDialog::canceled, [&]() {
            reply->abort();
        });

        // 使用事件循环等待下载完成
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(window, "错误", "下载失败: " + reply->errorString());
            file.close();
            reply->deleteLater();
            return false;
        }

        // 将下载的数据写入文件
        file.write(reply->readAll());
        file.close();
        reply->deleteLater();

        return true;
    }
};

#endif // UPDATER_HPP
