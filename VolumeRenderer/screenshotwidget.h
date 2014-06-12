/// http://qt-project.org/forums/viewthread/12207
/// http://stackoverflow.com/questions/9129955/draw-a-multi-colored-line-in-qt

#ifndef SCREENSHOTWIDGET_H
#define SCREENSHOTWIDGET_H

#include <iostream>
#include <algorithm>

#include <QWidget>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPoint>
#include <QPainter>
#include <QLabel>
#include <QFileDialog>
#include <QMenu>
#include <QApplication>

namespace Ui {
	class ScreenshotWidget;
}

class ScreenshotWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ScreenshotWidget(QWidget *parent = 0);
	~ScreenshotWidget();

	bool Auto_open_selected_image() const { return auto_open_selected_image; }
	void Auto_open_selected_image(bool val) { auto_open_selected_image = val; }

signals:
	void region_selected(QString filename);

public slots:

	virtual void show()
	{
		pixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
		QWidget::show();
	}

	virtual void showMinimized()
	{
		pixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
		QWidget::showMinimized();
	}

	virtual void showMaximized()
	{
		pixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
		QWidget::showMaximized();
	}

	virtual void showFullScreen()
	{
		pixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
		QWidget::showFullScreen();
	}

	virtual void showNormal()
	{
		pixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
		QWidget::showNormal();
	}

	virtual void mousePressEvent(QMouseEvent *e)
	{
		//std::cout << "mousePressEvent " << e->pos().x() << " " << e->pos().y() << std::endl;

		if (e->button() == Qt::RightButton)
		{
			contextMenu.exec(this->mapToGlobal(e->pos()));
		}
	}

	virtual void mouseMoveEvent(QMouseEvent *e)
	{
		//std::cout << "mouseMoveEvent " << e->pos().x() << " " << e->pos().y() << std::endl;

		if (selectionStarted)
		{
			point2 = e->pos();
			repaint();
		}
	}

	virtual void mouseReleaseEvent(QMouseEvent *e)
	{
		//std::cout << "mouseReleaseEvent " << e->pos().x() << " " << e->pos().y() << std::endl;

		selectionStarted = !selectionStarted;
		if (selectionStarted)
		{
			point1 = point2 = e->pos();
		} 
		else
		{
			QRect rect(point1, point2);
			rect = adjust_rect(rect, e->pos());
			if (rect.width() > 1 && rect.height() > 1)
			{
				saveSlot();
			}
		}
		repaint();
	}

	virtual void paintEvent(QPaintEvent *e)
	{
		QWidget::paintEvent(e);
		QPainter painter(this);

		painter.drawPixmap(this->rect(), pixmap.copy(this->rect()));

		if (selectionStarted)
		{
			QRect rect(point1, point2);
			rect = adjust_rect(rect);
			painter.drawPixmap(rect.topLeft(), pixmap.copy(rect));
			painter.setCompositionMode(QPainter::RasterOp_SourceAndNotDestination);
			QPen inverter(Qt::white);
			inverter.setWidth(5);
			painter.setPen(inverter);
			painter.drawRect(rect);
		}
	}

	virtual void keyPressEvent(QKeyEvent *e)
	{
		if (e->key() != Qt::Key_Escape)
		{
			QWidget::keyPressEvent(e);
		}
		else
		{
			closeSlot();
		}
	}

	void saveSlot()
	{
		//QString filename = QFileDialog::getSaveFileName(this, QObject::tr("Save File"), "../../images/~.png", QObject::tr("Images (*.png)"));
		QString filename("../../images/~.png");
		if (!filename.trimmed().isEmpty())
		{
			QRect rect(point1, point2);
			rect = adjust_rect(rect);
			pixmap.copy(rect).save(filename);
			closeSlot();

			if (Auto_open_selected_image())
			{
				emit region_selected(filename);
			}
		}
	}

	void closeSlot()
	{
		this->close();
	}

private:
	Ui::ScreenshotWidget *ui;

	bool selectionStarted;
	//QRect selectionRect;
	QPoint point1, point2;
	QMenu contextMenu;
	QPixmap pixmap;
	bool auto_open_selected_image;
	QRect adjust_rect(QRect rect)
	{
		std::vector<int> xv, yv;
		xv.push_back(rect.topLeft().x());
		yv.push_back(rect.topLeft().y());
		xv.push_back(rect.bottomRight().x());
		yv.push_back(rect.bottomRight().y());
		std::sort(xv.begin(), xv.end());
		std::sort(yv.begin(), yv.end());
		QRect r(QPoint(xv[0], yv[0]), QPoint(xv[1], yv[1]));
		return r;
	}

	QRect adjust_rect(QRect rect, QPoint p)
	{
		std::vector<int> xv, yv;
		xv.push_back(rect.topLeft().x());
		yv.push_back(rect.topLeft().y());
		xv.push_back(rect.bottomRight().x());
		yv.push_back(rect.bottomRight().y());
		xv.push_back(p.x());
		yv.push_back(p.y());
		std::sort(xv.begin(), xv.end());
		std::sort(yv.begin(), yv.end());
		QRect r(QPoint(xv[0], yv[0]), QPoint(xv[2], yv[2]));
		return r;
	}
};

#endif // SCREENSHOTWIDGET_H
