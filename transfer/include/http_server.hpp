// http_server.hpp - 新增的HTTP服务器头文件
#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <opencv2/core.hpp>
#include <QMutex>

class HttpServer : public QObject {
public:
    explicit HttpServer(QObject *parent = nullptr);
    ~HttpServer();

    bool start(quint16 port = 80);
    void stop();
    void updateFrame(const cv::Mat &frame);

private slots:
    void newConnection();
    void clientDisconnected();
    void readyRead();
    void sendFrame();

private:
    QTcpServer *server;
    QList<QTcpSocket*> streamClients;
    QMutex frameMutex;
    cv::Mat currentFrame;
    QByteArray jpegBuffer;
    bool isRunning;

    void handleRequest(QTcpSocket *socket, const QByteArray &request);
    void sendHttpResponse(QTcpSocket *socket, const QByteArray &content,
                         const QByteArray &contentType = "text/html",
                         int statusCode = 200);
    void sendStreamHeader(QTcpSocket *socket);
    void sendStreamBoundary(QTcpSocket *socket);
    void sendStreamContent(QTcpSocket *socket);
    QByteArray createJpegFromFrame(const cv::Mat &frame);
};

#endif // HTTP_SERVER_HPP