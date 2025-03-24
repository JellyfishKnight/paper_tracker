/********************************************************************************
** Form generated from reading UI file 'eye_tracker_window.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EYE_TRACKER_WINDOW_H
#define UI_EYE_TRACKER_WINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PaperEyeTrackerWindow
{
public:
    QStackedWidget *stackedWidget;
    QWidget *page;
    QPlainTextEdit *RightEyeIPAddress;
    QLabel *label_2;
    QPlainTextEdit *LeftEyeIPAddress;
    QComboBox *EnergyModelBox;
    QPlainTextEdit *LogText;
    QLabel *LeftEyeImage;
    QPushButton *RestartButton;
    QLabel *label;
    QPlainTextEdit *PassWordInput;
    QPlainTextEdit *SSIDInput;
    QLabel *RightEyeImage;
    QLabel *label_3;
    QPushButton *FlashButton;
    QPushButton *SendButton;
    QScrollBar *LeftBrightnessBar;
    QScrollBar *RightBrightnessBar;
    QLabel *label_4;
    QLabel *label_5;
    QWidget *page_2;
    QLabel *label_6;
    QPushButton *MainPageButton;
    QPushButton *SettingButton;
    QLabel *RightEyeWifiStatus;
    QLabel *EyeWindowSerialStatus;
    QLabel *LeftEyeWifiStatus;

    void setupUi(QWidget *PaperEyeTrackerWindow)
    {
        if (PaperEyeTrackerWindow->objectName().isEmpty())
            PaperEyeTrackerWindow->setObjectName("PaperEyeTrackerWindow");
        PaperEyeTrackerWindow->resize(907, 614);
        stackedWidget = new QStackedWidget(PaperEyeTrackerWindow);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setGeometry(QRect(0, 40, 901, 571));
        page = new QWidget();
        page->setObjectName("page");
        RightEyeIPAddress = new QPlainTextEdit(page);
        RightEyeIPAddress->setObjectName("RightEyeIPAddress");
        RightEyeIPAddress->setGeometry(QRect(530, 340, 201, 41));
        label_2 = new QLabel(page);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(470, 350, 54, 16));
        LeftEyeIPAddress = new QPlainTextEdit(page);
        LeftEyeIPAddress->setObjectName("LeftEyeIPAddress");
        LeftEyeIPAddress->setGeometry(QRect(530, 290, 201, 41));
        EnergyModelBox = new QComboBox(page);
        EnergyModelBox->addItem(QString());
        EnergyModelBox->addItem(QString());
        EnergyModelBox->addItem(QString());
        EnergyModelBox->setObjectName("EnergyModelBox");
        EnergyModelBox->setGeometry(QRect(800, 300, 91, 31));
        LogText = new QPlainTextEdit(page);
        LogText->setObjectName("LogText");
        LogText->setGeometry(QRect(20, 400, 721, 151));
        LeftEyeImage = new QLabel(page);
        LeftEyeImage->setObjectName("LeftEyeImage");
        LeftEyeImage->setGeometry(QRect(10, 5, 261, 261));
        RestartButton = new QPushButton(page);
        RestartButton->setObjectName("RestartButton");
        RestartButton->setGeometry(QRect(330, 290, 111, 41));
        label = new QLabel(page);
        label->setObjectName("label");
        label->setGeometry(QRect(470, 300, 54, 16));
        PassWordInput = new QPlainTextEdit(page);
        PassWordInput->setObjectName("PassWordInput");
        PassWordInput->setGeometry(QRect(20, 340, 171, 41));
        SSIDInput = new QPlainTextEdit(page);
        SSIDInput->setObjectName("SSIDInput");
        SSIDInput->setGeometry(QRect(20, 290, 171, 41));
        RightEyeImage = new QLabel(page);
        RightEyeImage->setObjectName("RightEyeImage");
        RightEyeImage->setGeometry(QRect(280, 5, 261, 261));
        label_3 = new QLabel(page);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(740, 300, 51, 31));
        FlashButton = new QPushButton(page);
        FlashButton->setObjectName("FlashButton");
        FlashButton->setGeometry(QRect(330, 340, 111, 41));
        SendButton = new QPushButton(page);
        SendButton->setObjectName("SendButton");
        SendButton->setGeometry(QRect(210, 290, 101, 91));
        LeftBrightnessBar = new QScrollBar(page);
        LeftBrightnessBar->setObjectName("LeftBrightnessBar");
        LeftBrightnessBar->setGeometry(QRect(660, 40, 231, 21));
        LeftBrightnessBar->setOrientation(Qt::Orientation::Horizontal);
        RightBrightnessBar = new QScrollBar(page);
        RightBrightnessBar->setObjectName("RightBrightnessBar");
        RightBrightnessBar->setGeometry(QRect(660, 80, 231, 21));
        RightBrightnessBar->setOrientation(Qt::Orientation::Horizontal);
        label_4 = new QLabel(page);
        label_4->setObjectName("label_4");
        label_4->setGeometry(QRect(580, 35, 54, 31));
        label_5 = new QLabel(page);
        label_5->setObjectName("label_5");
        label_5->setGeometry(QRect(580, 80, 54, 21));
        stackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName("page_2");
        label_6 = new QLabel(page_2);
        label_6->setObjectName("label_6");
        label_6->setGeometry(QRect(160, 140, 571, 281));
        QFont font;
        font.setPointSize(30);
        font.setBold(true);
        font.setItalic(true);
        label_6->setFont(font);
        label_6->setAlignment(Qt::AlignmentFlag::AlignCenter);
        stackedWidget->addWidget(page_2);
        MainPageButton = new QPushButton(PaperEyeTrackerWindow);
        MainPageButton->setObjectName("MainPageButton");
        MainPageButton->setGeometry(QRect(10, 10, 75, 24));
        SettingButton = new QPushButton(PaperEyeTrackerWindow);
        SettingButton->setObjectName("SettingButton");
        SettingButton->setGeometry(QRect(120, 10, 75, 24));
        RightEyeWifiStatus = new QLabel(PaperEyeTrackerWindow);
        RightEyeWifiStatus->setObjectName("RightEyeWifiStatus");
        RightEyeWifiStatus->setGeometry(QRect(680, 10, 161, 31));
        QFont font1;
        font1.setBold(true);
        font1.setItalic(true);
        RightEyeWifiStatus->setFont(font1);
        EyeWindowSerialStatus = new QLabel(PaperEyeTrackerWindow);
        EyeWindowSerialStatus->setObjectName("EyeWindowSerialStatus");
        EyeWindowSerialStatus->setGeometry(QRect(300, 15, 161, 21));
        EyeWindowSerialStatus->setFont(font1);
        LeftEyeWifiStatus = new QLabel(PaperEyeTrackerWindow);
        LeftEyeWifiStatus->setObjectName("LeftEyeWifiStatus");
        LeftEyeWifiStatus->setGeometry(QRect(490, 10, 161, 31));
        LeftEyeWifiStatus->setFont(font1);

        retranslateUi(PaperEyeTrackerWindow);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(PaperEyeTrackerWindow);
    } // setupUi

    void retranslateUi(QWidget *PaperEyeTrackerWindow)
    {
        PaperEyeTrackerWindow->setWindowTitle(QCoreApplication::translate("PaperEyeTrackerWindow", "PaperEyeTrackerWindow", nullptr));
        label_2->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\217\263\347\234\274IP", nullptr));
        EnergyModelBox->setItemText(0, QCoreApplication::translate("PaperEyeTrackerWindow", "\346\231\256\351\200\232\346\250\241\345\274\217", nullptr));
        EnergyModelBox->setItemText(1, QCoreApplication::translate("PaperEyeTrackerWindow", "\350\212\202\350\203\275\346\250\241\345\274\217", nullptr));
        EnergyModelBox->setItemText(2, QCoreApplication::translate("PaperEyeTrackerWindow", "\346\200\247\350\203\275\346\250\241\345\274\217", nullptr));

        LeftEyeImage->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "                          \346\262\241\346\234\211\345\233\276\345\203\217\350\276\223\345\205\245", nullptr));
        RestartButton->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\351\207\215\345\220\257", nullptr));
        label->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\267\246\347\234\274IP", nullptr));
        RightEyeImage->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "                          \346\262\241\346\234\211\345\233\276\345\203\217\350\276\223\345\205\245", nullptr));
        label_3->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\346\250\241\345\274\217\351\200\211\346\213\251", nullptr));
        FlashButton->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\210\267\345\206\231\345\233\272\344\273\266", nullptr));
        SendButton->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\217\221\351\200\201", nullptr));
        label_4->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\267\246\347\234\274\350\241\245\345\205\211", nullptr));
        label_5->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\217\263\347\234\274\350\241\245\345\205\211", nullptr));
        label_6->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\346\226\275\345\267\245\344\270\255\357\274\214\346\225\254\350\257\267\346\234\237\345\276\205", nullptr));
        MainPageButton->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\344\270\273\351\241\265\351\235\242", nullptr));
        SettingButton->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\350\256\276\347\275\256", nullptr));
        RightEyeWifiStatus->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\217\263\347\234\274WIFI\346\234\252\350\277\236\346\216\245", nullptr));
        EyeWindowSerialStatus->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\275\223\345\211\215\346\227\240\344\270\262\345\217\243\350\277\236\346\216\245", nullptr));
        LeftEyeWifiStatus->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "\345\267\246\347\234\274WIFI\346\234\252\350\277\236\346\216\245", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PaperEyeTrackerWindow: public Ui_PaperEyeTrackerWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EYE_TRACKER_WINDOW_H
