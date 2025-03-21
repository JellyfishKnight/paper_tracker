//
// Created by JellyfishKnight on 25-3-1.
//

#ifndef ROI_EVENT_HPP
#define ROI_EVENT_HPP

#include <QObject>
#include <QPainter>


class ROIEventFilter final : public QObject {
public:
    explicit ROIEventFilter(std::function<void(QRect rect, bool is_end)> func, QObject *parent = nullptr);
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
    QRect selectionRect;
    QPoint selectionStart;
    bool selecting;

    std::function<void(QRect rect, bool is_end)> func;
};


#endif //ROI_EVENT_HPP
