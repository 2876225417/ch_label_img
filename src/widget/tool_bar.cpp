#include <QStyle>
#include <cctype>
#include <qaction.h>
#include <qkeysequence.h>
#include <qstyle.h>
#include <qwidget.h>
#include <widget/tool_bar.h>

void ToolBar::create_actions() {
    struct ActionInfo {
        ActionId               id;          // 动作 id
        QStyle::StandardPixmap icon;        // 动作 图标
        QString                text;        // 动作 文本内容
        QString                status_tip;  // 动作 提示
        QKeySequence           shortcut;    // 动作 快捷键
    };

    const QList<ActionInfo>action_infos = {
        {.id=ActionId::Open, .icon=QStyle::SP_DirOpenIcon,      .text="打开(&O)", .status_tip="打开一个图像文件", .shortcut=QKeySequence::Open},
        {.id=ActionId::Save, .icon=QStyle::SP_DialogSaveButton, .text="保存(&S)", .status_tip="保存标注结果",    .shortcut=QKeySequence::Save},
    };

    for (const auto& info: action_infos) {
        auto icon = this->style()->standardIcon(info.icon);
        auto action = new QAction{icon, info.text, this};
        action->setStatusTip(info.status_tip);
        action->setShortcut(info.shortcut);

        addAction(action);
        m_actions[info.id] = action;
    }
}

auto ToolBar::get_action(ActionId id) const -> QAction* {
    return m_actions.value(id, nullptr);
}

ToolBar::ToolBar(QWidget* parent)
    : QToolBar{"主工具栏", parent}
    {
    setMovable(false);
    
    create_actions();
}

ToolBar::~ToolBar() = default;


