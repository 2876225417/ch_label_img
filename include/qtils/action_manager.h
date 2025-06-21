#ifndef ACTION_MANAGER_H
#define ACTION_MANAGER_H

#include "utils/singleton.h"
#include <QObject>
#include <qobject.h>
#include <qtmetamacros.h>


class ActionManager: public QObject, private Singleton<ActionManager> {
    Q_OBJECT
    MAKE_SINGLETON(ActionManager)
public:



private:

};



#endif // ACTION_MANAGER_H