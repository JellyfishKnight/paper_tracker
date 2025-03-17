//
// Created by paper on 25-3-15.
//
// http_server.cpp - HTTP服务器实现
#include "http_server.hpp"
#include <QDateTime>
#include <QTimer>
#include <QBuffer>
#include <opencv2/imgcodecs.hpp>
#include "logger.hpp"

#define STREAM_BOUNDARY "123456789000000000000987654321"
#define STREAM_CONTENT_TYPE "multipart/x-mixed-replace;boundary=" STREAM_BOUNDARY
#define STREAM_BOUNDARY_START "--" STREAM_BOUNDARY "\r\n"
#define STREAM_PART "Content-Type: image/jpeg\r\nContent-Length: %1\r\n\r\n"

HttpServer::HttpServer(QObject *parent) : QObject(parent) {
    server = new QTcpServer(this);
    isRunning = false;

    // 当有新连接时触发newConnection槽函数
    connect(server, &QTcpServer::newConnection, this, &HttpServer::newConnection);
}

HttpServer::~HttpServer() {
    stop();
    delete server;
}

bool HttpServer::start(quint16 port) {
    if (isRunning) {
        return true;
    }

    if (!server->listen(QHostAddress::Any, port)) {
        LOG_ERROR("HTTP服务器启动失败: {}", server->errorString().toStdString());
        return false;
    }

    isRunning = true;
    LOG_INFO("HTTP服务器已启动，监听端口: {}", port);
    return true;
}

void HttpServer::stop() {
    if (!isRunning) {
        return;
    }

    server->close();

    // 关闭所有客户端连接
    for (QTcpSocket* socket : streamClients) {
        socket->disconnectFromHost();
        if (socket->state() != QAbstractSocket::UnconnectedState) {
            socket->waitForDisconnected(1000);
        }
        socket->deleteLater();
    }
    streamClients.clear();

    isRunning = false;
    LOG_INFO("HTTP服务器已停止");
}

void HttpServer::updateFrame(const cv::Mat &frame) {
    QMutexLocker locker(&frameMutex);

    if (frame.empty() || streamClients.isEmpty()) {
        return;
    }

    // 更新当前帧并转换为JPEG
    frame.copyTo(currentFrame);
    jpegBuffer = createJpegFromFrame(currentFrame);

    // 发送给所有客户端
    QMetaObject::invokeMethod(this, [this] ()
    {
        sendFrame();
    }, Qt::QueuedConnection);
    // QMetaObject::invokeMethod(this, "sendFrame", Qt::QueuedConnection);
}

void HttpServer::newConnection() {
    while (server->hasPendingConnections()) {
        QTcpSocket *socket = server->nextPendingConnection();

        // 连接信号槽
        connect(socket, &QTcpSocket::readyRead, this, &HttpServer::readyRead);
        connect(socket, &QTcpSocket::disconnected, this, &HttpServer::clientDisconnected);

        LOG_INFO("HTTP服务器：新连接来自 {}", socket->peerAddress().toString().toStdString());
    }
}

void HttpServer::clientDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        streamClients.removeAll(socket);
        socket->deleteLater();
        LOG_INFO("HTTP服务器：客户端断开连接，剩余连接数: {}", streamClients.size());
    }
}

void HttpServer::readyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray request = socket->readAll();
    handleRequest(socket, request);
}

void HttpServer::handleRequest(QTcpSocket *socket, const QByteArray &request) {
    // 简单解析HTTP请求，只检查第一行
    QString requestStr = QString::fromUtf8(request);
    QStringList lines = requestStr.split("\r\n");

    if (lines.isEmpty()) {
        return;
    }

    QStringList firstLineParts = lines[0].split(" ");
    if (firstLineParts.size() < 2) {
        return;
    }

    QString method = firstLineParts[0];
    QString path = firstLineParts[1];

    LOG_INFO("HTTP服务器：收到请求 {} {}", method.toStdString(), path.toStdString());

    if (method == "GET") {
        if (path == "/" || path == "/stream") {
            // 添加到流客户端列表
            if (!streamClients.contains(socket)) {
                streamClients.append(socket);
            }

            // 发送MJPEG流头部
            sendStreamHeader(socket);

            // 如果有当前帧，立即发送
            QMutexLocker locker(&frameMutex);
            if (!currentFrame.empty()) {
                sendStreamBoundary(socket);
                sendStreamContent(socket);
            }
        }
        else if (path == "/favicon.ico") {
            // 返回空图标
            sendHttpResponse(socket, QByteArray(), "image/x-icon");
        }
        else {
            // 返回简单的HTML页面
            QByteArray html =
                "<!DOCTYPE html>"
                "<html>"
                "<head><title>摄像头流</title></head>"
                "<body>"
                "<h1>摄像头实时流</h1>"
                "<img src=\"/stream\" width=\"640\" height=\"480\" />"
                "</body>"
                "</html>";

            sendHttpResponse(socket, html);
        }
    }
    else {
        // 不支持的请求方法
        QByteArray errorMsg = "不支持的请求方法";
        sendHttpResponse(socket, errorMsg, "text/plain", 405);
    }
}

void HttpServer::sendHttpResponse(QTcpSocket *socket, const QByteArray &content,
                                 const QByteArray &contentType, int statusCode) {
    QByteArray response;
    QByteArray status = "200 OK";

    if (statusCode != 200) {
        status = QByteArray::number(statusCode) + " ";
        if (statusCode == 404) status += "Not Found";
        else if (statusCode == 405) status += "Method Not Allowed";
        else status += "Error";
    }

    response.append("HTTP/1.1 " + status + "\r\n");
    response.append("Content-Type: " + contentType + "\r\n");
    response.append("Content-Length: " + QByteArray::number(content.size()) + "\r\n");
    response.append("Connection: close\r\n");
    response.append("Access-Control-Allow-Origin: *\r\n");
    response.append("\r\n");
    response.append(content);

    socket->write(response);
    socket->flush();

    // 如果不是流请求，发送完响应后关闭连接
    if (contentType != STREAM_CONTENT_TYPE) {
        socket->disconnectFromHost();
    }
}

void HttpServer::sendStreamHeader(QTcpSocket *socket) {
    QByteArray response;
    response.append("HTTP/1.1 200 OK\r\n");
    response.append("Content-Type: " + QByteArray(STREAM_CONTENT_TYPE) + "\r\n");
    response.append("Cache-Control: no-cache, no-store, must-revalidate, private\r\n");
    response.append("Pragma: no-cache\r\n");
    response.append("Expires: 0\r\n");
    response.append("Access-Control-Allow-Origin: *\r\n");
    response.append("Connection: close\r\n");
    response.append("\r\n");

    socket->write(response);
    socket->flush();
}

void HttpServer::sendStreamBoundary(QTcpSocket *socket) {
    socket->write(STREAM_BOUNDARY_START);
    socket->flush();
}

void HttpServer::sendStreamContent(QTcpSocket *socket) {
    // 构建JPEG帧的HTTP部分
    QByteArray header = QString(STREAM_PART).arg(jpegBuffer.size()).toUtf8();

    // 发送帧
    socket->write(header);
    socket->write(jpegBuffer);
    socket->write("\r\n");
    socket->flush();
}

void HttpServer::sendFrame() {
    QMutexLocker locker(&frameMutex);

    // 移除无效的套接字
    QList<QTcpSocket*> invalidSockets;
    for (QTcpSocket* socket : streamClients) {
        if (socket->state() != QAbstractSocket::ConnectedState) {
            invalidSockets.append(socket);
            continue;
        }

        // 发送帧数据
        sendStreamBoundary(socket);
        sendStreamContent(socket);
    }

    // 移除无效套接字
    for (QTcpSocket* socket : invalidSockets) {
        streamClients.removeAll(socket);
        socket->deleteLater();
    }
}

QByteArray HttpServer::createJpegFromFrame(const cv::Mat &frame) {
    std::vector<uchar> buffer;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 80}; // 使用80%的JPEG质量

    cv::imencode(".jpg", frame, buffer, params);
    return QByteArray(reinterpret_cast<const char*>(buffer.data()), static_cast<int>(buffer.size()));
}