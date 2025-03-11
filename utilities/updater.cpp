//
// Created by JellyfishKnight on 25-3-10.
//
#include "updater.hpp"

#include <QApplication>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

std::optional<Version> Updater::getClientVersionSync(QWidget* window) {
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

// 同步读取本地版本信息
std::optional<Version> Updater::getCurrentVersion() {
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

// 同步下载 zip 更新包，并返回下载是否成功
bool Updater::downloadZipSync(QWidget* window, const QString &zipFilePath, const QString &downloadUrl) {
    QNetworkAccessManager manager;
    QNetworkRequest request{QUrl(downloadUrl)};
    QNetworkReply* reply = manager.get(request);

    QProgressDialog progress("正在下载更新包，请稍候...", "取消", 0, 100, window);
    progress.setWindowTitle("下载更新");
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);

    connect(reply, &QNetworkReply::downloadProgress, [&](qint64 bytesReceived, qint64 bytesTotal) {
        if (bytesTotal > 0) {
            progress.setMaximum(bytesTotal);
            progress.setValue(bytesReceived);
        }
        QCoreApplication::processEvents();
    });
    // 如果用户取消，则中止下载
    connect(&progress, &QProgressDialog::canceled, [&]() {
        reply->abort();
    });

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(window, "错误", "下载更新包失败: " + reply->errorString());
        reply->deleteLater();
        return false;
    }

    QFile file(zipFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(window, "错误", "无法创建文件: " + zipFilePath);
        reply->deleteLater();
        return false;
    }
    file.write(reply->readAll());
    file.close();
    reply->deleteLater();
    return true;
}

// 解压 zip 文件到目标目录（使用 QuaZip 或其他方式实现）
// 下面给出使用 QuaZip 的示例伪代码，实际使用时需要确保引入并配置好 QuaZip 库
bool Updater::extractZip(const QString &zipFilePath, const QString &destinationPath) {
    // 假设 ExtractZip.ps1 脚本与应用程序位于同一目录下
    QString appDir = QCoreApplication::applicationDirPath();
    QString scriptPath = QDir(appDir).filePath("scripts/extract.ps1");

    // 构造调用 PowerShell 脚本的参数
    QStringList args;
    args << "-NoProfile"                  // 不加载用户配置文件
         << "-ExecutionPolicy" << "Bypass"// 绕过执行策略
         << "-File" << scriptPath         // 指定脚本文件
         << "-zipPath" << zipFilePath     // 脚本参数：zip 文件路径
         << "-destination" << destinationPath; // 脚本参数：目标解压路径

    qDebug() << args.join(" ");
    // 使用 detach 模式启动 PowerShell 脚本，解压操作在后台独立运行
    bool detached = QProcess::startDetached("powershell", args);
    if (!detached) {
        QMessageBox::critical(nullptr, "错误", "无法启动 PowerShell 脚本（detach方式）");
        return false;
    }
    return true;
}

// 更新程序：下载 zip 包，解压覆盖文件，关闭当前程序并重启应用
bool Updater::updateApplicationSync(QWidget* window, const Version &remoteVersion)
{
    // 假设服务器提供的更新包 URL 为：<下载地址>/<版本tag>.zip
    QString downloadUrl = "http://47.116.163.1/downloads/" + remoteVersion.version.tag;
    // 保存 zip 文件到当前应用目录下（你也可以选择临时目录）
    QString appDir = QCoreApplication::applicationDirPath();
    QString zipFilePath = QDir(appDir).filePath("update.zip");

    // 同步下载 zip 更新包
    if (!downloadZipSync(window, zipFilePath, downloadUrl))
        return false;

    QMessageBox::information(window, "提示", "更新包下载成功，重启软件以应用更新");

    // 解压更新包到当前目录，覆盖已有文件
    extractZip(zipFilePath, appDir);

    QApplication::quit();
    return true;
}