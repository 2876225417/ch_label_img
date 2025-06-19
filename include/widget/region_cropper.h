#ifndef REGION_CROPPER_H
#define REGION_CROPPER_H

#include "utils/non-copyable.h"
#include <QPoint>
#include <QRect>
#include <QWidget>
#include <qpaintdevice.h>
#include <qpoint.h>
#include <qwidget.h>

class RegionCropper: public QWidget, private NonCopyable {
    Q_OBJECT
public:
    explicit RegionCropper(QWidget* parent = nullptr);
    ~RegionCropper() override;

    [[nodiscard]] auto get_selection_rect() const -> const QRect&;

protected:
    void mousePressEvent(QMouseEvent*)   override;
    void mouseMoveEvent(QMouseEvent*)    override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*)        override;
private:
    bool   m_is_selecting;    // 记录当前框选的状态(状态：正在框选/未框选)
    QPoint m_start_point;     // 框选的起始位置
    QRect  m_selection_rect;  // 框选的类矩形框
};

#endif // REGION_CROPPER_H