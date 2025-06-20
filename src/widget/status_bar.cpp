#include <qstatusbar.h>
#include <qwidget.h>
#include <widget/status_bar.h>

void StatusBar::set_layout() {
    showMessage("准备就绪", 3000);
    
}

void StatusBar::set_connections() {

}

StatusBar::StatusBar(QWidget* parent)
    : QStatusBar(parent)
    {
    set_layout();
    set_connections();
}

StatusBar::~StatusBar() = default;

