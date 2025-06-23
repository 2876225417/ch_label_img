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

signals:
    void region_selected(const QRect& region);
    void mouse_moved(const QPoint& pos);

public slots:
    bool remove_selection_box(int id);
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


    struct BoxHitInfo {
        SelectionBox* box = nullptr;
        SelectionBox::HoverRegion region = SelectionBox::HoverRegion::None;
    };

    enum class MapFrom: std::uint8_t {
        Parent,
        Global,
    };

    [[nodiscard]] auto 
    find_hit_box(const QPoint&, MapFrom) const -> BoxHitInfo;

};

#endif // REGION_CROPPER_H