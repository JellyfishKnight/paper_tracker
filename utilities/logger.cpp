//
// Created by JellyfishKnight on 25-3-4.
//
#include "logger.hpp"
#include "iostream"
#include <QTranslator>


void init_logger(QPlainTextEdit* log_window)
{
    Logger::pthis = new Logger(log_window);
}

