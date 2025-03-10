#ifndef UPDATER_HPP
#define UPDATER_HPP

#include <iostream>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QProgressDialog>
#include <QProcess>
#include <QEventLoop>
#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <optional>

// 如果使用 QuaZip，请包含以下头文件
// #include <quazip.h>
// #include <quazipfile.h>

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

    // 同步获取远程版本信息（代码略，与之前类似）
    std::optional<Version> getClientVersionSync(QWidget* window);

    // 同步读取本地版本信息
    std::optional<Version> getCurrentVersion();

    // 同步下载 zip 更新包，并返回下载是否成功
    bool downloadZipSync(QWidget* window, const QString &zipFilePath, const QString &downloadUrl);

    // 解压 zip 文件到目标目录（使用 QuaZip 或其他方式实现）
    // 下面给出使用 QuaZip 的示例伪代码，实际使用时需要确保引入并配置好 QuaZip 库
    bool extractZip(const QString &zipFilePath, const QString &destinationPath);

    // 更新程序：下载 zip 包，解压覆盖文件，关闭当前程序并重启应用
    bool updateApplicationSync(QWidget* window, const Version &remoteVersion);
};

#endif // UPDATER_HPP
