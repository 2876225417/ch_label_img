#ifndef ACTION_MANAGER_H
#define ACTION_MANAGER_H

#include "qtils_pch.h"
#include "utils/action_types.h"

class ActionManager: public QObject, public Singleton<ActionManager> {
    Q_OBJECT
    MAKE_SINGLETON(ActionManager)
public:
    template <typename ActionEnum>
    auto create_action( ActionEnum id
                      , const QString& text
                      , const QKeySequence& shortcut = QKeySequence()
                      , const QString& tooltip = QString()
                      ) -> QAction* {
        return create_action(static_cast<ActionId>(id), text, shortcut, tooltip);
    }
    
    template <typename ActionEnum>
    [[nodiscard]] auto action(ActionEnum id) const 
    -> QAction* { return action(static_cast<ActionId>(id)); }

    template <typename ActionEnum>
    void register_handler(ActionEnum id, std::function<void()> handler) {
        return register_handler(static_cast<ActionId>(id), std::move(handler));
    }

    // 批量注册 Actions
    template <typename... ActionEnum>
    void register_actions(QWidget* parent) {
        (register_actions_for_category<ActionEnum>(parent), ...);
    }

    // 禁用 / 启用 某个类别的所有 Actions
    void set_category_enabled(ActionCategory category, bool enabled);

    // 获取某个类别的所有 Actions
    [[nodiscard]] auto actions_for_category(ActionCategory category)
    const -> QList<QAction*> ;

signals:
    void action_triggered(ActionId id);

private:
    auto create_action( ActionId id
                      , const QString& text
                      , const QKeySequence& shortcut
                      , const QString& tooltip
                      ) -> QAction* ;
    
    [[nodiscard]] auto action(ActionId id)
    const -> QAction* ;

    void register_handler(ActionId id, std::function<void()> handler);

    template <typename ActionEnum>
    void register_actions_for_category(QWidget* parent);
private:
    QMap<ActionId, QAction*> m_actions;
    QMap<ActionId, std::function<void()>> m_handlers;
};

#endif // ACTION_MANAGER_H