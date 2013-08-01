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
	QObject::connect(ui->action_Optimise_Transfer_Function, SIGNAL(triggered()), this, SLOT(on_optimiseButton_clicked()));

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

	generateDefaultTransferFunction();

	volume_filename = "../../data/nucleon.mhd";
	transfer_function_filename = "../../transferfuncs/nucleon2.tfi";

	//std::cout<<"map to range test "<<map_to_range(0.5, 0, 1, 0, 255)<<" "<<map_to_range(192, 0, 255, 0, 1)<<" "<<map_to_range(0.6, 0.5, 1, 128, 255)<<std::endl;

	// should not be used before initialization
	count_of_voxels = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_optimiseButton_clicked()
{
	updateTransferFunctionArraysFromWidgets();
	int n = ui->spinBox->value();
	if (n < 1 || n > 99)
	{
		n = 1;
	}
	while (n-- > 0)
	{
		optimiseTransferFunction();
	}
	updateTransferFunctionWidgetsFromArrays();
	updateTransferFunctionArraysFromWidgets();
}

void MainWindow::on_pushButton1_clicked()
{
	double height = ui->graphicsView->height();
	QGraphicsScene *scene = getGraphicsScene();
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
	QGraphicsScene *scene = getGraphicsScene();
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
	QGraphicsScene *scene = getGraphicsScene();
	scene->clear();
	scene->addText("Visibility " + QTime::currentTime().toString());
	if (intensity_list.size() > 0)
	{
		for (unsigned int i=0; i<intensity_list.size(); i++)
		{
			double intensity = denormalise_intensity(intensity_list[i]);
			auto line = scene->addLine(intensity, height, intensity+1, (1-get_visibility(i)*10)*height);
			line->setFlag(QGraphicsItem::ItemIsMovable);
			std::cout<<"visibility="<<get_visibility(i)<<" ymax="<<y_max<<std::endl;
		}
	}
}

void MainWindow::on_updateButton_clicked()
{
	updateTransferFunctionArraysFromWidgets();
	updateTransferFunctionWidgetsFromArrays();
	std::cout<<"area by integral "<<get_area_integral(0)<<std::endl;
	std::cout<<"area by integral "<<get_area_integral(1)<<std::endl;
	std::cout<<"area by integral "<<get_area_integral(2)<<std::endl;
	std::cout<<"area by integral "<<get_area_integral(3)<<std::endl;
	std::cout<<"area by integral "<<get_area_integral(4)<<std::endl;
	std::cout<<"area by integral "<<get_area_integral(5)<<std::endl;
	std::cout<<"area by integral "<<get_area_integral(6)<<std::endl;
}

void MainWindow::on_defaultButton_clicked()
{
	generateDefaultTransferFunction();
	updateTransferFunctionWidgetsFromArrays();
}

void MainWindow::on_entropyButton_clicked()
{
	double height = ui->graphicsView->height();
	QGraphicsScene *scene = getGraphicsScene();
	scene->clear();
	scene->addText("Entropy " + QTime::currentTime().toString());
	if (frequency_list.size() > 0)
	{
		unsigned int index = -1;
		std::cout<<"entropy "<<get_entropy(10, 0);
		for (unsigned int intensity=0; intensity<frequency_list.size(); intensity++) // 0 to 255
		{
			double entropy = get_entropy(intensity);
			std::cout<<" "<<entropy;
			auto line = scene->addLine(intensity, height, intensity+1, (1-entropy*10)*height);
			line->setFlag(QGraphicsItem::ItemIsMovable);
	
		}
		std::cout<<std::endl;
	}
}
