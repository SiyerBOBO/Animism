#include "Welcome.h"
#include <QMovie>

Welcome::Welcome(QWidget *parent)
	: QWidget(parent)
{
	this->initUi();
}

Welcome::~Welcome()
{
}

void Welcome::initUi()
{
	ui.setupUi(this);
	this->_ptr_movie_home = new QMovie(this);
	this->_ptr_movie_home->setFileName(":/Animism/Resource/ani_home.gif");
	this->_ptr_movie_home->setScaledSize(QSize(300, 300));
	this->_ptr_movie_home->setCacheMode(QMovie::CacheAll);

	QObject::connect(this->_ptr_movie_home, &QMovie::frameChanged, this, [=](int frameNumber) {
		if (frameNumber == 59)
			this->_ptr_movie_home->jumpToFrame(30);
		});
	this->ui.label_logo->setMovie(this->_ptr_movie_home);

}

bool Welcome::event(QEvent* e) {
	switch (e->type())
	{
	case QEvent::Show: {
		this->_ptr_movie_home->start();
		break;
	}
	case QEvent::Hide: {
		this->_ptr_movie_home->stop();
		break;
	}
	default:
		break;
	}
	return QWidget::event(e);
}