#include <cstdlib>
#include <qbrush.h>
#include <qcolor.h>
#include <qcursor.h>
#include <qevent.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpoint.h>
#include <qvariant.h>
#include <qwidget.h>
#include <widget/selection_box.h>

constexpr int HANDLE_SIZE = 8;
constexpr int BORDER_WIDTH = 5;

auto SelectionBox::get_selection_rect() const -> const QRect& {
    return m_selection_rect;
}

void SelectionBox::set_highlighted(bool highlighted) {
    if (m_is_highlighted != highlighted) {
        m_is_highlighted = highlighted;
        update();
    }
}

void SelectionBox::paintEvent(QPaintEvent* /*event*/) {
    if (m_selection_rect.isNull() ||
       !m_selection_rect.isValid()
       ) return;
    
    QPainter painter{this};
    painter.setRenderHint(QPainter::Antialiasing);

    // 选框边框
    QPen pen(QColor(0, 120, 215, 200), 2, Qt::SolidLine);
    painter.setPen(pen);
    // 选框填充
    if (m_is_highlighted) painter.setBrush(QBrush(QColor(0, 120, 215, 130)));
    else                  painter.setBrush(QBrush(QColor(0, 120, 215, 70)));
    painter.drawRect(m_selection_rect);

    // 边框 8 个点
    painter.setBrush(Qt::white);
    pen.setColor(Qt::black);
    pen.setWidth(1);
    painter.setPen(pen);

    painter.drawRect(m_selection_rect.x() - HANDLE_SIZE / 2, m_selection_rect.y() - HANDLE_SIZE / 2, HANDLE_SIZE, HANDLE_SIZE);              // TopLeft
    painter.drawRect(m_selection_rect.right() - HANDLE_SIZE / 2, m_selection_rect.y() - HANDLE_SIZE / 2, HANDLE_SIZE, HANDLE_SIZE);          // TopRight
    painter.drawRect(m_selection_rect.x() - HANDLE_SIZE / 2, m_selection_rect.bottom() - HANDLE_SIZE / 2, HANDLE_SIZE, HANDLE_SIZE);         // BottomLeft
    painter.drawRect(m_selection_rect.right() - HANDLE_SIZE / 2, m_selection_rect.bottom() - HANDLE_SIZE / 2, HANDLE_SIZE, HANDLE_SIZE);     // BottomRight
    painter.drawRect(m_selection_rect.center().x() - HANDLE_SIZE / 2, m_selection_rect.y() - HANDLE_SIZE / 2, HANDLE_SIZE, HANDLE_SIZE);     // Top
    painter.drawRect(m_selection_rect.center().x() - HANDLE_SIZE / 2, m_selection_rect.bottom() - HANDLE_SIZE / 2, HANDLE_SIZE, HANDLE_SIZE);// Bottom
    painter.drawRect(m_selection_rect.x() - HANDLE_SIZE / 2, m_selection_rect.center().y() - HANDLE_SIZE / 2, HANDLE_SIZE, HANDLE_SIZE);     // Left
    painter.drawRect(m_selection_rect.right() - HANDLE_SIZE / 2, m_selection_rect.center().y() - HANDLE_SIZE / 2, HANDLE_SIZE, HANDLE_SIZE); // Right
}


void SelectionBox::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_hover_region = get_hover_region(event->pos());
        if (m_hover_region != HoverRegion::None) {
            m_is_interacting = true;
            m_drag_start_pos = event->pos();
            m_original_rect = m_selection_rect;
            qDebug() << "Selection box with id: " << m_id << "clicked";
            event->accept();
        } else event->ignore();
    } else event->ignore();
}

void SelectionBox::mouseMoveEvent(QMouseEvent* event) {
    if (m_is_interacting) {
        QRect new_rect = m_original_rect;
        QPoint delta = event->pos() - m_drag_start_pos;

        switch (m_hover_region) {
            case HoverRegion::Body:         new_rect.translate(delta); break;
            case HoverRegion::Top:          new_rect.setTop(new_rect.top() + delta.y());          break;
            case HoverRegion::Bottom:       new_rect.setBottom(new_rect.bottom() + delta.y());    break;
            case HoverRegion::Left:         new_rect.setLeft(new_rect.left() + delta.x());        break;
            case HoverRegion::Right:        new_rect.setRight(new_rect.right() + delta.x());      break;
            case HoverRegion::TopLeft:      new_rect.setTopLeft(new_rect.topLeft() + delta);        break;
            case HoverRegion::TopRight:     new_rect.setTopRight(new_rect.topRight() + delta);      break;
            case HoverRegion::BottomLeft:   new_rect.setBottomLeft(new_rect.bottomLeft() + delta);  break;
            case HoverRegion::BottomRight:  new_rect.setBottomRight(new_rect.bottomRight() + delta);break;
            default: break;
        }

        set_selection_rect(new_rect.normalized());
        emit rect_changed(m_id, m_selection_rect);
        event->accept();
    } else {
        update_cursor_shape(event->pos());
        event->ignore();
    }
}

void SelectionBox::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton &&
        m_is_interacting ) {
        m_is_interacting = false;
        emit editing_finished(m_id, m_selection_rect);
        event->accept(); 
    } else event->ignore();
}

void SelectionBox::update_cursor_shape(const QPoint& pos) {
    HoverRegion region = get_hover_region(pos);
    if (region != m_hover_region) {
        m_hover_region = region;
        if (m_is_interacting) {
            QCursor new_cursor = Qt::ArrowCursor;
            switch (m_hover_region) {
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
        }
    }
}

auto SelectionBox::get_hover_region(const QPoint& pos) const -> HoverRegion {
    if (!m_selection_rect.adjusted(-BORDER_WIDTH, -BORDER_WIDTH, BORDER_WIDTH, BORDER_WIDTH).contains(pos)) return HoverRegion::None;

    QRect inner_rect = m_selection_rect.adjusted(BORDER_WIDTH, BORDER_WIDTH, -BORDER_WIDTH, -BORDER_WIDTH);
    if (!inner_rect.contains(pos)) {
        if (abs(pos.y()- m_selection_rect.top()) < BORDER_WIDTH) {
            if (abs(pos.x() - m_selection_rect.left()) < BORDER_WIDTH) return HoverRegion::TopLeft;
            if (abs(pos.x() - m_selection_rect.right()) < BORDER_WIDTH)   return HoverRegion::TopRight;
            return HoverRegion::Top;
        }
        if (abs(pos.y() - m_selection_rect.bottom()) < BORDER_WIDTH) {
            if (abs(pos.x() - m_selection_rect.left()) < BORDER_WIDTH) return HoverRegion::BottomLeft;
            if (abs(pos.x() - m_selection_rect.right()) < BORDER_WIDTH) return HoverRegion::BottomRight;
            return HoverRegion::Bottom;
        }
        if (abs(pos.x() - m_selection_rect.left()) < BORDER_WIDTH) return HoverRegion::Left;
        if (abs(pos.x() - m_selection_rect.right()) < BORDER_WIDTH) return HoverRegion::Right;
    }

    if (m_selection_rect.contains(pos)) return HoverRegion::Body;
    return HoverRegion::None;
}

void SelectionBox::set_selection_rect(const QRect& rect) {
    if (m_selection_rect != rect) {
        m_selection_rect = rect;
        update();
    }
}

auto SelectionBox::id() const -> int { return m_id; }

SelectionBox::SelectionBox(int id, QWidget* parent)
    : QWidget{parent}
    , m_id{id}
    , m_is_highlighted{false}
    , m_is_interacting{false}
    , m_hover_region{HoverRegion::None}
    {
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
}

SelectionBox::~SelectionBox() = default;