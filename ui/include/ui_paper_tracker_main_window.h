/********************************************************************************
** Form generated from reading UI file 'paper_tracker_main_window.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAPER_TRACKER_MAIN_WINDOW_H
#define UI_PAPER_TRACKER_MAIN_WINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PaperTrackerMainWindow
{
public:
    QPushButton *FaceTrackerButton;
    QPushButton *EyeTrackerButton;
    QLabel *LOGOLabel;
    QPushButton *ClientUpdateButton;
    QLabel *ClientStatusLabel;
    QLabel *FaceTrackerInstructionLabel;
    QLabel *EyeTrackerInstructionLabel;

    void setupUi(QWidget *PaperTrackerMainWindow)
    {
        if (PaperTrackerMainWindow->objectName().isEmpty())
            PaperTrackerMainWindow->setObjectName("PaperTrackerMainWindow");
        PaperTrackerMainWindow->resize(585, 459);
        FaceTrackerButton = new QPushButton(PaperTrackerMainWindow);
        FaceTrackerButton->setObjectName("FaceTrackerButton");
        FaceTrackerButton->setGeometry(QRect(60, 240, 201, 81));
        EyeTrackerButton = new QPushButton(PaperTrackerMainWindow);
        EyeTrackerButton->setObjectName("EyeTrackerButton");
        EyeTrackerButton->setGeometry(QRect(310, 240, 201, 81));
        LOGOLabel = new QLabel(PaperTrackerMainWindow);
        LOGOLabel->setObjectName("LOGOLabel");
        LOGOLabel->setGeometry(QRect(-10, 0, 601, 231));
        LOGOLabel->setOpenExternalLinks(true);
        ClientUpdateButton = new QPushButton(PaperTrackerMainWindow);
        ClientUpdateButton->setObjectName("ClientUpdateButton");
        ClientUpdateButton->setGeometry(QRect(230, 340, 101, 51));
        ClientStatusLabel = new QLabel(PaperTrackerMainWindow);
        ClientStatusLabel->setObjectName("ClientStatusLabel");
        ClientStatusLabel->setGeometry(QRect(180, 410, 201, 20));
        QFont font;
        font.setBold(true);
        font.setItalic(true);
        ClientStatusLabel->setFont(font);
        ClientStatusLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
        FaceTrackerInstructionLabel = new QLabel(PaperTrackerMainWindow);
        FaceTrackerInstructionLabel->setObjectName("FaceTrackerInstructionLabel");
        FaceTrackerInstructionLabel->setGeometry(QRect(40, 340, 161, 51));
        QFont font1;
        font1.setPointSize(13);
        font1.setBold(true);
        font1.setItalic(true);
        FaceTrackerInstructionLabel->setFont(font1);
        FaceTrackerInstructionLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
        FaceTrackerInstructionLabel->setOpenExternalLinks(true);
        EyeTrackerInstructionLabel = new QLabel(PaperTrackerMainWindow);
        EyeTrackerInstructionLabel->setObjectName("EyeTrackerInstructionLabel");
        EyeTrackerInstructionLabel->setGeometry(QRect(360, 340, 161, 51));
        EyeTrackerInstructionLabel->setFont(font1);
        EyeTrackerInstructionLabel->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        EyeTrackerInstructionLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
        EyeTrackerInstructionLabel->setOpenExternalLinks(true);

        retranslateUi(PaperTrackerMainWindow);

        QMetaObject::connectSlotsByName(PaperTrackerMainWindow);
    } // setupUi

    void retranslateUi(QWidget *PaperTrackerMainWindow)
    {
        PaperTrackerMainWindow->setWindowTitle(QCoreApplication::translate("PaperTrackerMainWindow", "PaperTrackerMainWindow", nullptr));
        FaceTrackerButton->setText(QCoreApplication::translate("PaperTrackerMainWindow", "\351\235\242\346\215\225\347\225\214\351\235\242", nullptr));
        EyeTrackerButton->setText(QCoreApplication::translate("PaperTrackerMainWindow", "\347\234\274\350\277\275\347\225\214\351\235\242", nullptr));
        LOGOLabel->setText(QString());
        ClientUpdateButton->setText(QCoreApplication::translate("PaperTrackerMainWindow", "\345\256\242\346\210\267\347\253\257\346\233\264\346\226\260\346\243\200\346\237\245", nullptr));
        ClientStatusLabel->setText(QCoreApplication::translate("PaperTrackerMainWindow", "\346\227\240\346\263\225\350\277\236\346\216\245\345\210\260\346\234\215\345\212\241\345\231\250\357\274\214\350\257\267\346\243\200\346\237\245\347\275\221\347\273\234", nullptr));
        FaceTrackerInstructionLabel->setText(QCoreApplication::translate("PaperTrackerMainWindow", "\347\202\271\345\207\273\346\237\245\347\234\213\351\235\242\346\215\225\350\257\264\346\230\216\344\271\246", nullptr));
        EyeTrackerInstructionLabel->setText(QCoreApplication::translate("PaperTrackerMainWindow", "\347\202\271\345\207\273\346\237\245\347\234\213\347\234\274\350\277\275\350\257\264\346\230\216\344\271\246", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PaperTrackerMainWindow: public Ui_PaperTrackerMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAPER_TRACKER_MAIN_WINDOW_H
