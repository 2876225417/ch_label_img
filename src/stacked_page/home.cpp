#include "stacked_page/base_page.h"
#include <stacked_page/home.h>
#include <qboxlayout.h>
#include <qnamespace.h>
#include <qwidget.h>
#include <QLabel>

void HomePage::set_layout() {
    auto layout = new QVBoxLayout{this};
    auto label = new QLabel{"欢迎使用!\n这里是主页。", this};
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 24px; color: #333;");
    layout->addWidget(label);
}

void HomePage::set_connections() {

}


HomePage::HomePage(QWidget* parent)
    : BasePage{parent}
    {
    set_layout();
    set_connections();
}

HomePage::~HomePage() = default;