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
	ui->verticalLayout_3->addWidget(&histogramWidget);

	// set up interactor
	interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	interactor->SetRenderWindow(widget.GetRenderWindow());

	// allow the user to interactively manipulate (rotate, pan, etc.) the camera, the viewpoint of the scene.
	auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	interactor->SetInteractorStyle(style);

	// connect signals to slots
	connect(ui->action_Open_Volume, SIGNAL(triggered()), this, SLOT(onOpenVolumeSlot()));
	connect(ui->action_About, SIGNAL(triggered()), this, SLOT(onAboutSlot()));
	connect(ui->action_Exit, SIGNAL(triggered()), this, SLOT(onExitSlot()));
	connect(ui->action_Append_Volume, SIGNAL(triggered()), this, SLOT(onAppendVolumeSlot()));
	connect(ui->action_Load_Transfer_Function, SIGNAL(triggered()), this, SLOT(onLoadTransferFunctionSlot()));

	// Create transfer mapping scalar value to opacity.
	opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	opacityTransferFunction->AddPoint(0.0,  0.0);
	opacityTransferFunction->AddPoint(36.0,  0.125);
	opacityTransferFunction->AddPoint(72.0,  0.25);
	opacityTransferFunction->AddPoint(108.0, 0.375);
	opacityTransferFunction->AddPoint(144.0, 0.5);
	opacityTransferFunction->AddPoint(180.0, 0.625);
	opacityTransferFunction->AddPoint(216.0, 0.75);
	opacityTransferFunction->AddPoint(255.0, 0.875);

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

	volume_filename = "../../data/nucleon.mhd";
	transfer_function_filename = "../../data/nucleon2.tfi";
}

MainWindow::~MainWindow()
{
    delete ui;
}
