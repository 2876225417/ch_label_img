#include "utils/action_types.h"
#include "widget/selection_box.h"
#include <qapplication.h>
#include <qcolor.h>
#include <qevent.h>
#include <qkeysequence.h>
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

#include <qtils/action_manager.h>


void RegionCropper::setup_actions() {
    auto& am = ActionManager::instance();

    am.create_action(AnnotationAction::DeleteBox, tr("Delete Selection Box"), QKeySequence(Qt::Key_Delete), tr("Delete the selected annotation box"));

    am.register_handler(AnnotationAction::DeleteBox, [this]() {
        qDebug() << "Delete box (box id: " << m_selected_box_id << ")"; 
        if (remove_selection_box()) { 
            qDebug() << "Remove selected box with id: " << m_selected_box_id;
            m_selected_box_id = -1; // 成功删除后重置 id
        }
    });

    for (auto* action: am.actions_for_category(ActionCategory::Annotation))
        addAction(action);

}

void RegionCropper::mousePressEvent(QMouseEvent* event) {
    // 鼠标按下时切换 框选状态
    // 鼠标按下时的位置为 初始位置
    if (event->button() == Qt::LeftButton) { // 只响应鼠标左键
        auto [clicked_box, hover_region] = find_hit_box(event->pos(), MapFrom::Parent);
        
        if (clicked_box) {
            // 重置所有选框的高亮状态
            for (auto* box: m_selection_boxes) 
                box->set_highlighted_status(false, SelectionBox::HighlightedType::Selected);
            
            m_selected_box_id = clicked_box->id();
            // 高亮选中的选框
            clicked_box->set_highlighted_status(true, SelectionBox::HighlightedType::Selected);
            
            auto box_event = create_box_mouse_event(event, clicked_box);

            clicked_box->mousePressEvent(&box_event);
            event->accept();
        } else {
            for (auto* box: m_selection_boxes) box->set_highlighted_status(false, SelectionBox::HighlightedType::Selected);
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
    
    auto [hover_box, hover_region] 
        = find_hit_box(event->pos(), MapFrom::Parent);
    if (hover_box != nullptr)  hover_box->raise();

    for (auto* box: m_selection_boxes) 
        box->set_highlighted_status(box == hover_box, SelectionBox::HighlightedType::Hovered);

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


    if (m_is_selecting)   { // 鼠标在 框选状态激活时 移动会进行
        if (auto* box = m_selection_boxes.value(m_current_box_id, nullptr))
            box->set_selection_rect(QRect(m_start_point, event->pos()).normalized());
        event->accept();
    } else if (hover_box) { // 如果不是框选状态，且 hover 了一个选框
            auto box_event = create_box_mouse_event(event, hover_box);

            hover_box->mouseMoveEvent(&box_event);
            event->accept();
    } else {
        event->accept();
    }
}

void RegionCropper::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) { // 鼠标释放时需为左键
        if (m_is_selecting) {                // 且是处在框选状态的
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

                if (box->get_hover_region(event->pos()) != SelectionBox::HoverRegion::None) {
                    auto box_event = create_box_mouse_event(event, box);
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

bool RegionCropper::remove_selection_box(int id) {
    if (auto* box = m_selection_boxes.take(id)) { delete box; return true; }
    
    return false;
}


// -------- Region Cropper Helper Functions -------- //
auto RegionCropper::find_hit_box(const QPoint& pos, MapFrom type)
    // 这个 hover 选框的实现有点小问题
    // 因为 for 是从最近创建的选框往前进行遍历的
    // 导致当有两个选框重叠时，尽管之前创建的选框在最新创建的选框之上
    // 最终 hover 的选框也是最新的那个一个选框 
    const -> BoxHitInfo {
    for ( auto it = m_selection_boxes.constEnd()
        ; it != m_selection_boxes.constBegin()
        ; /* No Steps */ ) {
        --it;
        auto* box = it.value();
        QPoint map_pos = {};
        if (type == MapFrom::Global)     map_pos = box->mapFromGlobal(pos); 
        else if(type == MapFrom::Parent) map_pos = box->mapFromParent(pos);

        auto region = box->get_hover_region(map_pos);
        if (region != SelectionBox::HoverRegion::None) 
            return {.box = box, .region = region};
    }
    return {};
}  

auto RegionCropper::remove_selection_box() -> bool {
    if (m_selected_box_id == -1) { qDebug() << "No selected box"; return false; }
    if (auto* box = m_selection_boxes.take(m_selected_box_id)) {
        delete box;
        return true; 
    }
    return false;
}

auto RegionCropper::RegionCropper::create_box_mouse_event(QMouseEvent* event, SelectionBox* box) 
    -> QMouseEvent {
    QPointF box_pos = box->mapFromParent(event->pos());
    QPointF global_pos = box->mapFromGlobal(box_pos);
    return {
        event->type(),
        box_pos,
        global_pos,
        event->button(),
        event->buttons(),
        event->modifiers(),
        event->pointingDevice()
    };
}

// ------------------------------------------------- //


RegionCropper::RegionCropper(QWidget* parent)
    : QWidget{parent}
    , m_is_selecting{false}
    , m_current_box_id{-1}
    , m_next_box_id(0)
    , m_selected_box_id{-1}
    {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setup_actions();
}

RegionCropper::~RegionCropper() = default;