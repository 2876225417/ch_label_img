#include <qboxlayout.h>
#include <qbuttongroup.h>
#include <qnamespace.h>
#include <qsizepolicy.h>
#include <qwidget.h>
#include <widget/side_menu_bar.h>
#include <QButtonGroup>
#include <QStyle>
#include <QToolButton>

void SideMenuBar::set_layout() {
    auto main_side_menu_bar_layout = new QVBoxLayout{this};
    main_side_menu_bar_layout->setContentsMargins(5, 10, 5, 10);
    main_side_menu_bar_layout->setSpacing(5);

    auto button_group = new  QButtonGroup{this};
    button_group->setExclusive(true);

    auto create_buttons = [this, &main_side_menu_bar_layout, &button_group]() -> void {
        struct ButtonInfo {
            PageId                 id;
            QStyle::StandardPixmap icon;
            QString                text;
        };

        const QList<ButtonInfo> buttons_infos = {
        {.id=PageId::Home, .icon=QStyle::SP_ComputerIcon, .text="主页"},
        {.id=PageId::LabelImg, .icon=QStyle::SP_FileIcon, .text="图像标注"},
        {.id=PageId::Settings, .icon=QStyle::SP_BrowserReload, .text="设置"},
        };

        for (const auto& info: buttons_infos) {
            auto button = new QToolButton{this};
            button->setIcon(this->style()->standardIcon(info.icon));
            button->setText(info.text);
            button->setCheckable(true);
            button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            button->setMinimumHeight(40);

            button->setStyleSheet(R"(
                QToolButton {
                    color: white;
                    background-color: transparent;
                    border: none;
                    padding: 10px;
                    text-align: left;
                }
                QToolButton:hover {
                    background-color: #34495e;
                }
                QToolButton:checked {
                    background-color: #1abc9c;
                }
            )");

            layout()->addWidget(button);
            button_group->addButton(button, static_cast<int>(info.id));
        }

        if (button_group->button(static_cast<int>(PageId::Home)))
            button_group->button(static_cast<int>(PageId::Home))->setChecked(true);
    
        connect(button_group, &QButtonGroup::idClicked, this, [this](int id) {
            emit page_changed(static_cast<PageId>(id));
        });
    };

    create_buttons();
    
    main_side_menu_bar_layout->addStretch();


}


SideMenuBar::SideMenuBar(QWidget* parent)
    : QWidget{parent}
    {
    setFixedWidth(180);
    setStyleSheet("background-color: #2c3e50;");

    set_layout();
}


SideMenuBar::~SideMenuBar() = default;