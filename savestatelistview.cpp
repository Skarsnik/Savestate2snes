#include "savestatelistview.h"

#include <QEvent>
#include <QToolTip>
#include <QHelpEvent>

SaveStateListView::SaveStateListView(QWidget *parent) : QListView(parent)
{

}

SaveStateListView::setHandleStuff(HandleStuff *hs)
{
    m_handleStuff = hs;
}

bool SaveStateListView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::ToolTip)
    {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
        QModelIndex index = indexAt(helpEvent->pos());
        if (index.isValid() && m_handleStuff->hasScreenshots())
        {
            QAbstractItemModel* md = model();
            QStandardItemModel* model = static_cast<QStandardItemModel*>(md);
            QString scPath = m_handleStuff->getScreenshotPath(model->itemFromIndex(index)->text());
            if (!scPath.isEmpty())
                QToolTip::showText(helpEvent->globalPos(), QString("<img src='%1'>").arg(scPath), this, QRect());
        } else {
            QToolTip::hideText();
            event->ignore();
        }
        return true;
    }
    return QListView::viewportEvent(event);
}
