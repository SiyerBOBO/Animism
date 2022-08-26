#pragma once

#include <QPushButton>

class ValueButton : public QPushButton
{
	Q_OBJECT

public:
	ValueButton(QWidget *parent);
	~ValueButton();

	void setValue(const QString& value);
	QString value() const;

protected:
	void initUi();
	void paintEvent(QPaintEvent* e) override;

private:
	QString _value;
};
