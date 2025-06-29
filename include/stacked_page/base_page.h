#ifndef BASE_PAGE_H
#define BASE_PAGE_H

#include "stacked_page_pch.h"

class BasePage: public QWidget, private NonCopyable {
    Q_OBJECT
public:
    explicit BasePage(QWidget* parent = nullptr): QWidget{parent} { }
    ~BasePage() override = default;

    virtual void set_layout() = 0;
    virtual void set_connections() = 0;
};
#endif // BASE_PAGE_H