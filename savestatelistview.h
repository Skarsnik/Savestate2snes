#ifndef SAVESTATELISTVIEW_H
#define SAVESTATELISTVIEW_H

#include <QObject>
#include <QWidget>
#include <QListView>
#include "handlestuff.h"

class SaveStateListView : public QListView
{
public:
    explicit SaveStateListView(QWidget *parent = 0);
    setHandleStuff(HandleStuff* hs);
protected:
    bool    viewportEvent(QEvent *event);

private:
    HandleStuff*    m_handleStuff;
};

#endif // SAVESTATELISTVIEW_H
