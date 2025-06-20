#ifndef SELECTION_BOX_H
#define SELECTION_BOX_H

#include "utils/non-copyable.h"
#include <qevent.h>
#include <qpoint.h>
#include <qtmetamacros.h>
#include <qvariant.h>
#include <qwidget.h>

class SelectionBox: public QWidget, private NonCopyable {
    Q_OBJECT
public:
    explicit SelectionBox(QWidget* parent = nullptr);
    ~SelectionBox() override;
signals:
    void rect_changed(const QRect&);
    void editing_finished(const QRect&);
public slots:
    void set_selection_rect(const QRect& rect);

protected:
    void mousePressEvent(QMouseEvent*)   override;
    void mouseMoveEvent(QMouseEvent*)    override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*)        override;

private:
    enum class HoverRegion: std::int8_t {
        None, Body,
        Top, Bottom, Right, Left,
        TopLeft, TopRight, BottomLeft, BottomRight
    };

    QRect m_selection_rect;  
    HoverRegion m_hover_region;

    void update_cursor_shape(const QPoint&);
    [[nodiscard]] auto get_hover_region(const QPoint&) const -> HoverRegion;
    bool m_is_interacting;
    QPoint m_drag_start_pos;
};
#endif // SELECTION_BOX_H