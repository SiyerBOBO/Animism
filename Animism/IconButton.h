#pragma once

#include <QPushButton>
#include <QPainter>
#include <QDebug>

class IconButton : public QPushButton {
	Q_OBJECT
	Q_ENUMS(TextAlign)
	Q_ENUMS(TrianglePosition)
	Q_ENUMS(LinePosition)
	Q_ENUMS(IconPosition)

	Q_PROPERTY(int paddingLeft READ getPaddingLeft WRITE setPaddingLeft)
	Q_PROPERTY(int paddingRight READ getPaddingRight WRITE setPaddingRight)
	Q_PROPERTY(int paddingTop READ getPaddingTop WRITE setPaddingTop)
	Q_PROPERTY(int paddingBottom READ getPaddingBottom WRITE setPaddingBottom)
	Q_PROPERTY(TextAlign textAlign READ getTextAlign WRITE setTextAlign)

	Q_PROPERTY(bool showTriangle READ getShowTriangle WRITE setShowTriangle)
	Q_PROPERTY(int triangleLen READ getTriangleLen WRITE setTriangleLen)
	Q_PROPERTY(TrianglePosition trianglePosition READ getTrianglePosition WRITE setTrianglePosition)
	Q_PROPERTY(QColor triangleColor READ getTriangleColor WRITE setTriangleColor)

	Q_PROPERTY(bool showIcon READ getShowIcon WRITE setShowIcon)
	Q_PROPERTY(int iconSpace READ getIconSpace WRITE setIconSpace)
	Q_PROPERTY(QSize iconSize READ getIconSize WRITE setIconSize)
	Q_PROPERTY(QPixmap iconNormal READ getIconNormal WRITE setIconNormal)
	Q_PROPERTY(QPixmap iconHover READ getIconHover WRITE setIconHover)
	Q_PROPERTY(QPixmap iconPress READ getIconPress WRITE setIconPress)
	Q_PROPERTY(QPixmap iconCheck READ getIconCheck WRITE setIconCheck)

	Q_PROPERTY(bool showLine READ getShowLine WRITE setShowLine)
	Q_PROPERTY(int lineSpace READ getLineSpace WRITE setLineSpace)
	Q_PROPERTY(int lineWidth READ getLineWidth WRITE setLineWidth)
	Q_PROPERTY(int linePadding READ getLinePadding WRITE setLinePadding)
	Q_PROPERTY(LinePosition linePosition READ getLinePosition WRITE setLinePosition)
	Q_PROPERTY(QColor lineColor READ getLineColor WRITE setLineColor)

	Q_PROPERTY(int bgRadius READ getBgRadius WRITE setBgRadius)
	Q_PROPERTY(QColor normalBgColor READ getNormalBgColor WRITE setNormalBgColor)
	Q_PROPERTY(QColor hoverBgColor READ getHoverBgColor WRITE setHoverBgColor)
	Q_PROPERTY(QColor pressBgColor READ getPressBgColor WRITE setPressBgColor)
	Q_PROPERTY(QColor checkBgColor READ getCheckBgColor WRITE setCheckBgColor)
	Q_PROPERTY(QColor normalTextColor READ getNormalTextColor WRITE setNormalTextColor)
	Q_PROPERTY(QColor hoverTextColor READ getHoverTextColor WRITE setHoverTextColor)
	Q_PROPERTY(QColor pressTextColor READ getPressTextColor WRITE setPressTextColor)
	Q_PROPERTY(QColor checkTextColor READ getCheckTextColor WRITE setCheckTextColor)

	Q_PROPERTY(bool showBorder READ getShowBorder WRITE setShowBorder)
	Q_PROPERTY(QColor borderColor READ getBorderColor WRITE setBorderColor)
	Q_PROPERTY(int borderWidth READ getBorderWidth WRITE setBorderWidth)

public:
	enum TextAlign {
		TextAlign_Left = 0x0001,    //????????
		TextAlign_Right = 0x0002,   //????????
		TextAlign_Top = 0x0020,     //????????
		TextAlign_Bottom = 0x0040,  //????????
		TextAlign_Center = 0x0004   //????????
	};

	enum TrianglePosition {
		TrianglePosition_Left = 0,  //????
		TrianglePosition_Right = 1, //????
		TrianglePosition_Top = 2,   //????
		TrianglePosition_Bottom = 3 //????
	};

	enum IconPosition {
		IconPosition_Left = 0,      //????
		IconPosition_Right = 1,     //????
		IconPosition_Top = 2,       //????
		IconPosition_Bottom = 3     //????
	};

	enum LinePosition {
		LinePosition_Left = 0,      //????
		LinePosition_Right = 1,     //????
		LinePosition_Top = 2,       //????
		LinePosition_Bottom = 3     //????
	};

	explicit IconButton(QWidget* parent = 0);

protected:
	void paintEvent(QPaintEvent*);
	void drawBg(QPainter* painter);
	void drawText(QPainter* painter);
	void drawIcon(QPainter* painter);
	void drawLine(QPainter* painter);
	void drawTriangle(QPainter* painter);

private:
	int paddingLeft;                    //????????????
	int paddingRight;                   //????????????
	int paddingTop;                     //????????????
	int paddingBottom;                  //????????????
	TextAlign textAlign;                //????????

	bool showTriangle;                  //??????????
	int triangleLen;                    //??????????
	TrianglePosition trianglePosition;  //??????????
	QColor triangleColor;               //??????????

	bool showIcon;                      //????????
	int iconSpace;                      //????????
	QSize iconSize;                     //????????
	QPixmap iconNormal;                 //????????
	QPixmap iconHover;                  //????????
	QPixmap iconPress;                  //????????
	QPixmap iconCheck;                  //????????

	bool showLine;                      //????????
	int lineSpace;                      //????????
	int lineWidth;                      //????????
	int linePadding;                    //????????????
	LinePosition linePosition;          //????????
	QColor lineColor;                   //????????

	QColor normalBgColor;               //????????????
	QColor hoverBgColor;                //????????????
	QColor pressBgColor;                //????????????
	QColor checkBgColor;                //????????????
	QColor normalTextColor;             //????????????
	QColor hoverTextColor;              //????????????
	QColor pressTextColor;              //????????????
	QColor checkTextColor;              //????????????

	int bgRadius;
	QBrush normalBgBrush;               //????????????
	QBrush hoverBgBrush;                //????????????
	QBrush pressBgBrush;                //????????????
	QBrush checkBgBrush;                //????????????

	bool showBorder;					//????????
	QColor borderColor;					//????????
	int borderWidth;					//????????

public:
	int getPaddingLeft()                const;
	int getPaddingRight()               const;
	int getPaddingTop()                 const;
	int getPaddingBottom()              const;
	TextAlign getTextAlign()            const;

	bool getShowTriangle()              const;
	int getTriangleLen()                const;
	TrianglePosition getTrianglePosition()const;
	QColor getTriangleColor()           const;

	bool getShowIcon()                  const;
	int getIconSpace()                  const;
	QSize getIconSize()                 const;
	QPixmap getIconNormal()             const;
	QPixmap getIconHover()              const;
	QPixmap getIconPress()				const;
	QPixmap getIconCheck()              const;

	bool getShowLine()                  const;
	int getLineSpace()                  const;
	int getLineWidth()                  const;
	int getLinePadding()                const;
	LinePosition getLinePosition()      const;
	QColor getLineColor()               const;

	int getBgRadius()                   const;
	QColor getNormalBgColor()           const;
	QColor getHoverBgColor()            const;
	QColor getPressBgColor()            const;
	QColor getCheckBgColor()            const;
	QColor getNormalTextColor()         const;
	QColor getHoverTextColor()          const;
	QColor getPressTextColor()          const;
	QColor getCheckTextColor()          const;

	bool getShowBorder()				const;
	QColor getBorderColor()				const;
	int getBorderWidth()				const;

	QSize sizeHint()                    const;
	QSize minimumSizeHint()             const;

public Q_SLOTS:
	//????????????
	void setPaddingLeft(int paddingLeft);
	void setPaddingRight(int paddingRight);
	void setPaddingTop(int paddingTop);
	void setPaddingBottom(int paddingBottom);
	void setPadding(int padding);
	void setPadding(int paddingLeft, int paddingRight, int paddingTop, int paddingBottom);

	//????????????
	void setTextAlign(const TextAlign& textAlign);

	//??????????????
	void setShowTriangle(bool showTriangle);
	//??????????????
	void setTriangleLen(int triangleLen);
	//??????????????
	void setTrianglePosition(const TrianglePosition& trianglePosition);
	//??????????????
	void setTriangleColor(const QColor& triangleColor);

	//????????????
	void setShowIcon(bool showIcon);
	//????????????
	void setIconSpace(int iconSpace);
	//????????????
	void setIconSize(const QSize& iconSize);
	//????????????
	void setIconNormal(const QPixmap& iconNormal);
	//????????????
	void setIconHover(const QPixmap& iconHover);
	//????????????
	void setIconPress(const QPixmap& iconPress);
	//????????????
	void setIconCheck(const QPixmap& iconCheck);

	//????????????
	void setShowLine(bool showLine);
	//????????????
	void setLineSpace(int lineSpace);
	//????????????
	void setLineWidth(int lineWidth);
	//????????????????
	void setLinePadding(int linePadding);
	//????????????
	void setLinePosition(const LinePosition& linePosition);
	//????????????
	void setLineColor(const QColor& lineColor);

	//????????????
	void setBgRadius(int bgRadius);
	//????????????????
	void setNormalBgColor(const QColor& normalBgColor);
	//????????????????
	void setHoverBgColor(const QColor& hoverBgColor);
	//????????????????
	void setPressBgColor(const QColor& pressBgColor);
	//????????????????
	void setCheckBgColor(const QColor& checkBgColor);

	//????????????????
	void setNormalTextColor(const QColor& normalTextColor);
	//????????????????
	void setHoverTextColor(const QColor& hoverTextColor);
	//????????????????
	void setPressTextColor(const QColor& pressTextColor);
	//????????????????
	void setCheckTextColor(const QColor& checkTextColor);

	//????????????????
	void setNormalBgBrush(const QBrush& normalBgBrush);
	//????????????????
	void setHoverBgBrush(const QBrush& hoverBgBrush);
	//????????????????
	void setPressBgBrush(const QBrush& pressBgBrush);
	//????????????????
	void setCheckBgBrush(const QBrush& checkBgBrush);

	void setShowBorder(bool showBorder);
	void setBorderColor(const QColor& borderColor);
	void setBorderWidth(int borderWidth);
};