#include <QTime>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	// add VTK widgets
	ui->verticalLayout->addWidget(&widget);
	ui->verticalLayout_2->addWidget(&volumePropertywidget);
	//ui->verticalLayout_3->addWidget(&histogramWidget);

	// set up interactor
	interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	interactor->SetRenderWindow(widget.GetRenderWindow());

	// allow the user to interactively manipulate (rotate, pan, etc.) the camera, the viewpoint of the scene.
	auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	interactor->SetInteractorStyle(style);
	interactor->SetStillUpdateRate(1);

	// connect signals to slots
	QObject::connect(ui->action_Open_Volume, SIGNAL(triggered()), this, SLOT(onOpenVolumeSlot()));
	QObject::connect(ui->action_About, SIGNAL(triggered()), this, SLOT(onAboutSlot()));
	QObject::connect(ui->action_Exit, SIGNAL(triggered()), this, SLOT(onExitSlot()));
	QObject::connect(ui->action_Append_Volume, SIGNAL(triggered()), this, SLOT(onAppendVolumeSlot()));
	QObject::connect(ui->action_Open_Transfer_Function, SIGNAL(triggered()), this, SLOT(onOpenTransferFunctionSlot()));
	QObject::connect(ui->action_Save_Transfer_Function, SIGNAL(triggered()), this, SLOT(onSaveTransferFunctionSlot()));
	QObject::connect(ui->action_Optimise_Transfer_Function, SIGNAL(triggered()), this, SLOT(onOptimiseTransferFunctionSlot()));

	// Create transfer mapping scalar value to opacity.
	opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	opacityTransferFunction->AddPoint(0.0,  0.0);
	opacityTransferFunction->AddPoint(36.0,  0.125);
	opacityTransferFunction->AddPoint(72.0,  0.25);
	opacityTransferFunction->AddPoint(108.0, 0.375);
	opacityTransferFunction->AddPoint(144.0, 0.5);
	opacityTransferFunction->AddPoint(180.0, 0.625);
	opacityTransferFunction->AddPoint(216.0, 0.75);
	opacityTransferFunction->AddPoint(255.0, 0.0);

	// Create transfer mapping scalar value to color.
	colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	colorTransferFunction->AddRGBPoint(0.0,  0.0, 0.0, 0.0);
	colorTransferFunction->AddRGBPoint(36.0, 1.0, 0.0, 0.0);
	colorTransferFunction->AddRGBPoint(72.0, 1.0, 1.0, 0.0);
	colorTransferFunction->AddRGBPoint(108.0, 0.0, 1.0, 0.0);
	colorTransferFunction->AddRGBPoint(144.0, 0.0, 1.0, 1.0);
	colorTransferFunction->AddRGBPoint(180.0, 0.0, 0.0, 1.0);
	colorTransferFunction->AddRGBPoint(216.0, 1.0, 0.0, 1.0);
	colorTransferFunction->AddRGBPoint(255.0, 1.0, 1.0, 1.0);

	generateTransferFunction();

	volume_filename = "../../data/nucleon.mhd";
	transfer_function_filename = "../../transferfuncs/nucleon2.tfi";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_optimiseButton_clicked()
{
	updateTransferFunctionArraysFromWidgets();
	optimiseTransferFunction();
	updateTransferFunctionWidgetsFromArrays();
}

void MainWindow::on_pushButton1_clicked()
{
	double height = ui->graphicsView->height();
	QGraphicsScene *scene = ui->graphicsView->scene();
	if (scene == NULL)
	{
		scene = new QGraphicsScene();
		ui->graphicsView->setScene(scene);
		std::cout<<"create scene"<<std::endl;
	}
	scene->clear();
	scene->addText("Frequency " + QTime::currentTime().toString());
	if (frequency_list.size() > 0)
	{
		for (unsigned int itensity=0; itensity<frequency_list.size(); itensity++) // 0 to 255
		{
			auto line = scene->addLine(itensity, height, itensity+1, (1-get_frequency(itensity)/y_max)*height);
			line->setFlag(QGraphicsItem::ItemIsMovable);
		}
	}
}

void MainWindow::on_pushButton2_clicked()
{
	double height = ui->graphicsView->height();
	QGraphicsScene *scene = ui->graphicsView->scene();
	if (scene == NULL)
	{
		scene = new QGraphicsScene();
		ui->graphicsView->setScene(scene);
		std::cout<<"create scene"<<std::endl;
	}
	scene->clear();
	scene->addText("Visibility " + QTime::currentTime().toString());
	if (intensity_list.size() > 0)
	{
		for (unsigned int i=0; i<intensity_list.size(); i++)
		{
			double intensity = denormalise_intensity(intensity_list[i]);
			auto line = scene->addLine(intensity, height, intensity+1, (1-get_opacity(i))*height);
			line->setFlag(QGraphicsItem::ItemIsMovable);
		}
	}
}

void MainWindow::on_pushButton3_clicked()
{
	double height = ui->graphicsView->height();
	QGraphicsScene *scene = ui->graphicsView->scene();
	if (scene == NULL)
	{
		scene = new QGraphicsScene();
		ui->graphicsView->setScene(scene);
		std::cout<<"create scene"<<std::endl;
	}
	scene->clear();
	scene->addText("Visibility " + QTime::currentTime().toString());
	if (intensity_list.size() > 0)
	{
		for (unsigned int i=0; i<intensity_list.size(); i++)
		{
			double intensity = denormalise_intensity(intensity_list[i]);
			auto line = scene->addLine(intensity, height, intensity+1, (1-get_visibility(i)/y_max)*height);
			line->setFlag(QGraphicsItem::ItemIsMovable);
		}
	}
}
