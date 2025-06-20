#include "page/base_page.h"
#include "page/home.h"
#include "page/label_img.h"
#include <functional>
#include <qstackedwidget.h>
#include <qwidget.h>
#include <widget/stacked_page_widget.h>






void StackedPageWidget::create_pages() {
    using PageFactory = std::function<BasePage*(QWidget*)>;
    
    struct PageInfo {
        PageId id;
        PageFactory factory;
    };

    const std::vector<PageInfo> page_infos = {
    {.id=PageId::Home, .factory=[](QWidget* parent) { return new HomePage{parent}; }},
    {.id=PageId::LabelImg, .factory=[](QWidget* parent) { return new LabelImagePage{parent}; }},
    };

    for (const auto& info: page_infos) {
        BasePage* page = info.factory(this);
        addWidget(page);
        m_pages[info.id] = page;
    }
}

auto StackedPageWidget::get_page(PageId id) const -> BasePage* {
    return m_pages.value(id, nullptr);
}

StackedPageWidget::StackedPageWidget(QWidget* parent)
    : QStackedWidget{parent}
    {
    create_pages();
};

StackedPageWidget::~StackedPageWidget() = default;