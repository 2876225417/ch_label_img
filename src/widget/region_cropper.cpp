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


RegionCropper::RegionCropper(QWidget* parent)
    : QWidget{parent}
    , m_is_selecting{false}
    {
    setAttribute(Qt::WA_TranslucentBackground);
    
    setFocusPolicy(Qt::StrongFocus);
}

RegionCropper::~RegionCropper() = default;


const QRect& RegionCropper::get_selection_rect() const {
    return m_selection_rect;
}

void RegionCropper::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_is_selecting = true;
        m_start_point = event->pos();
        m_selection_rect = QRect(m_start_point, QSize());
        update();
    }
}

void RegionCropper::mouseMoveEvent(QMouseEvent* event) {
    if (m_is_selecting) {
        QPoint current_point = event->pos();
        m_selection_rect = QRect(m_start_point, current_point).normalized();
        update();
    }
}

void RegionCropper::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_is_selecting) {
        m_is_selecting = false;
        
        if (m_selection_rect.width() > 0 && m_selection_rect.height() > 0)
            qDebug() << "Cropped rect" << '\n'
                     << "height: " << m_selection_rect.height()
                     << "width:  " << m_selection_rect.width()
                     << '\n';

        // m_selection_rect = QRect();
        update();
    }
}

void RegionCropper::paintEvent(QPaintEvent* event) {
    if (!m_is_selecting            || // 未进行选择 
         m_selection_rect.isNull() || // 选框无效
        !m_selection_rect.isValid()   // 选框无效
       ) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(QColor(0, 120, 215));
    pen.setWidth(2);
    painter.setPen(pen);

    QBrush brush(QColor(0, 120, 215, 70));
    painter.setBrush(brush);
    painter.drawRect(m_selection_rect);
}





