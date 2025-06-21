#include "widget/selection_box.h"
#include <qapplication.h>
#include <qcolor.h>
#include <qevent.h>
#include <qnamespace.h>
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

void RegionCropper::mousePressEvent(QMouseEvent* event) {
    // 鼠标按下时切换 框选状态
    // 鼠标按下时的位置为 初始位置
    if (event->button() == Qt::LeftButton) { // 只响应鼠标左键
        SelectionBox* clicked_box = nullptr;
        SelectionBox::HoverRegion hover_region = SelectionBox::HoverRegion::None;

        for (auto it = m_selection_boxes.constEnd(); it != m_selection_boxes.constBegin(); ) {
            --it;
            auto* box = it.value();
            QPoint box_pos = box->mapFromParent(event->pos());
            hover_region = box->get_hover_region(box_pos);
            if (hover_region != SelectionBox::HoverRegion::None) {
                clicked_box = box;
                break;
            }
        }
        
        if (clicked_box) {
            QPoint box_pos = clicked_box->mapFromParent(event->pos());
            QMouseEvent box_event(
                event->type(),
                box_pos,
                event->button(),
                event->buttons(),
                event->modifiers()
            );
            clicked_box->mousePressEvent(&box_event);
            event->accept();
        } else {
            m_is_selecting = true;
            m_start_point = event->pos();
            m_current_box_id = m_next_box_id++;
            auto* box = new SelectionBox{m_current_box_id, this};
            m_selection_boxes.insert(m_current_box_id, box);
            box->setGeometry(this->rect());
            box->set_selection_rect(QRect(m_start_point, QSize()));
            box->show();
            event->accept();
        }
    }
}

void RegionCropper::mouseMoveEvent(QMouseEvent* event) {
    emit mouse_moved(event->pos());
    
    using HoverRegion = SelectionBox::HoverRegion;
    SelectionBox* hover_box = nullptr;
    HoverRegion hover_region = HoverRegion::None;

    for (auto it = m_selection_boxes.constEnd(); it != m_selection_boxes.constBegin(); ) {
        --it;
        SelectionBox* box = it.value();
        QPoint box_pos = box->mapFromParent(event->pos());
        SelectionBox::HoverRegion region = box->get_hover_region(box_pos);
        if (region != SelectionBox::HoverRegion::None) {
            hover_box = box;
            hover_region = region;
            break;
        }
    }

    for (auto* box: m_selection_boxes) box->set_highlighted(box == hover_box);

    if (hover_box && hover_region != HoverRegion::None) {
         QCursor new_cursor = Qt::ArrowCursor;
            switch (hover_region) {
                case HoverRegion::Top:
                case HoverRegion::Bottom:       new_cursor = Qt::SizeVerCursor;   break;
                case HoverRegion::Left: 
                case HoverRegion::Right:        new_cursor = Qt::SizeHorCursor;   break;
                case HoverRegion::TopLeft:
                case HoverRegion::BottomRight:  new_cursor = Qt::SizeFDiagCursor; break;
                case HoverRegion::TopRight:
                case HoverRegion::BottomLeft:   new_cursor = Qt::SizeBDiagCursor; break;
                case HoverRegion::Body:         new_cursor = Qt::SizeAllCursor;   break;
                default: break;
            }
            setCursor(new_cursor);
    } else setCursor(Qt::ArrowCursor);


    if (m_is_selecting) { // 鼠标在 框选状态激活时 移动会进行
        if (auto* box = m_selection_boxes.value(m_current_box_id, nullptr))
            box->set_selection_rect(QRect(m_start_point, event->pos()).normalized());
        event->accept();
    } else if (hover_box) {
            QPoint box_pos = hover_box->mapFromParent(event->pos());
            QMouseEvent box_event(
                event->type(),
                box_pos,
                event->button(),
                event->buttons(),
                event->modifiers()
            );
            hover_box->mouseMoveEvent(&box_event);
            event->accept();
    } else {
        event->accept();
    }
}

void RegionCropper::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) { // 鼠标释放时需为左键
        if (m_is_selecting) {                    // 且是处在框选状态的
            m_is_selecting = false;
            if (auto* box = m_selection_boxes.value(m_current_box_id, nullptr)) {
                auto rect = box->get_selection_rect();
                if (rect.width()  > 0 &&
                    rect.height() > 0
                ) emit region_selected(box->geometry());
                else remove_selection_box(m_current_box_id);
            }
            event->accept();
        } else {
            for (auto it = m_selection_boxes.constEnd(); it != m_selection_boxes.constBegin(); ) {
                --it;
                SelectionBox* box = it.value();
                QPoint box_pos = box->mapFromParent(event->pos());
                if (box->get_hover_region(event->pos()) != SelectionBox::HoverRegion::None) {
                    QMouseEvent box_event(
                        event->type(),
                        box_pos,
                        event->button(),
                        event->buttons(),
                        event->modifiers()
                    );
                    box->mouseReleaseEvent(&box_event);
                }
            }
            event->accept();
        }
    } else event->ignore();
}

void RegionCropper::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    for (auto* box: m_selection_boxes) 
        box->setGeometry(this->rect());
}

void RegionCropper::remove_selection_box(int id) {
    if (auto* box = m_selection_boxes.take(id)) delete box;
}

RegionCropper::RegionCropper(QWidget* parent)
    : QWidget{parent}
    , m_is_selecting{false}
    , m_current_box_id{-1}
    , m_next_box_id(0)
    {
    setMouseTracking(true);

}

RegionCropper::~RegionCropper() = default;








