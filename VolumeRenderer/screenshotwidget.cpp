#include "screenshotwidget.h"
#include "ui_screenshotwidget.h"

ScreenshotWidget::ScreenshotWidget(QWidget *parent) :
QWidget(parent),
ui(new Ui::ScreenshotWidget)
{
	ui->setupUi(this);

	selectionStarted = false;
	QAction *saveAction = contextMenu.addAction("&Save");
	QAction *closeAction = contextMenu.addAction("&Close");
	connect(saveAction, SIGNAL(triggered()), this, SLOT(saveSlot()));
	connect(closeAction, SIGNAL(triggered()), this, SLOT(closeSlot()));
}

ScreenshotWidget::~ScreenshotWidget()
{
	delete ui;
}
