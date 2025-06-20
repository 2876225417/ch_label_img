#include "widget/selection_box.h"
#include <qcolor.h>
#include <qevent.h>
#include <qpaintdevice.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qvectornd.h>
#include <qwidget.h>
#include <widget/region_cropper.h>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QApplication>

auto RegionCropper::get_selection_rect() const -> const QRect& {
    return m_current_rect;
}

void RegionCropper::mousePressEvent(QMouseEvent* event) {
    // 鼠标按下时切换 框选状态
    // 鼠标按下时的位置为 初始位置
    if (event->button() == Qt::LeftButton) { // 只响应鼠标左键
        m_is_selecting = true;
        m_start_point = event->pos();
        m_current_rect = QRect(m_start_point, QSize());
        m_selection_box->set_selection_rect(m_current_rect);
        m_selection_box->show();
    }
}

void RegionCropper::mouseMoveEvent(QMouseEvent* event) {
    
    emit mouse_moved(event->pos());

    if (m_is_selecting) { // 鼠标在 框选状态激活时 移动会进行框选
        m_current_rect = QRect(m_start_point, event->pos()).normalized();
        m_selection_box->set_selection_rect(m_current_rect);
    }
}

void RegionCropper::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && // 鼠标释放时需为左键
        m_is_selecting) {                    // 且是处在框选状态的
        m_is_selecting = false;
        if (m_current_rect.width() > 0 &&
            m_current_rect.height()
           ) emit region_selected(m_current_rect);
    }
}

void RegionCropper::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    m_selection_box->setGeometry(this->rect());
}


RegionCropper::RegionCropper(QWidget* parent)
    : QWidget{parent}
    , m_is_selecting{false}
    {
    setMouseTracking(true);

    m_selection_box = new SelectionBox{this};
}

RegionCropper::~RegionCropper() = default;








