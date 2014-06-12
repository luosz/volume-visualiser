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
		capture();
		QWidget::show();
	}

	virtual void showMinimized()
	{
		capture();
		QWidget::showMinimized();
	}

	virtual void showMaximized()
	{
		capture();
		QWidget::showMaximized();
	}

	virtual void showFullScreen()
	{
		capture();
		QWidget::showFullScreen();
	}

	virtual void showNormal()
	{
		capture();
		QWidget::showNormal();
	}

	virtual void mousePressEvent(QMouseEvent *e)
	{
		//std::cout << "mousePressEvent " << e->pos().x() << " " << e->pos().y() << std::endl;
		if (e->button() == Qt::RightButton)
		{
			contextMenu.exec(this->mapToGlobal(e->pos()));
		}
		else
		{
			if (e->button() == Qt::LeftButton)
			{
				Selection_started(!Selection_started());
				if (Selection_started())
				{
					point1 = point2 = e->pos();
				}
				else
				{
					QRect rect(point1, point2);
					rect = get_valid_rect(rect, e->pos());
					if (rect.width() > 1 || rect.height() > 1)
					{
						save(default_filename());
						close_slot();
					}
				}
				repaint();
			}
		}
	}

	virtual void mouseMoveEvent(QMouseEvent *e)
	{
		//std::cout << "mouseMoveEvent " << e->pos().x() << " " << e->pos().y() << std::endl;
		point2 = e->pos();
		repaint();
	}

	virtual void mouseReleaseEvent(QMouseEvent *e)
	{
		//std::cout << "mouseReleaseEvent " << e->pos().x() << " " << e->pos().y() << std::endl;
	}

	virtual void paintEvent(QPaintEvent *e)
	{
		QWidget::paintEvent(e);

		QPainter painter(this);
		painter.drawPixmap(this->rect(), pixmap.copy(this->rect()));
		QPen inverter(Qt::white);
		inverter.setWidth(Pen_size());

		if (Selection_started())
		{
			QRect rect(point1, point2);
			rect = get_valid_rect(rect);
			painter.drawPixmap(rect.topLeft(), pixmap.copy(rect));
			painter.setCompositionMode(QPainter::RasterOp_SourceAndNotDestination);
			painter.setPen(inverter);
			painter.drawRect(rect);
		}
		else
		{
			painter.setCompositionMode(QPainter::RasterOp_SourceAndNotDestination);
			painter.setPen(inverter);
			painter.drawLine(0, point2.y(), this->width(), point2.y());
			painter.drawLine(point2.x(), 0, point2.x(), this->height());
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
			close_slot();
		}
	}

	void save_slot()
	{
		QString filename_backup = filename;
		filename_backup = QFileDialog::getSaveFileName(this, QObject::tr("Save File"), filename_backup, QObject::tr("Images (*.png)"));
		if (!filename_backup.trimmed().isEmpty())
		{
			filename = filename_backup;
		}
		else
		{
			return;
		}

		save(filename_backup);
	}

	void save(QString target_filename)
	{
		QRect rect(point1, point2);
		rect = get_valid_rect(rect);
		pixmap.copy(rect).save(target_filename);
		if (Auto_open_selected_image())
		{
			emit region_selected(target_filename);
		}
	}

	void close_slot()
	{
		this->close();
	}

private:
	Ui::ScreenshotWidget *ui;

	QPoint point1, point2;
	QMenu contextMenu;
	QPixmap pixmap;
	bool auto_open_selected_image;
	bool selection_started;
	bool Selection_started() const { return selection_started; }
	void Selection_started(bool val) { selection_started = val; }
	QString filename;
	QString Filename() const { return filename; }
	void Filename(QString val) { filename = val; }
	int pen_size;
	int Pen_size() const { return pen_size; }
	void Pen_size(int val) { pen_size = val; }

	QRect get_valid_rect(QRect rect)
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

	QRect get_valid_rect(QRect rect, QPoint p)
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

	QString default_filename()
	{
		return QString("../../images/~.png");
	}

	void capture()
	{
		pixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
	}
};

#endif // SCREENSHOTWIDGET_H
