//
// Created by JellyfishKnight on 25-3-3.
//

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <QPlainTextEdit>
#include <QDateTime>
#include <mutex>
#include <source_location>
#include <format>
#include <iostream>


class QString;

enum LogLevel
{
    debug = 0,
    info,
    warn,
    error
};

inline const static std::unordered_map<LogLevel, std::string> log_level_name {
    {debug, "DEBUG"},
    {info, "INFO"},
    {warn, "WARN"},
    {error, "ERROR"}
};

void append_log_window(QPlainTextEdit* log_window);

void remove_log_window(QPlainTextEdit* log_window);

template<class T>
struct with_source_location {
private:
    T inner;
    std::source_location loc;

public:
    template<class U>
        requires std::constructible_from<T, U>
    consteval with_source_location(U&& inner, std::source_location loc):
        inner(std::forward<U>(inner)),
        loc(std::move(loc)) {}

    constexpr T const& format() const {
        return inner;
    }

    constexpr std::source_location const& location() const {
        return loc;
    }
};

class Logger {
public:
    template <typename ...Args>
    static void log(LogLevel level, with_source_location<std::format_string<Args...>> format_str, Args&&... args)
    {
#ifdef DEBUG
        const auto& loc = format_str.location();
#endif
        auto msg = std::vformat(format_str.format().get(), std::make_format_args(args...));
        std::lock_guard<std::mutex> lock(pthis->log_mutex);
        QDateTime current_date_time = QDateTime::currentDateTime();
        auto current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss").toStdString();
#ifdef DEBUG
        auto message = std::format(
            "{} {}:{} [{}]:{}",
            current_date,
            loc.file_name(),
            loc.line(),
            log_level_name.at(level),
            msg
        );
#else
        auto message = std::format(
            "{} [{}]:{}",
            current_date,
            log_level_name.at(level),
            msg
        );
#endif
        QString log_message = QObject::tr(QString::fromStdString(message).toUtf8().constData());
        for (auto log_window : pthis->log_windows)
        {
            if (log_window->document()->blockCount() > 1000)
            {
                // 清空日志窗口
                QMetaObject::invokeMethod(log_window, "clear",
                              Qt::QueuedConnection);
            }

            if (level != debug)
            {
                QMetaObject::invokeMethod(log_window, "appendPlainText",
                              Qt::QueuedConnection, Q_ARG(QString, log_message));
            }
        }
#ifdef DEBUG
        std::cout << log_message.toStdString() << std::endl;
#endif
    }

private:
    Logger() = default;

    ~Logger() = default;

    static Logger* pthis;

    friend void append_log_window(QPlainTextEdit* log_window);

    friend void remove_log_window(QPlainTextEdit* log_window);

    std::vector<QPlainTextEdit*> log_windows;
    std::mutex log_mutex;
};


#define LOG_DEBUG(message, ...)\
    Logger::log(LogLevel::debug, { message, std::source_location::current() } __VA_OPT__(, ) __VA_ARGS__);
#define LOG_INFO(message, ...) \
    Logger::log(LogLevel::info, { message, std::source_location::current() } __VA_OPT__(, ) __VA_ARGS__);
#define LOG_WARN(message, ...) \
    Logger::log(LogLevel::warn, { message, std::source_location::current() } __VA_OPT__(, ) __VA_ARGS__);
#define LOG_ERROR(message, ...) \
    Logger::log(LogLevel::error, { message, std::source_location::current() } __VA_OPT__(, ) __VA_ARGS__);

#endif //LOGGER_HPP
