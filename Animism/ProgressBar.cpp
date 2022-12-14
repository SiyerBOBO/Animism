#pragma execution_character_set("utf-8")

#include "ProgressBar.h"
#include "qpainter.h"
#include "qevent.h"
#include "qpainterpath.h"

ProgressBar::ProgressBar(QWidget* parent) : QProgressBar(parent) {
	valueBrush = Qt::NoBrush;
	valueColor = QColor(34, 163, 169);
	bgColor = QColor(40, 45, 48);
	textColor = QColor(255, 255, 255);
	radius = 0;
	autoRadius = false;
	showProgressRadius = false;
	borderWidth = 0;
	borderColor = QColor(255, 255, 255);
}

void ProgressBar::paintEvent(QPaintEvent* e) {
	//绘制准备工作,启用反锯齿,平移坐标轴中心,等比例缩放
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	if (autoRadius)
		radius = this->height() / 2;
	//绘制背景
	drawBg(&painter);
	//绘制已播放进度
	drawValue(&painter);
	//绘制边框
	if (borderWidth > 0)
		drawBorder(&painter);
}

void ProgressBar::drawBg(QPainter* painter) {
	painter->save();
	painter->setPen(Qt::NoPen);
	painter->setBrush(bgColor);
	//将背景区域减少一个像素,这样圆角区域不会产生轻微锯齿状
	QRect rect(1, 1, this->width() - 2, this->height() - 2);
	painter->drawRoundedRect(rect, radius, radius);
	painter->restore();
}

void ProgressBar::drawValue(QPainter* painter) {
	painter->save();
	painter->setPen(Qt::NoPen);
	double percent = (double)this->value() / (this->maximum() - this->minimum());
	int width = this->width() * percent;
	//设置另类画刷
	QBrush brush;
	brush.setColor(valueColor);
	//这个画刷样式自己可以更改,形成各种风格
	brush.setStyle(Qt::SolidPattern);
	painter->setBrush(brush);
	//定义了画刷则取画刷,可以形成渐变效果
	painter->setBrush(valueBrush == Qt::NoBrush ? brush : valueBrush);
	//计算绘制的区域,需要裁剪圆角部分
	QPainterPath clipPath;
	clipPath.addRoundedRect(this->rect(), radius, radius);
	painter->setClipPath(clipPath);
	QRect rect(0, 0, width, this->height());
	if (showProgressRadius)
		painter->drawRoundedRect(rect, radius, radius);
	else
		painter->drawRect(rect);
	//绘制值
	//painter->setPen(textColor);
	//painter->drawText(this->rect(), Qt::AlignCenter, QString("%1").arg(this->value()));
	painter->restore();
}

void ProgressBar::drawBorder(QPainter* painter) {
	painter->save();
	QPen pen;
	pen.setWidthF(borderWidth);
	pen.setColor(borderColor);
	painter->setPen(pen);
	painter->setBrush(Qt::NoBrush);
	QRect rect(1, 1, this->width() - 2, this->height() - 2);
	painter->drawRoundedRect(rect, radius, radius);
	painter->restore();
}

QColor ProgressBar::getValueColor() const {
	return this->valueColor;
}

QColor ProgressBar::getBgColor() const {
	return this->bgColor;
}

QColor ProgressBar::getTextColor() const {
	return this->textColor;
}

int ProgressBar::getRadius() const {
	return this->radius;
}

bool ProgressBar::getAutoRadius() const {
	return this->autoRadius;
}

bool ProgressBar::getShowProgressRadius() const {
	return this->showProgressRadius;
}

double ProgressBar::getBorderWidth() const {
	return this->borderWidth;
}

QColor ProgressBar::getBorderColor() const {
	return this->borderColor;
}

QSize ProgressBar::sizeHint() const {
	return QSize(300, 30);
}

QSize ProgressBar::minimumSizeHint() const {
	return QSize(10, 5);
}

void ProgressBar::setValueBrush(const QBrush& valueBrush) {
	if (this->valueBrush != valueBrush) {
		this->valueBrush = valueBrush;
		this->update();
	}
}

void ProgressBar::setValueColor(const QColor& valueColor) {
	if (this->valueColor != valueColor) {
		this->valueColor = valueColor;
		this->update();
	}
}

void ProgressBar::setBgColor(const QColor& bgColor) {
	if (this->bgColor != bgColor) {
		this->bgColor = bgColor;
		this->update();
	}
}

void ProgressBar::setTextColor(const QColor& textColor) {
	if (this->textColor != textColor) {
		this->textColor = textColor;
		this->update();
	}
}

void ProgressBar::setRadius(int radius) {
	if (this->radius != radius) {
		this->radius = radius;
		this->update();
	}
}

void ProgressBar::setAutoRadius(bool autoRadius) {
	if (this->autoRadius != autoRadius) {
		this->autoRadius = autoRadius;
		this->update();
	}
}

void ProgressBar::setShowProgressRadius(bool showProgressRadius) {
	if (this->showProgressRadius != showProgressRadius) {
		this->showProgressRadius = showProgressRadius;
		this->update();
	}
}

void ProgressBar::setBorderWidth(double borderWidth) {
	if (this->borderWidth != borderWidth) {
		this->borderWidth = borderWidth;
		this->update();
	}
}

void ProgressBar::setBorderColor(const QColor& borderColor) {
	if (this->borderColor != borderColor) {
		this->borderColor = borderColor;
		this->update();
	}
}
