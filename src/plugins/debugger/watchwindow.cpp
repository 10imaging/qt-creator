/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact:  Qt Software Information (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
**************************************************************************/

#include "watchwindow.h"

#include "debuggeractions.h"

#include <utils/qtcassert.h>

#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include <QtGui/QAction>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QHeaderView>
#include <QtGui/QItemDelegate>
#include <QtGui/QLineEdit>
#include <QtGui/QMenu>
#include <QtGui/QResizeEvent>
#include <QtGui/QSplitter>

using namespace Debugger::Internal;


/////////////////////////////////////////////////////////////////////
//
// WatchDelegate
//
/////////////////////////////////////////////////////////////////////

enum { INameRole = Qt::UserRole, VisualRole, ExpandedRole };

class WatchDelegate : public QItemDelegate
{
public:
    WatchDelegate(QObject *parent) : QItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &,
        const QModelIndex &) const
    {
        return new QLineEdit(parent);
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
        QTC_ASSERT(lineEdit, return);
        lineEdit->setText(index.model()->data(index, Qt::EditRole).toString());
    }

    void setModelData(QWidget *editor, QAbstractItemModel *,
        const QModelIndex &index) const
    {
        QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
        QTC_ASSERT(lineEdit, return);
        QString value = lineEdit->text();
        QString exp = index.model()->data(index, Qt::EditRole).toString();
        if (index.column() == 1) {
            // the value column
            theDebuggerSetting(AssignValue)->trigger(exp + '=' + value);
        } else if (index.column() == 0) {
            // the watcher name column
            theDebuggerSetting(RemoveWatchExpression)->trigger(exp);
            theDebuggerSetting(WatchExpression)->trigger(lineEdit->text());
        }
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
        const QModelIndex &) const
    {
        editor->setGeometry(option.rect);
    }
};


/////////////////////////////////////////////////////////////////////
//
// WatchWindow
//
/////////////////////////////////////////////////////////////////////

WatchWindow::WatchWindow(Type type, QWidget *parent)
    : QTreeView(parent), m_alwaysResizeColumnsToContents(true), m_type(type)
{
    setWindowTitle(tr("Locals and Watchers"));
    setAlternatingRowColors(true);
    setIndentation(indentation() * 9/10);
    setUniformRowHeights(true);
    setItemDelegate(new WatchDelegate(this));

    connect(this, SIGNAL(expanded(QModelIndex)),
        this, SLOT(expandNode(QModelIndex)));
    connect(this, SIGNAL(collapsed(QModelIndex)),
        this, SLOT(collapseNode(QModelIndex)));
}

void WatchWindow::expandNode(const QModelIndex &idx)
{
    //QModelIndex mi0 = idx.sibling(idx.row(), 0);
    //QString iname = model()->data(mi0, INameRole).toString();
    //QString name = model()->data(mi0, Qt::DisplayRole).toString();
    emit requestExpandChildren(idx);
}

void WatchWindow::collapseNode(const QModelIndex &idx)
{
    //QModelIndex mi0 = idx.sibling(idx.row(), 0);
    //QString iname = model()->data(mi0, INameRole).toString();
    //QString name = model()->data(mi0, Qt::DisplayRole).toString();
    //qDebug() << "COLLAPSE NODE " << idx;
    emit requestCollapseChildren(idx);
}

void WatchWindow::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Delete) {
        QModelIndex idx = currentIndex();
        QModelIndex idx1 = idx.sibling(idx.row(), 0);
        QString exp = model()->data(idx1).toString();
        theDebuggerSetting(RemoveWatchExpression)->setValue(exp);
    }
    QTreeView::keyPressEvent(ev);
}

void WatchWindow::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu menu;
    QAction *act1 = new QAction("Adjust column widths to contents", &menu);
    QAction *act2 = new QAction("Always adjust column widths to contents", &menu);
    act2->setCheckable(true);
    act2->setChecked(m_alwaysResizeColumnsToContents);
    //QAction *act3 = 0;
    QAction *act4 = 0;

    menu.addAction(act1);
    menu.addAction(act2);

    QModelIndex idx = indexAt(ev->pos());
    QModelIndex mi0 = idx.sibling(idx.row(), 0);
    QString exp = model()->data(mi0).toString();
    QModelIndex mi1 = idx.sibling(idx.row(), 0);
    QString value = model()->data(mi1).toString();
    bool visual = false;

    menu.addSeparator();
    int type = (m_type == LocalsType) ? WatchExpression : RemoveWatchExpression;
    menu.addAction(theDebuggerSetting(type)->updatedAction(exp));

    visual = model()->data(mi0, VisualRole).toBool();
    //act4 = theDebuggerSetting(WatchExpressionInWindow)->action();
    //act4->setCheckable(true);
    //act4->setChecked(visual);
    //menu.addAction(act4);

    //act3 = new QAction(tr("Add to watch window..."), &menu); 
    //menu.addAction(act3);

    menu.addSeparator();
    menu.addAction(theDebuggerSetting(RecheckDumpers)->action());
    menu.addAction(theDebuggerSetting(UseDumpers)->action());
    menu.addSeparator();
    menu.addAction(theDebuggerSetting(SettingsDialog)->action());

    QAction *act = menu.exec(ev->globalPos());

    if (act == act1)
        resizeColumnsToContents();
    else if (act == act2)
        setAlwaysResizeColumnsToContents(!m_alwaysResizeColumnsToContents);
    else if (act == act4)
        model()->setData(mi0, !visual, VisualRole);
    else if (act == act4)
        model()->setData(mi0, !visual, VisualRole);
}

void WatchWindow::resizeColumnsToContents()
{
    resizeColumnToContents(0);
    resizeColumnToContents(1);
}

void WatchWindow::setAlwaysResizeColumnsToContents(bool on)
{
    if (!header())
        return;
    m_alwaysResizeColumnsToContents = on;
    QHeaderView::ResizeMode mode = on 
        ? QHeaderView::ResizeToContents : QHeaderView::Interactive;
    header()->setResizeMode(0, mode);
    header()->setResizeMode(1, mode);
}

void WatchWindow::editItem(const QModelIndex &idx)
{
    Q_UNUSED(idx); // FIXME
}

void WatchWindow::reset()
{
    int row = 0;
    if (m_type == TooltipType)
        row = 1;
    else if (m_type == WatchersType)
        row = 2;
    //qDebug() << "WATCHWINDOW::RESET" << row;
    QTreeView::reset(); 
    setRootIndex(model()->index(row, 0, model()->index(0, 0)));
    //setRootIndex(model()->index(0, 0));
    resetHelper(model()->index(0, 0));
}

void WatchWindow::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);

    setRootIsDecorated(true);
    header()->setDefaultAlignment(Qt::AlignLeft);
    header()->setResizeMode(QHeaderView::ResizeToContents);
    if (m_type != LocalsType)
        header()->hide();
}

void WatchWindow::resetHelper(const QModelIndex &idx)
{
    if (model()->data(idx, ExpandedRole).toBool()) {
        expand(idx);
        for (int i = 0, n = model()->rowCount(idx); i != n; ++i) {
            QModelIndex idx1 = model()->index(i, 0, idx);
            resetHelper(idx1);
        }
    }
}

