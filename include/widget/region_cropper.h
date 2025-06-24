#ifndef REGION_CROPPER_H
#define REGION_CROPPER_H

#include "utils/method_concepts.h"
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

signals:
    void region_selected(const QRect& region);
    void mouse_moved(const QPoint& pos);

public slots:
    auto remove_selection_box(int id) -> bool;
protected:
    void mousePressEvent(QMouseEvent*)   override;
    void mouseMoveEvent(QMouseEvent*)    override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*)      override;
private:
    auto   remove_selection_box() -> bool;
    bool   m_is_selecting;    // 记录当前框选的状态(状态：正在框选/未框选)
    QPoint m_start_point;     // 框选的起始位置
    int    m_current_box_id;  // 当前框选框的 id
    int    m_next_box_id;     // 下一个框选框的 id
    int    m_selected_box_id; // 当前选中的选框 id

    QMap<int, SelectionBox*> m_selection_boxes;

    void setup_actions();
    struct BoxHitInfo{
        SelectionBox* box = nullptr;
        SelectionBox::HoverRegion region = SelectionBox::HoverRegion::None;

        [[nodiscard]] constexpr operator bool() const noexcept {
            return box != nullptr;
        }
    };

    enum class MapFrom: std::uint8_t {
        Parent,
        Global,
        Local
    };

private:
    template <typename R = void> [[nodiscard]] auto
    find_hit_box(const QPoint& pos, MapFrom type) const -> BoxHitInfo
    requires EventHelper<RegionCropper> {
    // 这个 hover 选框的实现有点小问题
    // 因为 for 是从最近创建的选框往前进行遍历的
    // 导致当有两个选框重叠时，尽管之前创建的选框在最新创建的选框之上
    // 最终 hover 的选框也是最新的那个一个选框 
        for ( auto it = m_selection_boxes.constEnd()
            ; it != m_selection_boxes.constBegin()
            ; /* No Steps */ ) {
            --it;
            auto* box = it.value();
            QPoint map_pos = [&] () {
                switch (type) {
                    case MapFrom::Parent: return box->mapFromParent(pos);
                    case MapFrom::Global: return box->mapFromGlobal(pos);
                    case MapFrom::Local:  return pos;
                }
                return pos;
            }();
            
            if (type == MapFrom::Global)     map_pos = box->mapFromGlobal(pos); 
            else if(type == MapFrom::Parent) map_pos = box->mapFromParent(pos);

            auto region = box->get_hover_region(map_pos);
            if (region != SelectionBox::HoverRegion::None) 
                return {.box = box, .region = region};
        }
        return {};
    }
    
    template <typename R = void> auto 
    create_box_mouse_event(QMouseEvent* event, SelectionBox* box) -> QMouseEvent
    requires EventHelper<RegionCropper> {
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
};

#endif // REGION_CROPPER_H