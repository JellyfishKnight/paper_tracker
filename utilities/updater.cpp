//
// Created by JellyfishKnight on 25-3-10.
//
#include "updater.hpp"
#include <QTimer>
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
    // 创建临时目录用于解压，避免直接修改目标目录
    QString tempDir = QDir::tempPath() + "/paper_tracker_update_" +
                     QString::number(QDateTime::currentMSecsSinceEpoch());

    QDir().mkpath(tempDir);

    qDebug() << "创建临时目录用于解压: " << tempDir;

    // 首先显示确认对话框
    QMessageBox confirmBox;
    confirmBox.setWindowTitle("确认更新");
    confirmBox.setText("已准备好下载并安装更新。");
    confirmBox.setInformativeText("点击确定继续更新，这将关闭应用程序并替换当前文件。");
    confirmBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    confirmBox.setDefaultButton(QMessageBox::Ok);
    confirmBox.setWindowModality(Qt::ApplicationModal); // 确保这是应用级模态

    if (confirmBox.exec() == QMessageBox::Cancel) {
        QDir(tempDir).removeRecursively();
        return false;
    }

    // 创建单一进度对话框
    QProgressDialog progress("正在更新...", "取消", 0, 100);
    progress.setWindowTitle("更新安装");
    progress.setWindowModality(Qt::WindowModal);
    progress.setValue(10);
    progress.setLabelText("正在解压文件...");
    progress.show();
    QCoreApplication::processEvents(); // 确保对话框立即显示

    // 构造PowerShell命令
    QString command = QString(
        "Add-Type -AssemblyName System.IO.Compression.FileSystem; "
        "try { "
        "  [System.IO.Compression.ZipFile]::ExtractToDirectory('%1', '%2'); "
        "  Write-Host 'Files extracted to temp directory'; "
        "  $fileCount = (Get-ChildItem -Path '%2' -Recurse -File).Count; "
        "  if ($fileCount -eq 0) { throw 'Extraction failed - no files found' } "
        "  Write-Host \"Extracted $fileCount files successfully\"; "
        "  exit 0; "
        "} catch { "
        "  Write-Host \"Extraction error: $_\"; "
        "  exit 1; "
        "}"
    ).arg(QDir::toNativeSeparators(zipFilePath).replace("'", "''"),
          QDir::toNativeSeparators(tempDir).replace("'", "''"));

    // 构造命令行参数
    QStringList args;
    args << "-NoProfile"
         << "-ExecutionPolicy" << "Bypass"
         << "-Command" << command;

    qDebug() << "执行命令: powershell" << args.join(" ");

    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start("powershell", args);

    if (!process.waitForStarted(5000)) {
        progress.close();
        QMessageBox::critical(nullptr, "错误", "无法启动解压过程: " + process.errorString());
        QDir(tempDir).removeRecursively();
        return false;
    }

    // 定时更新进度对话框
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        if (process.state() == QProcess::Running) {
            QString output = process.readAllStandardOutput();
            if (!output.isEmpty()) {
                qDebug() << "PowerShell输出:" << output;
                // 更新对话框文本
                progress.setLabelText("正在解压文件...\n" + output.simplified().left(80));
                progress.setValue(30); // 更新进度
            }
        }
    });
    timer.start(100);

    // 连接取消按钮
    QObject::connect(&progress, &QProgressDialog::canceled, [&]() {
        process.kill();
        timer.stop();
        QDir(tempDir).removeRecursively();
    });

    // 等待处理完成
    QEventLoop loop;
    QObject::connect(&process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                    &loop, &QEventLoop::quit);
    loop.exec();

    timer.stop();

    // 检查解压结果
    if (process.exitCode() != 0) {
        progress.close();
        QString error = process.readAll();
        QMessageBox::critical(nullptr, "解压失败",
                             "无法解压更新文件: " + error.simplified());
        QDir(tempDir).removeRecursively();
        return false;
    }

    // 更新进度
    progress.setValue(50);
    progress.setLabelText("正在准备安装更新...");
    QCoreApplication::processEvents();

    // 检查临时目录是否包含文件
    QDir tempDirObj(tempDir);
    QStringList entries = tempDirObj.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    if (entries.isEmpty()) {
        progress.close();
        QMessageBox::critical(nullptr, "更新失败", "解压后没有找到任何文件");
        QDir(tempDir).removeRecursively();
        return false;
    }

    // 更新进度
    progress.setValue(70);
    progress.setLabelText("正在准备复制文件...");
    QCoreApplication::processEvents();

    // 创建批处理文件进行复制和重启
    QString batchPath = tempDir + "/update.bat";
    QFile batchFile(batchPath);
    if (!batchFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        progress.close();
        QMessageBox::critical(nullptr, "更新失败", "无法创建更新脚本");
        QDir(tempDir).removeRecursively();
        return false;
    }

    QTextStream stream(&batchFile);
    stream << "@echo off\n";
    stream << "echo 正在更新 PaperTracker...\n";
    stream << "timeout /t 1 /nobreak >nul\n";

    // 复制临时目录中的所有文件到目标目录
    stream << QString("xcopy \"%1\\*\" \"%2\" /E /Y /I /H /R\n")
                  .arg(QDir::toNativeSeparators(tempDir))
                  .arg(QDir::toNativeSeparators(destinationPath));

    // 删除临时目录
    stream << QString("rmdir /S /Q \"%1\"\n").arg(QDir::toNativeSeparators(tempDir));

    // 启动应用程序
    QString exePath = QDir(destinationPath).filePath("PaperTracker.exe");
    stream << "cd /d \"" << QDir::toNativeSeparators(destinationPath) << "\"\n";
    stream << "echo 启动应用程序中...\n";
    stream << "start \"\" \"" << QDir::toNativeSeparators(exePath) << "\"\n";
    stream << "if ERRORLEVEL 1 (\n";
    stream << "  echo 启动应用程序失败，请手动启动PaperTracker.exe\n";
    stream << "  pause\n";
    stream << ")\n";

    // 删除自身
    stream << "del \"%~f0\"\n";

    batchFile.close();

    // 更新进度
    progress.setValue(90);
    progress.setLabelText("正在完成更新，请手动重启程序");
    QCoreApplication::processEvents();

    // 给用户一点时间查看最终进度
    QTimer::singleShot(1500, [&]() {
        // 启动批处理文件并退出应用
        QProcess::startDetached("cmd.exe", {"/C", QDir::toNativeSeparators(batchPath)});
        progress.setValue(100);
        QCoreApplication::quit();
    });
    // 给当前进程一些时间来完全释放文件，避免文件锁问题
    QProcess::startDetached("cmd.exe", {"/C",
        QString("timeout /t 2 /nobreak >nul && \"%1\"")
            .arg(QDir::toNativeSeparators(batchPath))});
    // 进入事件循环，等待应用退出
    QEventLoop finalLoop;
    QTimer::singleShot(2000, &finalLoop, &QEventLoop::quit);
    finalLoop.exec();

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