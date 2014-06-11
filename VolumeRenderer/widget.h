/// http://qt-project.org/forums/viewthread/12207
/// http://stackoverflow.com/questions/9129955/draw-a-multi-colored-line-in-qt

#ifndef WIDGET_H
#define WIDGET_H

#include <iostream>
#include <QWidget>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPoint>
#include <QPainter>
#include <QLabel>
#include <QFileDialog>
#include <QMenu>

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
		auto originalPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
		//ui->label->setPixmap(originalPixmap.scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		pixmap = originalPixmap;
		//ui->label->setVisible(false);
		QWidget::showFullScreen();
	}

	virtual void show()
	{
		auto originalPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
		//ui->label->setPixmap(originalPixmap.scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		pixmap = originalPixmap;
		//ui->label->setVisible(false);
		QWidget::show();
	}

	virtual void mousePressEvent(QMouseEvent *e)
	{
		std::cout << "mousePressEvent " << e->pos().x() << " " << e->pos().y() << std::endl;

		if (e->button() == Qt::RightButton)
		{
			//if (selectionRect.contains(e->pos()))
			//	contextMenu.exec(this->mapToGlobal(e->pos()));
			contextMenu.exec(this->mapToGlobal(e->pos()));
		}
		else
		{
			selectionStarted = true;
			selectionRect.setTopLeft(e->pos());
			selectionRect.setBottomRight(e->pos());
		}
	}

	virtual void mouseMoveEvent(QMouseEvent *e)
	{
		std::cout << "mouseMoveEvent " << e->pos().x() << " " << e->pos().y() << std::endl;

		if (selectionStarted)
		{
			selectionRect.setBottomRight(e->pos());
			repaint();
		}
	}

	virtual void mouseReleaseEvent(QMouseEvent *e)
	{
		std::cout << "mouseReleaseEvent " << e->pos().x() << " " << e->pos().y() << std::endl;

		selectionStarted = false;

		std::cout << "rect " << selectionRect.x() << " " << selectionRect.y() << " " << selectionRect.width() << " " << selectionRect.height() << std::endl;

		auto p1 = selectionRect.topLeft();
		auto p2 = selectionRect.bottomRight();
		int x1 = p1.x() < p2.x() ? p1.x() : p2.x();
		int y1 = p1.y() < p2.y() ? p1.y() : p2.y();
		int x2 = p1.x() > p2.x() ? p1.x() : p2.x();
		int y2 = p1.y() > p2.y() ? p1.y() : p2.y();
		selectionRect.setCoords(x1, y1, x2, y2);

		repaint();

		//auto pic = ui->label->pixmap()->copy(selectionRect);
		//ui->label->setPixmap(pic);
	}

	virtual void paintEvent(QPaintEvent *e)
	{
		QWidget::paintEvent(e);
		//QPainter painter(ui->label);
		//painter.setPen(QPen(QBrush(QColor(0, 0, 0, 180)), 1, Qt::DashLine));
		//painter.setBrush(QBrush(QColor(255, 255, 255, 120)));

		//painter.drawRect(selectionRect);
		std::cout << "paintEvent" << std::endl;
		QPainter painter(this);
		painter.drawPixmap(selectionRect.topLeft(), pixmap.copy(selectionRect));

		// This is the important part you'll want to play with
		painter.setCompositionMode(QPainter::RasterOp_SourceAndNotDestination);
		QPen inverter(Qt::white);
		inverter.setWidth(10);
		painter.setPen(inverter);
		painter.drawLine(selectionRect.topLeft(), selectionRect.bottomRight());
	}

	void saveSlot()
	{
		QString fileName = QFileDialog::getSaveFileName(this, QObject::tr("Save File"),
			"/home",
			QObject::tr("Images (*.jpg)"));
		ui->label->pixmap()->copy(selectionRect).save(fileName);
	}

	void closeSlot()
	{
		this->close();
	}

private:
    Ui::Widget *ui;

	bool selectionStarted;
	QRect selectionRect;
	QMenu contextMenu;
	QPixmap pixmap;

};

#endif // WIDGET_H
