#include "page/base_page.h"
#include "widget/region_cropper.h"
#include <page/label_img.h>
#include <qboxlayout.h>
#include <qcolor.h>
#include <qpixmap.h>
#include <qwidget.h>
#include <QLabel>

void LabelImagePage::set_layout() {
    auto layout = new QVBoxLayout{this};
    layout->setContentsMargins(0, 0, 0, 0);

    auto image_label = new QLabel{this};
    QPixmap pixmap(800, 600);
    pixmap.fill(QColorConstants::Svg::lightgray);
    image_label->setPixmap(pixmap);
    image_label->setMinimumSize(1, 1);

    auto cropper = new RegionCropper(image_label);
    
    layout->addWidget(image_label);
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