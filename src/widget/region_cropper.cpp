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


auto RegionCropper::get_selection_rect() const -> const QRect& {
    return m_selection_rect;
}

void RegionCropper::mousePressEvent(QMouseEvent* event) {
    // 鼠标按下时切换 框选状态
    // 鼠标按下时的位置为 初始位置
    if (event->button() == Qt::LeftButton) { // 只响应鼠标左键
        m_is_selecting = true;
        m_start_point = event->pos();
        m_selection_rect = QRect(m_start_point, QSize());
        update();
    }
}

void RegionCropper::mouseMoveEvent(QMouseEvent* event) {
    
    if (m_is_selecting) { // 鼠标在 框选状态激活时 移动会进行框选
        QPoint current_point = event->pos();
        m_selection_rect = QRect(m_start_point, current_point).normalized();
        update();
    }
}

void RegionCropper::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && // 鼠标释放时需为左键
        m_is_selecting) {                    // 且是处在框选状态的
        m_is_selecting = false;
        
        if (m_selection_rect.width() > 0 && m_selection_rect.height() > 0)
            qDebug() << "Cropped rect" << '\n'
                     << "height: " << m_selection_rect.height()
                     << "width:  " << m_selection_rect.width()
                     << '\n';

        // m_selection_rect = QRect();
        // update();
    }
}

void RegionCropper::paintEvent(QPaintEvent* /*event*/) {
    if (!m_is_selecting            || // 未进行选择 
         m_selection_rect.isNull() || // 选框无效
        !m_selection_rect.isValid()   // 选框无效
       ) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 框选框的边界
    QPen pen(QColor(0, 120, 215));
    pen.setWidth(2);
    painter.setPen(pen); 

    // 框选框的填充
    QBrush brush(QColor(0, 120, 215, 70));
    painter.setBrush(brush);
    painter.drawRect(m_selection_rect);
}





