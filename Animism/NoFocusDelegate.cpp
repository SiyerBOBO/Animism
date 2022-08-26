#include "NoFocusDelegate.h"
#include <QTableView>
#include <QCheckBox>
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QPen>

void NoFocusDelegate::setDrawHoverRowEnabled(bool enabled)
{
    m_drawHoverRowEnabled = enabled;
}

void NoFocusDelegate::setHoveredRowForeground(const QBrush &brush)
{
    m_hoveredRowForeground = brush;
}

void NoFocusDelegate::setHoveredRowBackground(const QBrush &brush)
{
    m_hoveredRowBackground = brush;
}

void NoFocusDelegate::setHoveredRowBorderColor(const QColor &color)
{
    m_hoveredBorderColor = color;
}

void NoFocusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    if(opt.state.testFlag(QStyle::State_HasFocus)){
        opt.state = opt.state &~ QStyle::State_HasFocus;
    }
    QStyledItemDelegate::paint(painter, opt, index);
    if(!m_drawHoverRowEnabled){
        return ;
    }
    if(isHoveredRow(opt, index) && m_hoveredBorderColor.isValid()){
        //draw border
        drawRowLines(painter, opt, index, m_hoveredBorderColor);
    }
}

void NoFocusDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    if(!m_drawHoverRowEnabled){
        return ;
    }
    if(isHoveredRow(*option, index)){
        option->backgroundBrush = m_hoveredRowBackground;
        option->palette.setBrush(QPalette::Text, m_hoveredRowForeground);
        option->palette.setBrush(QPalette::Active, QPalette::Highlight, m_hoveredRowBackground);
        option->palette.setBrush(QPalette::Active, QPalette::HighlightedText, m_hoveredRowForeground);
    }
}

QVector<QLineF> NoFocusDelegate::getRowLines(const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    QVector<QLineF> lines;
    lines << QLineF{option.rect.topLeft(), option.rect.topRight()}
          << QLineF{option.rect.bottomLeft(), option.rect.bottomRight()};
    if(index.column() == 0){
        lines << QLineF{option.rect.topLeft(), option.rect.bottomLeft()};
    }
    else if(index.column() == index.model()->columnCount() - 1){
        lines << QLineF{option.rect.topRight(), option.rect.bottomRight()};
    }
    return lines;
}

void NoFocusDelegate::drawRowLines(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex& index, const QColor &lineColor) const
{
    painter->save();
    QVector<QLineF> lines = getRowLines(option, index);
    painter->setPen(QPen(lineColor, 1));
    painter->drawLines(lines);
    painter->restore();
}

bool NoFocusDelegate::isHoveredRow(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bool ret = false;
    int gridWidth = 0;
    if(const QTableView* tableView = qobject_cast<const QTableView*>(option.widget)){
        gridWidth = tableView->showGrid() ? 1 : 0;
    }
    for(int i = 0; i < index.model()->columnCount(); ++i){
        QModelIndex idx = index.model()->index(index.row(), i);
        if(const QAbstractItemView* view = qobject_cast<const QAbstractItemView*>(option.widget)){
            QStyleOptionViewItem o;
            o.initFrom(view);
            o.rect = view->visualRect(idx).adjusted(0, 0, gridWidth, gridWidth);
            if(o.rect.contains(view->viewport()->mapFromGlobal(QCursor::pos()))){
                ret = true;
            }
        }
    }
    return ret;
}
