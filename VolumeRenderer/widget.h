#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDesktopWidget>

#include "ui_widget.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

public slots:
	virtual void showFullScreen()
	{
		QWidget::showFullScreen();
		auto originalPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
		ui->label->setPixmap(originalPixmap.scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}

private:
    Ui::Widget *ui;
};

#endif // WIDGET_H
