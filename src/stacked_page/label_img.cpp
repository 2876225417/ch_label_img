#include "stacked_page/base_page.h"
#include "widget/labeling_canvas.h"
#include "widget/region_cropper.h"
#include <stacked_page/label_img.h>
#include <qboxlayout.h>
#include <qcolor.h>
#include <qpixmap.h>
#include <qwidget.h>
#include <QLabel>

void LabelImagePage::set_layout() {
    auto layout = new QVBoxLayout{this};
    layout->setContentsMargins(0, 0, 0, 0);


    m_canvas = new LabelingCanvas{this};
   
    QPixmap pixmap(800, 600);
    pixmap.fill(QColorConstants::Svg::lightgray);
    
    m_canvas->set_pixmap(pixmap);
        
    layout->addWidget(m_canvas);
}

void LabelImagePage::set_connections() {

}

LabelImagePage::LabelImagePage(QWidget* parent)
    : BasePage{parent}
    {
    set_layout();
    set_connections();
}

LabelImagePage::~LabelImagePage() = default;