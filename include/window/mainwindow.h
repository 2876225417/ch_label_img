

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "utils/non-copyable.h"
#include "widget/side_menu_bar.h"
#include "widget/stacked_page_widget.h"
#include "widget/status_bar.h"
#include "widget/tool_bar.h"
#include <QMainWindow>
#include <qboxlayout.h>
#include <qmainwindow.h>
#include <qstackedwidget.h>
#include <qwidget.h>

class MainWindow: public QMainWindow, private NonCopyable {
    Q_OBJECT
public:
    explicit MainWindow(QMainWindow* parent = nullptr);
    ~MainWindow() override;

private:
    void set_layout();
    void set_connections();
    
    ToolBar* m_tool_bar;

    SideMenuBar* m_side_menu_bar;
    StackedPageWidget* m_stacked_page;

    StatusBar* m_status_bar;
};
#endif // MAINWINDOW_H