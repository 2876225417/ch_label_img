#ifndef SIDE_MENU_BAR
#define SIDE_MENU_BAR

#include "utils/non-copyable.h"
#include <cstdint>
#include <qtmetamacros.h>
#include <qwidget.h>
class SideMenuBar: public QWidget, private NonCopyable {
    Q_OBJECT
public:
    explicit SideMenuBar(QWidget* parent = nullptr);
    ~SideMenuBar() override;

    enum class PageId: std::int8_t {
        Home,
        LabelImg,
        Settings
    };

signals:
    void page_changed(PageId page_id);

private:
    void set_layout();
};
#endif // SIDE_MENU_BAR