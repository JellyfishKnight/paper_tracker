//
// Created by JellyfishKnight on 25-3-4.
//
#include "logger.hpp"
#include "iostream"
#include <QTranslator>

Logger* Logger::pthis = new Logger();

void append_log_window(QPlainTextEdit* log_window)
{
    Logger::pthis->log_windows.emplace_back(log_window);
}

void remove_log_window(QPlainTextEdit* log_window)
{
    for (auto it = Logger::pthis->log_windows.begin(); it != Logger::pthis->log_windows.end(); it++)
    {
        if (*it == log_window)
        {
            Logger::pthis->log_windows.erase(it);
            break;
        }
    }
}
