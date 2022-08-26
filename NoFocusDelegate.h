#pragma once

#include <QStyledItemDelegate>

enum {
    RowHoverRole = Qt::UserRole + 0x1000,
    RowBorderColorRole,
};

class NoFocusDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void setDrawHoverRowEnabled(bool enabled);

    void setHoveredRowForeground(const QBrush& brush);

    void setHoveredRowBackground(const QBrush& brush);

    void setHoveredRowBorderColor(const QColor& color);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;

private:
    QVector<QLineF> getRowLines(const QStyleOptionViewItem& option, const QModelIndex &index)const;
    void drawRowLines(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex &index, const QColor& lineColor)const;
    bool isHoveredRow(const QStyleOptionViewItem& option, const QModelIndex& index)const;

private:
    bool m_drawHoverRowEnabled = false;
    QBrush m_hoveredRowForeground;
    QBrush m_hoveredRowBackground;
    QColor m_hoveredBorderColor;
};
