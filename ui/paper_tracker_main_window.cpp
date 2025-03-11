//
// Created by JellyfishKnight on 25-3-11.
//

// You may need to build the project (run Qt uic code generator) to get "ui_paper_tracker_main_window.h" resolved

#include "paper_tracker_main_window.hpp"

#include <paper_eye_tracker_window.hpp>
#include <paper_face_tracker_window.hpp>
#include <QFile>
#include <QTimer>

#include "ui_paper_tracker_main_window.h"


PaperTrackerMainWindow::PaperTrackerMainWindow(QWidget *parent) :
    QWidget(parent) {
    ui.setupUi(this);
    setFixedSize(585, 459);
    // set logo
    QFile Logo = QFile("./resources/logo.png");
    Logo.open(QIODevice::ReadOnly);
    QPixmap pixmap;
    pixmap.loadFromData(Logo.readAll());
    auto final_map = pixmap.scaled(ui.LOGOLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    Logo.close();
    ui.LOGOLabel->setAlignment(Qt::AlignCenter);
    ui.LOGOLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui.LOGOLabel->setScaledContents(true);
    ui.LOGOLabel->setPixmap(final_map);
    ui.LOGOLabel->update();

    updater = std::make_shared<Updater>();
    connect_callbacks();

    QTimer::singleShot(1000, this, [this] ()
    {
        auto remote_opt = updater->getClientVersionSync(nullptr);
        if (remote_opt.has_value())
        {
            const auto& remote_version = remote_opt.value();
            auto curr_opt = updater->getCurrentVersion();
            if (curr_opt.has_value())
            {
                const auto& curr_version = curr_opt.value();
                if (curr_version.version.tag != remote_version.version.tag)
                {
                    ui.ClientStatusLabel->setText(tr("当前客户端版本过低，请更新"));
                } else
                {
                    ui.ClientStatusLabel->setText(tr("当前客户端版本为最新版本"));
                }
            } else
            {
                ui.ClientStatusLabel->setText(tr("无法获取到当前客户端版本"));
            }
        } else
        {
            ui.ClientStatusLabel->setText(tr("无法连接到服务器，请检查网络"));
        }
    });

}

PaperTrackerMainWindow::~PaperTrackerMainWindow() = default;

void PaperTrackerMainWindow::onFaceTrackerButtonClicked()
{
    auto window = new PaperFaceTrackerWindow();
    window->setAttribute(Qt::WA_DeleteOnClose);  // 关闭时自动释放内存
    window->setWindowModality(Qt::NonModal);     // 设置为非模态
    window->setWindowIcon(this->windowIcon());
    window->show();
}

void PaperTrackerMainWindow::onEyeTrackerButtonClicked()
{
    auto window = new PaperEyeTrackerWindow();
    window->setAttribute(Qt::WA_DeleteOnClose);  // 关闭时自动释放内存
    window->setWindowModality(Qt::NonModal);     // 设置为非模态
    window->setWindowIcon(this->windowIcon());
    window->show();
}

void PaperTrackerMainWindow::onUpdateButtonClicked()
{
    // 1. 同步获取远程版本信息
    auto remoteVersionOpt = updater->getClientVersionSync(this);
    if (!remoteVersionOpt.has_value()) {
        return;
    }
    // 2. 获取本地版本信息
    auto currentVersionOpt = updater->getCurrentVersion();
    if (!currentVersionOpt.has_value()) {
        QMessageBox::critical(this, tr("错误"), tr("无法获取当前客户端版本信息"));
        return;
    }
    // 3. 如果版本不同，则执行更新
    if (remoteVersionOpt.value().version.tag != currentVersionOpt.value().version.tag) {
        auto reply = QMessageBox::question(this, tr("版本检查"),
            tr(("当前客户端版本不是最新版本是否更新？\n版本更新信息如下：\n" +
            remoteVersionOpt.value().version.description).toUtf8().constData()),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            if (!updater->updateApplicationSync(this, remoteVersionOpt.value())) {
                // 更新过程中有错误，提示信息已在内部处理
                return;
            }
            // 若更新成功，updateApplicationSync 内部会重启程序并退出当前程序
        }
    } else {
        QMessageBox::information(this, tr("版本检查"), tr("当前客户端版本已是最新版本"));
    }
}

void PaperTrackerMainWindow::connect_callbacks()
{
    connect(ui.FaceTrackerButton, &QPushButton::clicked, this, &PaperTrackerMainWindow::onFaceTrackerButtonClicked);
    connect(ui.EyeTrackerButton, &QPushButton::clicked, this, &PaperTrackerMainWindow::onEyeTrackerButtonClicked);
    connect(ui.ClientUpdateButton, &QPushButton::clicked, this, &PaperTrackerMainWindow::onUpdateButtonClicked);
}
