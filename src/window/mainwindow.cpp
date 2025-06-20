#include "page/base_page.h"
#include "page/home.h"
#include "page/label_img.h"
#include "widget/region_cropper.h"
#include "widget/side_menu_bar.h"
#include "widget/stacked_page_widget.h"
#include "widget/status_bar.h"
#include "widget/tool_bar.h"
#include <functional>
#include <qboxlayout.h>
#include <qlogging.h>
#include <qmainwindow.h>
#include <qwidget.h>
#include <qwindowdefs.h>
#include <window/mainwindow.h>
#include <QVBoxLayout>
#include <QStackedWidget>


void MainWindow::set_layout() {
    m_tool_bar = new ToolBar{this};
    addToolBar(m_tool_bar);

    auto create_central_widget = [this]() -> void {
        auto* central_widget = new QWidget;
        setCentralWidget(central_widget);

        auto* main_layout = new QHBoxLayout(central_widget);
        main_layout->setSpacing(0);
        main_layout->setContentsMargins(0, 0, 0, 0);

        m_side_menu_bar = new SideMenuBar(this);
        main_layout->addWidget(m_side_menu_bar);

        m_stacked_page = new StackedPageWidget{this};
        main_layout->addWidget(m_stacked_page);
    };
    create_central_widget();

    m_status_bar = new StatusBar(this);
    setStatusBar(m_status_bar);    
}

void MainWindow::set_connections() {
    connect(m_side_menu_bar, &SideMenuBar::page_changed, this, [this](StackedPageWidget::PageId id) {
        m_stacked_page->setCurrentIndex(static_cast<int>(id));
    });

}



MainWindow::MainWindow(QMainWindow* parent) 
    : QMainWindow{parent}
    {
    
    set_layout();
    set_connections();
    
    setWindowTitle("ppQwQqq");
    resize(1200, 800);
}

MainWindow::~MainWindow() = default;
