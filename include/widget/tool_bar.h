#ifndef TOOL_BAR_H
#define TOOL_BAR_H

#include <cstdint>
#include <qaction.h>
#include <qmainwindow.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <utils/non-copyable.h>
#include <QToolBar>

class ToolBar: public QToolBar, private NonCopyable{
    Q_OBJECT
public:
    explicit ToolBar(QWidget* parent = nullptr);
    ~ToolBar() override;

    enum class ActionId: std::int8_t{
        Open,
        Save,
        ZoomIn,
        ZoomOut,
    };

    [[nodiscard]] auto get_action(ActionId id) const -> QAction*;

private:
    void create_actions();

    QMap<ActionId, QAction*> m_actions;
};

#endif // TOOL_BAR_H