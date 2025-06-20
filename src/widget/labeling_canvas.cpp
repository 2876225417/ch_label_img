#include "widget/labeling_canvas.h"
#include "widget/image_viewer.h"
#include "widget/region_cropper.h"
#include <qboxlayout.h>
#include <qevent.h>
#include <qpixmap.h>
#include <qwidget.h>
#include <widget/labeling_canvas.h>


void LabelingCanvas::set_pixmap(const QPixmap& pixmap) {
    m_image_viewer->set_pixmap(pixmap);
}

void LabelingCanvas::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    // 使上层的选择控件和下层的展示控件大小相同
    m_region_cropper->setGeometry(this->rect());
}

LabelingCanvas::LabelingCanvas(QWidget* parent)
    : QWidget{parent}
    {
    auto* layout = new QVBoxLayout{this};
    layout->setContentsMargins(0, 0, 0, 0);
    
    m_image_viewer = new ImageViewer{this};
    layout->addWidget(m_image_viewer);

    m_region_cropper = new RegionCropper{this};

}
LabelingCanvas::~LabelingCanvas() = default;