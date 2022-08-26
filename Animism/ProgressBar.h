#pragma once
#include <QProgressBar>

class ProgressBar : public QProgressBar
{
	Q_OBJECT

public:
	explicit ProgressBar(QWidget* parent = 0);

protected:
	void paintEvent(QPaintEvent*);
	void drawBg(QPainter* painter);
	void drawValue(QPainter* painter);
	void drawBorder(QPainter* painter);

private:
	QBrush valueBrush;          //进度画刷
	QColor valueColor;          //进度颜色
	QColor bgColor;             //背景颜色
	QColor textColor;           //文字颜色

	int radius;                 //圆角角度
	bool autoRadius;            //自动圆角
	bool showProgressRadius;    //显示进度圆角角度

	double borderWidth;         //边框宽度
	QColor borderColor;         //边框颜色    

public:
	QColor getValueColor()      const;
	QColor getBgColor()         const;
	QColor getTextColor()       const;

	int getRadius()             const;
	bool getAutoRadius()        const;
	bool getShowProgressRadius()const;

	double getBorderWidth()     const;
	QColor getBorderColor()     const;

	QSize sizeHint()            const;
	QSize minimumSizeHint()     const;

public Q_SLOTS:
	//设置进度画刷+进度颜色+背景颜色+文字颜色
	void setValueBrush(const QBrush& valueBrush);
	void setValueColor(const QColor& valueColor);
	void setBgColor(const QColor& bgColor);
	void setTextColor(const QColor& textColor);

	//设置圆角+自动圆角+是否显示进度圆角角度
	void setRadius(int radius);
	void setAutoRadius(bool autoRadius);
	void setShowProgressRadius(bool showProgressRadius);

	//设置边框宽度+颜色
	void setBorderWidth(double borderWidth);
	void setBorderColor(const QColor& borderColor);
};
