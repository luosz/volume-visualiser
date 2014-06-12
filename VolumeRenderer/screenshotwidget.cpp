#include "screenshotwidget.h"
#include "ui_screenshotwidget.h"

ScreenshotWidget::ScreenshotWidget(QWidget *parent) :
QWidget(parent),
ui(new Ui::ScreenshotWidget)
{
	ui->setupUi(this);

	Selection_started(false);
	Auto_open_selected_image(true);
	Pen_size(3);
	Filename(default_filename());
	QAction *save_action = contextMenu.addAction("&Save as...");
	QAction *close_action = contextMenu.addAction("&Close");
	connect(save_action, SIGNAL(triggered()), this, SLOT(save_slot()));
	connect(close_action, SIGNAL(triggered()), this, SLOT(close_slot()));
}

ScreenshotWidget::~ScreenshotWidget()
{
	delete ui;
}
