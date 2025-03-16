/********************************************************************************
** Form generated from reading UI file 'paper_eye_tracker_window.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAPER_EYE_TRACKER_WINDOW_H
#define UI_PAPER_EYE_TRACKER_WINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PaperEyeTrackerWindow
{
public:
    QLabel *LeftEyeImage;
    QLabel *RightEyeImage;
    QPlainTextEdit *SSIDInput;
    QPlainTextEdit *PassWordInput;
    QPushButton *SendButton;
    QPushButton *RestartButton;
    QPushButton *FlashButton;
    QLabel *EyeWindowSerialStatus;
    QLabel *LeftEyeWifiStatus;
    QLabel *RightEyeWifiStatus;
    QLabel *label;
    QLabel *label_2;
    QPlainTextEdit *LeftEyeIPAddress;
    QPlainTextEdit *RightEyeIPAddress;

    void setupUi(QWidget *PaperEyeTrackerWindow)
    {
        if (PaperEyeTrackerWindow->objectName().isEmpty())
            PaperEyeTrackerWindow->setObjectName("PaperEyeTrackerWindow");
        PaperEyeTrackerWindow->resize(883, 596);
        LeftEyeImage = new QLabel(PaperEyeTrackerWindow);
        LeftEyeImage->setObjectName("LeftEyeImage");
        LeftEyeImage->setGeometry(QRect(10, 40, 261, 261));
        RightEyeImage = new QLabel(PaperEyeTrackerWindow);
        RightEyeImage->setObjectName("RightEyeImage");
        RightEyeImage->setGeometry(QRect(310, 40, 261, 261));
        SSIDInput = new QPlainTextEdit(PaperEyeTrackerWindow);
        SSIDInput->setObjectName("SSIDInput");
        SSIDInput->setGeometry(QRect(10, 370, 171, 41));
        PassWordInput = new QPlainTextEdit(PaperEyeTrackerWindow);
        PassWordInput->setObjectName("PassWordInput");
        PassWordInput->setGeometry(QRect(10, 420, 171, 41));
        SendButton = new QPushButton(PaperEyeTrackerWindow);
        SendButton->setObjectName("SendButton");
        SendButton->setGeometry(QRect(200, 370, 101, 91));
        RestartButton = new QPushButton(PaperEyeTrackerWindow);
        RestartButton->setObjectName("RestartButton");
        RestartButton->setGeometry(QRect(320, 370, 111, 41));
        FlashButton = new QPushButton(PaperEyeTrackerWindow);
        FlashButton->setObjectName("FlashButton");
        FlashButton->setGeometry(QRect(320, 420, 111, 41));
        EyeWindowSerialStatus = new QLabel(PaperEyeTrackerWindow);
        EyeWindowSerialStatus->setObjectName("EyeWindowSerialStatus");
        EyeWindowSerialStatus->setGeometry(QRect(10, 335, 161, 21));
        QFont font;
        font.setBold(true);
        font.setItalic(true);
        EyeWindowSerialStatus->setFont(font);
        LeftEyeWifiStatus = new QLabel(PaperEyeTrackerWindow);
        LeftEyeWifiStatus->setObjectName("LeftEyeWifiStatus");
        LeftEyeWifiStatus->setGeometry(QRect(200, 330, 161, 31));
        LeftEyeWifiStatus->setFont(font);
        RightEyeWifiStatus = new QLabel(PaperEyeTrackerWindow);
        RightEyeWifiStatus->setObjectName("RightEyeWifiStatus");
        RightEyeWifiStatus->setGeometry(QRect(390, 330, 161, 31));
        RightEyeWifiStatus->setFont(font);
        label = new QLabel(PaperEyeTrackerWindow);
        label->setObjectName("label");
        label->setGeometry(QRect(460, 380, 54, 16));
        label_2 = new QLabel(PaperEyeTrackerWindow);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(460, 430, 54, 16));
        LeftEyeIPAddress = new QPlainTextEdit(PaperEyeTrackerWindow);
        LeftEyeIPAddress->setObjectName("LeftEyeIPAddress");
        LeftEyeIPAddress->setGeometry(QRect(520, 370, 211, 41));
        RightEyeIPAddress = new QPlainTextEdit(PaperEyeTrackerWindow);
        RightEyeIPAddress->setObjectName("RightEyeIPAddress");
        RightEyeIPAddress->setGeometry(QRect(520, 420, 211, 41));

        retranslateUi(PaperEyeTrackerWindow);

        QMetaObject::connectSlotsByName(PaperEyeTrackerWindow);
    } // setupUi

    void retranslateUi(QWidget *PaperEyeTrackerWindow)
    {
        PaperEyeTrackerWindow->setWindowTitle(QCoreApplication::translate("PaperEyeTrackerWindow", "PaperEyeTrackerWindow", nullptr));
        LeftEyeImage->setText(QString());
        RightEyeImage->setText(QString());
        SendButton->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\217\221\351\200\201", nullptr));
        RestartButton->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\351\207\215\345\220\257", nullptr));
        FlashButton->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\210\267\345\206\231\345\233\272\344\273\266", nullptr));
        EyeWindowSerialStatus->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\275\223\345\211\215\346\227\240\344\270\262\345\217\243\350\277\236\346\216\245", nullptr));
        LeftEyeWifiStatus->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\267\246\347\234\274WIFI\346\234\252\350\277\236\346\216\245", nullptr));
        RightEyeWifiStatus->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\217\263\347\234\274WIFI\346\234\252\350\277\236\346\216\245", nullptr));
        label->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\267\246\347\234\274IP", nullptr));
        label_2->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\217\263\347\234\274IP", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PaperEyeTrackerWindow: public Ui_PaperEyeTrackerWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAPER_EYE_TRACKER_WINDOW_H
