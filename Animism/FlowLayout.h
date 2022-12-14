#pragma once

#include <QLayout>
#include <QRect>
#include <QStyle>

#include <QDebug>

class FlowLayout : public QLayout
{
public:
	FlowLayout(QWidget* parent, int margin = 0, int hSpacing = 0, int vSpacing = 0);
	FlowLayout(int margin = 0, int hSpacing = 0, int vSpacing = 0);
	~FlowLayout();

	void addItem(QLayoutItem* item);
	int horizontalSpacing() const;
	int verticalSpacing() const;
	Qt::Orientations expandingDirections() const;
	bool hasHeightForWidth() const;
	int heightForWidth(int) const;
	int count() const;
	QLayoutItem* itemAt(int index) const;
	QSize minimumSize() const;
	void setGeometry(const QRect& rect);
	QSize sizeHint() const;
	QLayoutItem* takeAt(int index);

private:
	int doLayout(const QRect& rect, bool testOnly) const;
	int smartSpacing(QStyle::PixelMetric pm) const;

	QList<QLayoutItem*> itemList;
	int m_hSpace;
	int m_vSpace;
};