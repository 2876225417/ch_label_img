#ifndef HOME_H
#define HOME_H

#include "page/base_page.h"
#include <qtmetamacros.h>
#include <qwidget.h>
class HomePage: public BasePage {
    Q_OBJECT
public:
    explicit HomePage(QWidget* parent = nullptr);
    ~HomePage() override;
private:
    void set_layout() override;
    void set_connections() override;
};



#endif // HOME_H