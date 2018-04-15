#include "savestatelistview.h"

#include <QEvent>
#include <QToolTip>
#include <QHelpEvent>
#include <QDebug>

SaveStateListView::SaveStateListView(QWidget *parent) : QListView(parent)
{

}

void    SaveStateListView::setHandleStuff(HandleStuff *hs)
{
    m_handleStuff = hs;
}

bool SaveStateListView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::ToolTip)
    {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
        QModelIndex index = indexAt(helpEvent->pos());
        if (index.isValid())
        {
            QAbstractItemModel* md = model();
            QStandardItemModel* model = static_cast<QStandardItemModel*>(md);
            QStandardItem* item = model->itemFromIndex(index);
            QString path = m_handleStuff->getSavestatePath(item->text());
            if (m_handleStuff->hasScreenshots())
            {
                QString scPath = m_handleStuff->getScreenshotPath(item->text());
                if (!scPath.isEmpty())
                    QToolTip::showText(helpEvent->globalPos(), QString("%1<br/><img src='%2'>").arg(path).arg(scPath), this, QRect());
                else
                    QToolTip::showText(helpEvent->globalPos(), QString("%1").arg(path), this, QRect());
            } else {
                QToolTip::showText(helpEvent->globalPos(), QString("%1").arg(path), this, QRect());
            }
        } else {
                QToolTip::hideText();
                event->ignore();
        }
        return true;
    }
    return QListView::viewportEvent(event);
}
