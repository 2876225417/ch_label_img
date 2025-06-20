#ifndef STATUS_BAR_H
#define STATUS_BAR_H


#include "utils/non-copyable.h"
#include <QStatusBar>
#include <qmainwindow.h>
#include <qtmetamacros.h>
#include <qwidget.h>

class StatusBar: public QStatusBar, private NonCopyable {
    Q_OBJECT
public:
    explicit StatusBar(QWidget* parent = nullptr);
    ~StatusBar() override;

private:
    void set_layout();
    void set_connections();
};
#endif // STATUS_BAR_H