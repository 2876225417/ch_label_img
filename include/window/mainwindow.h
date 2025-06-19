

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qmainwindow.h>


class MainWindow: public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QMainWindow* parent = nullptr);
    ~MainWindow() override;

private:
    void set_layout();
    void set_connections();
};

#endif // MAINWINDOW_H