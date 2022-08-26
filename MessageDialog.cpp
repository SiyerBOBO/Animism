#pragma execution_character_set("utf-8")
#include "MessageDialog.h"
#include <QDesktopWidget>
#include <QApplication>
#include <qmath.h>

MessageDialog::MessageDialog(QWidget* parent, QString title, QString message, bool isSignalBtn)
	: QDialog(parent), _str_title(title), _str_message(message), _bool_isSignalBtn(isSignalBtn) {
	this->initUi();
}

MessageDialog::~MessageDialog() {
}

void MessageDialog::initUi() {
	this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowTitle(this->_str_title);

    this->setMinimumSize(QSize(360, 140));
    this->setMaximumSize(QSize(360, 140));
    this->setStyleSheet("QLabel{font: 15px \"Microsoft YaHei\";color: rgb(50, 50, 50);}\n"
        "QPushButton{border: none;border-radius: 5px;color: rgb(1, 91, 187);font: 15px \"Microsoft YaHei\";background-color: transparent;}\n"
        "QPushButton:hover{border: none;border-radius: 5px;color: rgb(1, 91, 187);font: 15px \"Microsoft YaHei\";background-color: rgb(230, 230, 230);}\n"
        "QPushButton:pressed{border: none;border-radius: 5px;color: rgb(1, 91, 187);font: 15px \"Microsoft YaHei\";background-color: rgb(220, 220, 220);}");
    this->_ptr_layout1 = new QVBoxLayout(this);
    this->_ptr_layout1->setSpacing(0);
    this->_ptr_layout1->setContentsMargins(20, 20, 20, 20);
    this->_ptr_label = new QLabel(this);
    this->_ptr_label->setAlignment(Qt::AlignCenter);
    this->_ptr_label->setText(this->_str_message);
    this->_ptr_layout1->addWidget(this->_ptr_label);
    this->_ptr_layout2 = new QHBoxLayout();
    this->_ptr_layout2->setSpacing(10);
    this->_ptr_spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    this->_ptr_layout2->addItem(this->_ptr_spacerItem1);
    this->_ptr_button_accept = new QPushButton(this);
    this->_ptr_button_accept->setMinimumSize(QSize(100, 35));
    this->_ptr_button_accept->setMaximumSize(QSize(100, 35));
    this->_ptr_button_accept->setText(tr("确定"));
    this->_ptr_layout2->addWidget(this->_ptr_button_accept);
    this->_ptr_button_reject = new QPushButton(this);
    this->_ptr_button_reject->setMinimumSize(QSize(100, 35));
    this->_ptr_button_reject->setMaximumSize(QSize(100, 35));
    this->_ptr_button_reject->setText(tr("取消"));
    this->_ptr_layout2->addWidget(this->_ptr_button_reject);
    this->_ptr_spacerItem2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    this->_ptr_layout2->addItem(this->_ptr_spacerItem2);
    this->_ptr_layout1->addLayout(this->_ptr_layout2);
    this->_ptr_layout1->setStretch(1, 3);

    this->_ptr_button_reject->setVisible(!_bool_isSignalBtn);

    QObject::connect(this->_ptr_button_accept, &QPushButton::clicked, this, &QDialog::accept);
    QObject::connect(this->_ptr_button_reject, &QPushButton::clicked, this, &QDialog::reject);

	QRect _rect_frame = this->rect();
	_rect_frame.moveCenter(qApp->desktop()->geometry().center());
	this->move(_rect_frame.topLeft());
}

bool MessageDialog::event(QEvent* e) {
    static bool _b_windowActive = false;
    static bool _b_mousePressed = false;
    static QPoint _point_mousePressed;
    switch (e->type())
    {
    case QEvent::Paint: {
        int _int_winShadow = 15;
        int _int_winRadius = 10;
        QRect _rect_winRect = this->rect().marginsRemoved(QMargins(10, 10, 10, 10));

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

        QPainterPath path;
        if (this->isMaximized()) {
            path.addRect(_rect_winRect);
            painter.fillPath(path, QColor(249, 252, 255, 255));
        }
        else {
            path.addRoundedRect(_rect_winRect, _int_winRadius, _int_winRadius);
            painter.fillPath(path, QColor(249, 252, 255, 255));
            QColor color = Qt::black;
            for (int i = 0; i < _int_winShadow; i++)
            {
                _rect_winRect = _rect_winRect.marginsAdded(QMargins(1, 1, 1, 1));
                double _double_alpha = _b_windowActive ? 30 - 11 * qSqrt(i) : 20 - 8 * qSqrt(i);
                path.clear();
                path.setFillRule(Qt::WindingFill);
                path.addRoundedRect(_rect_winRect, _int_winRadius, _int_winRadius++);
                color.setAlpha(_double_alpha > 0 ? _double_alpha : 0);
                painter.setPen(color);
                painter.drawPath(path);
            }
        }
        break;
    }
    case QEvent::MouseButtonPress: {
        if (((QMouseEvent*)e)->button() == Qt::LeftButton) {
            _b_mousePressed = true;
            _point_mousePressed = ((QMouseEvent*)e)->globalPos() - this->pos();
        }
        break;
    }
    case QEvent::MouseMove: {
        if (_b_mousePressed && this->underMouse())
            this->move(((QMouseEvent*)e)->globalPos() - _point_mousePressed);
        break;
    }
    case QEvent::MouseButtonRelease: {
        _b_mousePressed = false;
        break;
    }
    case QEvent::WindowActivate:
    {
        _b_windowActive = true;
        this->update();
        break;
    }
    case QEvent::WindowDeactivate:
    {
        _b_windowActive = false;
        this->update();
        break;
    }
    default:
        break;
    }
	return QDialog::event(e);
}

bool MessageDialog::eventFilter(QObject* object, QEvent* event) {
	return QWidget::eventFilter(object, event);
}