#ifndef STACKED_PAGE_WIDGET_H
#define STACKED_PAGE_WIDGET_H


#include "stacked_page/base_page.h"
#include "utils/non-copyable.h"
#include "widget/side_menu_bar.h"
#include <qstackedwidget.h>
#include <qtmetamacros.h>
#include <qwidget.h>
class StackedPageWidget: public QStackedWidget, private NonCopyable {
    Q_OBJECT
public:
    explicit StackedPageWidget(QWidget* parent = nullptr);
    ~StackedPageWidget() override;

    using PageId = SideMenuBar::PageId;
    [[nodiscard]] auto get_page(PageId id) const -> BasePage*;

public slots:
    void set_current_page(PageId id);

private:
    void create_pages();

    QMap<PageId, BasePage*> m_pages;
};
#endif // STACKED_PAGE_WIDGET_H