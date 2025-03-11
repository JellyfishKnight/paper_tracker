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
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PaperEyeTrackerWindow
{
public:
    QLabel *label;

    void setupUi(QWidget *PaperEyeTrackerWindow)
    {
        if (PaperEyeTrackerWindow->objectName().isEmpty())
            PaperEyeTrackerWindow->setObjectName("PaperEyeTrackerWindow");
        PaperEyeTrackerWindow->resize(671, 442);
        label = new QLabel(PaperEyeTrackerWindow);
        label->setObjectName("label");
        label->setGeometry(QRect(120, 110, 421, 206));
        QFont font;
        font.setFamilies({QString::fromUtf8("Microsoft JhengHei")});
        font.setPointSize(28);
        font.setBold(true);
        font.setItalic(true);
        label->setFont(font);
        label->setAlignment(Qt::AlignmentFlag::AlignCenter);

        retranslateUi(PaperEyeTrackerWindow);

        QMetaObject::connectSlotsByName(PaperEyeTrackerWindow);
    } // setupUi

    void retranslateUi(QWidget *PaperEyeTrackerWindow)
    {
        PaperEyeTrackerWindow->setWindowTitle(QCoreApplication::translate("PaperEyeTrackerWindow", "PaperEyeTrackerWindow", nullptr));
        label->setText(QCoreApplication::translate("PaperEyeTrackerWindow", "PaperEyeTracker\n"
"\345\212\237\350\203\275\345\274\200\345\217\221\344\270\255\357\274\214\346\225\254\350\257\267\346\234\237\345\276\205\357\274\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PaperEyeTrackerWindow: public Ui_PaperEyeTrackerWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAPER_EYE_TRACKER_WINDOW_H
