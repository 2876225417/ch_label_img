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
    explicit SelectionBox(int id, QWidget* parent = nullptr);
    ~SelectionBox() override;
    
    enum class HoverRegion: std::int8_t {
        None, Body,
        Top, Bottom, Right, Left,
        TopLeft, TopRight, BottomLeft, BottomRight
    };

    enum class HighlightedType: std::uint8_t {
        Hovered,
        Selected,
    };
private:
    struct HighlightedStatus {
        bool is_hovered;
        bool is_selected;
    } m_higlighted_status;
public:
    [[nodiscard]] auto id() const -> int;
    [[nodiscard]] auto get_selection_rect() const -> const QRect&;
    [[nodiscard]] auto get_hover_region(const QPoint&) const -> HoverRegion;
signals:
    void rect_changed(int id, const QRect&);
    void editing_finished(int id, const QRect&);
public slots:
    void set_selection_rect(const QRect& rect);
    void set_highlighted_status(bool highlighted, HighlightedType);
public:
    void mousePressEvent(QMouseEvent*)   override;
    void mouseMoveEvent(QMouseEvent*)    override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*)        override;

private:
    const int m_id;

    bool m_is_interacting;
    QRect m_selection_rect;  
    QPoint m_drag_start_pos;
    QRect m_original_rect;
    HoverRegion m_hover_region;
    
    void update_cursor_shape(const QPoint&);

};
#endif // SELECTION_BOX_H