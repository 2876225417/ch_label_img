#include "widget/region_cropper.h"
#include <qboxlayout.h>
#include <qmainwindow.h>
#include <qwidget.h>
#include <qwindowdefs.h>
#include <window/mainwindow.h>
#include <QVBoxLayout>


MainWindow::MainWindow(QMainWindow* parent) 
    : QMainWindow{parent}
    {
    
    set_layout();
    set_connections();
    
    setWindowTitle("ppQwQqq");
    adjustSize();
}

MainWindow::~MainWindow() = default;


void MainWindow::set_layout() {
    auto* central_widget = new QWidget{this};
    setCentralWidget(central_widget);

    auto* main_layout = new QVBoxLayout{central_widget};

    auto* region_cropper = new RegionCropper();
    main_layout->addWidget(region_cropper);
}

void MainWindow::set_connections() {
    
}



