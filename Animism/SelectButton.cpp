#include "SelectButton.h"
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>

SelectButton::SelectButton(QWidget *parent)
	: QWidget(parent)
{
	this->initUi();
}

SelectButton::~SelectButton()
{
}

void SelectButton::setIcon(const QPixmap& icon)
{
	this->_icon = icon;
	this->_ptr_icon->setPixmap(this->_icon);
}

void SelectButton::setUuid(const QUuid& uuid)
{
	this->_uuid = uuid;
}

void SelectButton::setText(const QString& text)
{
	this->_ptr_text->setText(text);
}

QString SelectButton::text() const
{
	return this->_ptr_text->text();
}

QPixmap SelectButton::icon() const
{
	return this->_icon;
}

QUuid SelectButton::uuid() const
{
	return this->_uuid;
}

void SelectButton::setChecked(bool state)
{
	this->_ptr_switch->setChecked(state);
}

bool SelectButton::isChecked()
{
	return this->_ptr_switch->getChecked();
}

void SelectButton::initUi()
{
	this->setMinimumSize(200, 60);
	this->setMaximumSize(200, 60);
	this->setAttribute(Qt::WA_Hover);

	this->_ptr_icon = new QLabel(this);
	this->_ptr_icon->setMaximumSize(36, 36);
	this->_ptr_icon->setMinimumSize(36, 36);
	this->_ptr_icon->setScaledContents(true);

	this->_ptr_text = new QPushButton(this);
	this->_ptr_text->setMinimumHeight(30);
	this->_ptr_text->setMaximumHeight(30);
	this->_ptr_text->setFont(qApp->font());
	this->_ptr_text->setStyleSheet("QLabel{background:transparent;font-size:13px;}");
	this->_ptr_text->setCursor(Qt::PointingHandCursor);
	this->_ptr_text->setFlat(true);
	QObject::connect(this->_ptr_text, &QPushButton::clicked, this, &SelectButton::help);

	this->_ptr_switch = new SwitchButton(this);
	this->_ptr_switch->setMinimumSize(40, 20);
	this->_ptr_switch->setMaximumSize(40, 20);
	this->_ptr_switch->setCursor(Qt::PointingHandCursor);
	this->_ptr_switch->setAnimation(true);
	this->_ptr_switch->setShowText(false);
	this->_ptr_switch->setBgColorOff(QColor(195, 195, 195));
	this->_ptr_switch->setBgColorOn(QColor(0, 130, 255));
	QObject::connect(this->_ptr_switch, &SwitchButton::checkedChanged, this, &SelectButton::checked);

	this->_ptr_layout = new QHBoxLayout(this);
	this->_ptr_layout->addWidget(this->_ptr_icon);
	this->_ptr_layout->addWidget(this->_ptr_text);
	this->_ptr_layout->addWidget(this->_ptr_switch);
	this->_ptr_layout->setContentsMargins(20, 10, 20, 10);
	this->_ptr_layout->setSpacing(10);
	this->setLayout(this->_ptr_layout);
}

void SelectButton::paintEvent(QPaintEvent* e)
{
	if (this->underMouse()) 
	{
		QPainter painter(this);
		// painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
		painter.setPen(QColor(0, 130, 255));
		painter.setBrush(QColor(0, 130, 255, 10));
		painter.drawRect(this->rect().marginsRemoved(QMargins(0, 0, 1, 1)));
	}
	return QWidget::paintEvent(e);
}