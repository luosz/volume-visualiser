#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
QWidget(parent),
ui(new Ui::Widget)
{
	ui->setupUi(this);

	selectionStarted = false;
	QAction *saveAction = contextMenu.addAction("Save");
	QAction *closeAction = contextMenu.addAction("Close");
	connect(saveAction, SIGNAL(triggered()), this, SLOT(saveSlot()));
	connect(closeAction, SIGNAL(triggered()), this, SLOT(closeSlot()));

	ui->label->setVisible(false);
}

Widget::~Widget()
{
	delete ui;
}
