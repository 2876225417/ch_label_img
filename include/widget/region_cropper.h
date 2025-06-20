#ifndef REGION_CROPPER_H
#define REGION_CROPPER_H

#include "utils/non-copyable.h"
#include "widget/selection_box.h"
#include <QPoint>
#include <QRect>
#include <QWidget>
#include <qevent.h>
#include <qpaintdevice.h>
#include <qpoint.h>
#include <qvectornd.h>
#include <qwidget.h>

class RegionCropper: public QWidget, private NonCopyable {
    Q_OBJECT
public:
    explicit RegionCropper(QWidget* parent = nullptr);
    ~RegionCropper() override;

    [[nodiscard]] auto get_selection_rect() const -> const QRect&;

signals:
    void region_selected(const QRect& region);
    void mouse_moved(const QPoint& pos);

protected:
    void mousePressEvent(QMouseEvent*)   override;
    void mouseMoveEvent(QMouseEvent*)    override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*)      override;
private:
    bool   m_is_selecting;    // 记录当前框选的状态(状态：正在框选/未框选)
    QPoint m_start_point;     // 框选的起始位置
    QRect  m_current_rect;    // 框选的类矩形框

    SelectionBox* m_selection_box;

};

#endif // REGION_CROPPER_H