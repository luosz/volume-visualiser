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

namespace Ui {
	class ScreenshotWidget;
}

class ScreenshotWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ScreenshotWidget(QWidget *parent = 0);
	~ScreenshotWidget();

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
		std::cout << "mousePressEvent " << e->pos().x() << " " << e->pos().y() << std::endl;

		if (e->button() == Qt::RightButton)
		{
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
			selectionRect = get_rect(selectionRect, e->pos());
			repaint();
		}
	}

	virtual void mouseReleaseEvent(QMouseEvent *e)
	{
		std::cout << "mouseReleaseEvent " << e->pos().x() << " " << e->pos().y() << std::endl;
		selectionStarted = false;
		std::cout << "rect " << selectionRect.x() << " " << selectionRect.y() << " " << selectionRect.width() << " " << selectionRect.height() << std::endl;
		selectionRect = get_rect(selectionRect, e->pos());
		repaint();

		if (selectionRect.width() > 0 && selectionRect.height() > 0)
		{
			saveSlot();
		}
	}

	virtual void paintEvent(QPaintEvent *e)
	{
		QWidget::paintEvent(e);
		std::cout << "paintEvent" << std::endl;
		QPainter painter(this);

		painter.drawPixmap(this->rect(), pixmap.copy(this->rect()));

		if (selectionStarted)
		{
			painter.drawPixmap(selectionRect.topLeft(), pixmap.copy(selectionRect));
			painter.setCompositionMode(QPainter::RasterOp_SourceAndNotDestination);
			QPen inverter(Qt::white);
			inverter.setWidth(10);
			painter.setPen(inverter);
			painter.drawRect(selectionRect);
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
		QString fileName = QFileDialog::getSaveFileName(this, QObject::tr("Save File"),
			"../../images/~.png",
			QObject::tr("Images (*.png)"));
		pixmap.copy(selectionRect).save(fileName);
	}

	void closeSlot()
	{
		this->close();
	}

private:
	Ui::ScreenshotWidget *ui;

	bool selectionStarted;
	QRect selectionRect;
	QMenu contextMenu;
	QPixmap pixmap;

	QRect get_rect(QRect rect, QPoint p)
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
