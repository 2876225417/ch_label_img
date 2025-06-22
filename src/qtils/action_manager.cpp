#include "utils/action_types.h"
#include <qaction.h>
#include <qkeysequence.h>
#include <qtils/action_manager.h>

auto ActionManager::create_action( ActionId id
                                 , const QString& text
                                 , const QKeySequence& shortcut
                                 , const QString& tooltip
                                 ) -> QAction* {
    if (m_actions.contains(id)) return m_actions[id];

    auto* action = new QAction(text, this);

    if (!shortcut.isEmpty()) action->setShortcut(shortcut);

    if (!tooltip.isEmpty()) action->setToolTip(tooltip);

    connect(action, &QAction::triggered, this, [this, id]() {
       emit action_triggered(id);
       if (m_handlers.contains(id)) m_handlers[id](); 
    });

    m_actions[id] = action;
    return action;
}

auto ActionManager::action(ActionId id) 
    const -> QAction* {
    return m_actions.value(id, nullptr);
}

void ActionManager::register_handler(ActionId id, std::function<void()> handler) {
    m_handlers[id] = std::move(handler);
}

void ActionManager::set_category_enabled(ActionCategory category, bool enabled) {
    for (auto it = m_actions.begin(); it != m_actions.end(); ++it) {
        if (get_action_category(it.key()) == category) 
            it.value()->setEnabled(enabled);
    }
}

auto ActionManager::actions_for_category(ActionCategory category) 
    const -> QList<QAction*> {
    QList<QAction*> actions;
    for (auto it = m_actions.begin(); it != m_actions.end(); ++it) {
        if (get_action_category(it.key()) == category)
            actions.append(it.value());
    }
    return actions;
}