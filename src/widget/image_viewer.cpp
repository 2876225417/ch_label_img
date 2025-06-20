#include <qevent.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qwidget.h>
#include <widget/image_viewer.h>

void ImageViewer::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);

    if (m_pixmap.isNull()) return;

    QPainter painter{this};
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPixmap scaled_pixmap = m_pixmap.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    // 图像居中显示
    int x = (this->width() - scaled_pixmap.width()) / 2;
    int y = (this->height() - scaled_pixmap.height()) / 2;

    painter.drawPixmap(x, y, scaled_pixmap);
}

void ImageViewer::set_pixmap(const QPixmap& pixmap) {
    m_pixmap = pixmap;
    update();
}

ImageViewer::ImageViewer(QWidget* parent)
    : QWidget{parent}
    {
    // 默认背景色为深灰色
    setStyleSheet("background-color: #333;");
}

ImageViewer::~ImageViewer() = default;