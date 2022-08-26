#pragma execution_character_set("utf-8")
#include "IconButton.h"

IconButton::IconButton(QWidget* parent)
	: QPushButton(parent) {
	paddingLeft = 20;
	paddingRight = 5;
	paddingTop = 5;
	paddingBottom = 5;
	textAlign = TextAlign_Left;

	showTriangle = false;
	triangleLen = 5;
	trianglePosition = TrianglePosition_Right;
	triangleColor = QColor(255, 255, 255);

	showIcon = false;
	iconSpace = 10;
	iconSize = QSize(16, 16);
	iconNormal = QPixmap();
	iconHover = QPixmap();
	iconPress = QPixmap();
	iconCheck = QPixmap();

	showLine = false;
	lineSpace = 0;
	lineWidth = 5;
	linePadding = 0;
	linePosition = LinePosition_Left;
	lineColor = QColor(0, 187, 158);

	normalBgColor = QColor(230, 230, 230);
	hoverBgColor = QColor(130, 130, 130);
	checkBgColor = QColor(80, 80, 80);
	pressBgColor = QColor(50, 50, 50);
	normalTextColor = QColor(100, 100, 100);
	hoverTextColor = QColor(255, 255, 255);
	pressTextColor = QColor(255, 255, 255);
	checkTextColor = QColor(255, 255, 255);

	bgRadius = 0;
	normalBgBrush = Qt::NoBrush;
	hoverBgBrush = Qt::NoBrush;
	pressBgBrush = Qt::NoBrush;
	checkBgBrush = Qt::NoBrush;

	showBorder = false;
	borderColor = Qt::darkGray;
	borderWidth = 1;

	setCheckable(true);
}

void IconButton::paintEvent(QPaintEvent*) {
	//绘制准备工作,启用反锯齿
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

	//绘制背景
	drawBg(&painter);
	//绘制文字
	drawText(&painter);
	//绘制图标
	drawIcon(&painter);
	//绘制边框线条
	drawLine(&painter);
	//绘制倒三角
	drawTriangle(&painter);
}

void IconButton::drawBg(QPainter* painter) {
	painter->save();

	if (this->showBorder) {
		QPen pen;
		pen.setColor(this->borderColor);
		pen.setWidth(this->borderWidth);
		painter->setPen(this->borderColor);
	}
	else
		painter->setPen(Qt::NoPen);

	int width = this->width();
	int height = this->height();

	QRect bgRect;
	if (linePosition == LinePosition_Left)
		bgRect = QRect(lineSpace, 0, width - lineSpace, height);
	else if (linePosition == LinePosition_Right)
		bgRect = QRect(0, 0, width - lineSpace, height);
	else if (linePosition == LinePosition_Top)
		bgRect = QRect(0, lineSpace, width, height - lineSpace);
	else if (linePosition == LinePosition_Bottom)
		bgRect = QRect(0, 0, width, height - lineSpace);

	//如果画刷存在则取画刷
	QBrush bgBrush;
	if(this->isDown())
		bgBrush = pressBgBrush;
	else if (isChecked())
		bgBrush = checkBgBrush;
	else if (this->underMouse())
		bgBrush = hoverBgBrush;
	else
		bgBrush = normalBgBrush;

	if (bgBrush != Qt::NoBrush)
		painter->setBrush(bgBrush);
	else {
		//根据当前状态选择对应颜色
		QColor bgColor;
		if (this->isDown())
			bgColor = pressBgColor;
		else if (isChecked())
			bgColor = checkBgColor;
		else if (this->underMouse())
			bgColor = hoverBgColor;
		else
			bgColor = normalBgColor;
		painter->setBrush(bgColor);
	}

	painter->drawRoundedRect(bgRect, bgRadius, bgRadius);
	painter->restore();
}

void IconButton::drawText(QPainter* painter) {
	painter->save();
	painter->setBrush(Qt::NoBrush);

	//根据当前状态选择对应颜色
	QColor textColor;
	if (this->isDown())
		textColor = pressTextColor;
	else if (isChecked())
		textColor = checkTextColor;
	else if (this->underMouse())
		textColor = hoverTextColor;
	else
		textColor = normalTextColor;

	QRect textRect = QRect(this->showIcon ? paddingLeft + iconSize.width() : paddingLeft, paddingTop, width() - paddingLeft - paddingRight, height() - paddingTop - paddingBottom);
	QString strDes = QFontMetrics(this->font()).elidedText(text(), Qt::ElideRight, textRect.width());
	painter->setFont(this->font());
	painter->setPen(textColor);
	painter->drawText(textRect, textAlign | Qt::AlignVCenter, strDes);
	painter->restore();
}

void IconButton::drawIcon(QPainter* painter) {
	if (!showIcon)
		return;

	painter->save();

	QPixmap pix;
	if (this->isDown())
		pix = iconPress;
	else if (isChecked())
		pix = iconCheck;
	else if (this->underMouse())
		pix = iconHover;
	else
		pix = iconNormal;

	if (!pix.isNull()) {
		//等比例平滑缩放图标
		pix = pix.scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		painter->drawPixmap(iconSpace, (height() - iconSize.height()) / 2, pix);
	}
	painter->restore();
}

void IconButton::drawLine(QPainter* painter) {
	if (!showLine)
		return;

	if (!isChecked())
		return;

	painter->save();

	QPen pen;
	pen.setWidth(lineWidth);
	pen.setColor(lineColor);
	painter->setPen(pen);

	//根据线条位置设置线条坐标
	QPoint pointStart, pointEnd;
	if (linePosition == LinePosition_Left) {
		pointStart = QPoint(0, linePadding);
		pointEnd = QPoint(0, height() - linePadding);
	}
	else if (linePosition == LinePosition_Right) {
		pointStart = QPoint(width(), linePadding);
		pointEnd = QPoint(width(), height() - linePadding);
	}
	else if (linePosition == LinePosition_Top) {
		pointStart = QPoint(linePadding, 0);
		pointEnd = QPoint(width() - linePadding, 0);
	}
	else if (linePosition == LinePosition_Bottom) {
		pointStart = QPoint(linePadding, height());
		pointEnd = QPoint(width() - linePadding, height());
	}

	painter->drawLine(pointStart, pointEnd);
	painter->restore();
}

void IconButton::drawTriangle(QPainter* painter) {
	if (!showTriangle)
		return;

	//选中或者悬停显示
	if (!this->underMouse() && !isChecked())
		return;

	painter->save();
	painter->setPen(Qt::NoPen);
	painter->setBrush(triangleColor);

	//绘制在右侧中间,根据设定的倒三角的边长设定三个点位置
	int width = this->width();
	int height = this->height();
	int midWidth = width / 2;
	int midHeight = height / 2;

	QPolygon pts;
	if (trianglePosition == TrianglePosition_Left)
		pts.setPoints(3, triangleLen, midHeight, 0, midHeight - triangleLen, 0, midHeight + triangleLen);
	else if (trianglePosition == TrianglePosition_Right)
		pts.setPoints(3, width - triangleLen, midHeight, width, midHeight - triangleLen, width, midHeight + triangleLen);
	else if (trianglePosition == TrianglePosition_Top)
		pts.setPoints(3, midWidth, triangleLen, midWidth - triangleLen, 0, midWidth + triangleLen, 0);
	else if (trianglePosition == TrianglePosition_Bottom)
		pts.setPoints(3, midWidth, height - triangleLen, midWidth - triangleLen, height, midWidth + triangleLen, height);

	painter->drawPolygon(pts);
	painter->restore();
}

int IconButton::getPaddingLeft() const {
	return this->paddingLeft;
}

int IconButton::getPaddingRight() const {
	return this->paddingRight;
}

int IconButton::getPaddingTop() const {
	return this->paddingTop;
}

int IconButton::getPaddingBottom() const {
	return this->paddingBottom;
}

IconButton::TextAlign IconButton::getTextAlign() const {
	return this->textAlign;
}

bool IconButton::getShowTriangle() const {
	return this->showTriangle;
}

int IconButton::getTriangleLen() const {
	return this->triangleLen;
}

IconButton::TrianglePosition IconButton::getTrianglePosition() const {
	return this->trianglePosition;
}

QColor IconButton::getTriangleColor() const {
	return this->triangleColor;
}

bool IconButton::getShowIcon() const {
	return this->showIcon;
}

int IconButton::getIconSpace() const {
	return this->iconSpace;
}

QSize IconButton::getIconSize() const {
	return this->iconSize;
}

QPixmap IconButton::getIconNormal() const {
	return this->iconNormal;
}

QPixmap IconButton::getIconHover() const {
	return this->iconHover;
}

QPixmap IconButton::getIconPress() const {
	return this->iconPress;
}

QPixmap IconButton::getIconCheck() const {
	return this->iconCheck;
}

bool IconButton::getShowLine() const {
	return this->showLine;
}

int IconButton::getLineSpace() const {
	return this->lineSpace;
}

int IconButton::getLineWidth() const {
	return this->lineWidth;
}

int IconButton::getLinePadding() const {
	return this->linePadding;
}

IconButton::LinePosition IconButton::getLinePosition() const {
	return this->linePosition;
}

QColor IconButton::getLineColor() const {
	return this->lineColor;
}

int IconButton::getBgRadius() const {
	return this->bgRadius;
}

QColor IconButton::getNormalBgColor() const {
	return this->normalBgColor;
}

QColor IconButton::getHoverBgColor() const {
	return this->hoverBgColor;
}

QColor IconButton::getPressBgColor() const {
	return this->pressBgColor;
}

QColor IconButton::getCheckBgColor() const {
	return this->checkBgColor;
}

QColor IconButton::getNormalTextColor() const {
	return this->normalTextColor;
}

QColor IconButton::getHoverTextColor() const {
	return this->hoverTextColor;
}

QColor IconButton::getPressTextColor() const {
	return this->pressTextColor;
}

QColor IconButton::getCheckTextColor() const {
	return this->checkTextColor;
}

bool IconButton::getShowBorder() const {
	return this->showBorder;
}

QColor IconButton::getBorderColor() const {
	return this->borderColor;
}

int IconButton::getBorderWidth() const {
	return this->borderWidth;
}

QSize IconButton::sizeHint() const {
	return QSize(100, 30);
}

QSize IconButton::minimumSizeHint() const {
	return QSize(20, 10);
}

void IconButton::setPaddingLeft(int paddingLeft) {
	if (this->paddingLeft != paddingLeft) {
		this->paddingLeft = paddingLeft;
		this->update();
	}
}

void IconButton::setPaddingRight(int paddingRight) {
	if (this->paddingRight != paddingRight) {
		this->paddingRight = paddingRight;
		this->update();
	}
}

void IconButton::setPaddingTop(int paddingTop) {
	if (this->paddingTop != paddingTop) {
		this->paddingTop = paddingTop;
		this->update();
	}
}

void IconButton::setPaddingBottom(int paddingBottom) {
	if (this->paddingBottom != paddingBottom) {
		this->paddingBottom = paddingBottom;
		this->update();
	}
}

void IconButton::setPadding(int padding) {
	setPadding(padding, padding, padding, padding);
}

void IconButton::setPadding(int paddingLeft, int paddingRight, int paddingTop, int paddingBottom) {
	this->paddingLeft = paddingLeft;
	this->paddingRight = paddingRight;
	this->paddingTop = paddingTop;
	this->paddingBottom = paddingBottom;
	this->update();
}

void IconButton::setTextAlign(const IconButton::TextAlign& textAlign) {
	if (this->textAlign != textAlign) {
		this->textAlign = textAlign;
		this->update();
	}
}

void IconButton::setShowTriangle(bool showTriangle) {
	if (this->showTriangle != showTriangle) {
		this->showTriangle = showTriangle;
		this->update();
	}
}

void IconButton::setTriangleLen(int triangleLen) {
	if (this->triangleLen != triangleLen) {
		this->triangleLen = triangleLen;
		this->update();
	}
}

void IconButton::setTrianglePosition(const IconButton::TrianglePosition& trianglePosition) {
	if (this->trianglePosition != trianglePosition) {
		this->trianglePosition = trianglePosition;
		this->update();
	}
}

void IconButton::setTriangleColor(const QColor& triangleColor) {
	if (this->triangleColor != triangleColor) {
		this->triangleColor = triangleColor;
		this->update();
	}
}

void IconButton::setShowIcon(bool showIcon) {
	if (this->showIcon != showIcon) {
		this->showIcon = showIcon;
		this->update();
	}
}

void IconButton::setIconSpace(int iconSpace) {
	if (this->iconSpace != iconSpace) {
		this->iconSpace = iconSpace;
		this->update();
	}
}

void IconButton::setIconSize(const QSize& iconSize) {
	if (this->iconSize != iconSize) {
		this->iconSize = iconSize;
		this->update();
	}
}

void IconButton::setIconNormal(const QPixmap& iconNormal) {
	this->iconNormal = iconNormal;
	this->update();
}

void IconButton::setIconHover(const QPixmap& iconHover) {
	this->iconHover = iconHover;
	this->update();
}

void IconButton::setIconPress(const QPixmap& iconPress) {
	this->iconPress = iconPress;
	this->update();
}

void IconButton::setIconCheck(const QPixmap& iconCheck) {
	this->iconCheck = iconCheck;
	this->update();
}

void IconButton::setShowLine(bool showLine) {
	if (this->showLine != showLine) {
		this->showLine = showLine;
		this->update();
	}
}

void IconButton::setLineSpace(int lineSpace) {
	if (this->lineSpace != lineSpace) {
		this->lineSpace = lineSpace;
		this->update();
	}
}

void IconButton::setLineWidth(int lineWidth) {
	if (this->lineWidth != lineWidth) {
		this->lineWidth = lineWidth;
		this->update();
	}
}

void IconButton::setLinePadding(int linePadding) {
	if (this->linePadding != linePadding) {
		this->linePadding = linePadding;
		this->update();
	}
}

void IconButton::setLinePosition(const IconButton::LinePosition& linePosition) {
	if (this->linePosition != linePosition) {
		this->linePosition = linePosition;
		this->update();
	}
}

void IconButton::setLineColor(const QColor& lineColor) {
	if (this->lineColor != lineColor) {
		this->lineColor = lineColor;
		this->update();
	}
}

void IconButton::setBgRadius(int bgRadius) {
	if (this->bgRadius != bgRadius) {
		this->bgRadius = bgRadius;
		this->update();
	}
}

void IconButton::setNormalBgColor(const QColor& normalBgColor) {
	if (this->normalBgColor != normalBgColor) {
		this->normalBgColor = normalBgColor;
		this->update();
	}
}

void IconButton::setHoverBgColor(const QColor& hoverBgColor) {
	if (this->hoverBgColor != hoverBgColor) {
		this->hoverBgColor = hoverBgColor;
		this->update();
	}
}

void IconButton::setPressBgColor(const QColor& pressBgColor) {
	if (this->pressBgColor != pressBgColor) {
		this->pressBgColor = pressBgColor;
		this->update();
	}
}

void IconButton::setCheckBgColor(const QColor& checkBgColor) {
	if (this->checkBgColor != checkBgColor) {
		this->checkBgColor = checkBgColor;
		this->update();
	}
}

void IconButton::setNormalTextColor(const QColor& normalTextColor) {
	if (this->normalTextColor != normalTextColor) {
		this->normalTextColor = normalTextColor;
		this->update();
	}
}

void IconButton::setHoverTextColor(const QColor& hoverTextColor) {
	if (this->hoverTextColor != hoverTextColor) {
		this->hoverTextColor = hoverTextColor;
		this->update();
	}
}

void IconButton::setPressTextColor(const QColor& pressTextColor) {
	if (this->pressTextColor != pressTextColor) {
		this->pressTextColor = pressTextColor;
		this->update();
	}
}

void IconButton::setCheckTextColor(const QColor& checkTextColor) {
	if (this->checkTextColor != checkTextColor) {
		this->checkTextColor = checkTextColor;
		this->update();
	}
}

void IconButton::setNormalBgBrush(const QBrush& normalBgBrush) {
	if (this->normalBgBrush != normalBgBrush) {
		this->normalBgBrush = normalBgBrush;
		this->update();
	}
}

void IconButton::setHoverBgBrush(const QBrush& hoverBgBrush) {
	if (this->hoverBgBrush != hoverBgBrush) {
		this->hoverBgBrush = hoverBgBrush;
		this->update();
	}
}

void IconButton::setPressBgBrush(const QBrush& pressBgBrush) {
	if (this->pressBgBrush != pressBgBrush) {
		this->pressBgBrush = pressBgBrush;
		this->update();
	}
}

void IconButton::setCheckBgBrush(const QBrush& checkBgBrush) {
	if (this->checkBgBrush != checkBgBrush) {
		this->checkBgBrush = checkBgBrush;
		this->update();
	}
}

void IconButton::setShowBorder(bool showBorder) {
	if (this->showBorder == showBorder)
		return;
	this->showBorder = showBorder;
	this->update();
}

void IconButton::setBorderColor(const QColor& borderColor) {
	if (this->borderColor == borderColor)
		return;
	this->borderColor = borderColor;
	this->update();
}

void IconButton::setBorderWidth(int borderWidth) {
	if (this->borderWidth == borderWidth)
		return;
	this->borderWidth = borderWidth;
	this->update();
}
