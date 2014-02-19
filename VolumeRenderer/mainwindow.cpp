#include <QTime>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "voxel_utility.h"

#ifndef OUTPUT_TO_FILE
#define OUTPUT_TO_FILE
#endif // OUTPUT_TO_FILE

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
	//QObject::connect(ui->action_Open_Volume, SIGNAL(triggered()), this, SLOT(onOpenVolumeSlot()));
	//QObject::connect(ui->action_About, SIGNAL(triggered()), this, SLOT(onAboutSlot()));
	//QObject::connect(ui->action_Exit, SIGNAL(triggered()), this, SLOT(onExitSlot()));
	//QObject::connect(ui->action_Append_Volume, SIGNAL(triggered()), this, SLOT(onAppendVolumeSlot()));
	//QObject::connect(ui->action_Open_Transfer_Function, SIGNAL(triggered()), this, SLOT(onOpenTransferFunctionSlot()));
	//QObject::connect(ui->action_Save_Transfer_Function, SIGNAL(triggered()), this, SLOT(onSaveTransferFunctionSlot()));
	//QObject::connect(ui->action_Open_Selected_Region, SIGNAL(triggered()), this, SLOT(onOpenSelectedRegionSlot()));
	//QObject::connect(ui->action_Compute_Distance, SIGNAL(triggered()), this, SLOT(onComputeDistanceSlot()));
	//QObject::connect(ui->action_Compute_Squared_Distance, SIGNAL(triggered()), this, SLOT(onComputeSquaredDistanceSlot()));
	//QObject::connect(ui->action_Default_Transfer_Function, SIGNAL(triggered()), this, SLOT(onDefaultTransferFunctionSlot()));
	//QObject::connect(ui->action_Spectrum_Transfer_Function, SIGNAL(triggered()), this, SLOT(onSpectrumTransferFunctionSlot()));
	//QObject::connect(ui->action_Open_Path_and_Generate_Transfer_Functions, SIGNAL(triggered()), this, SLOT(on_Open_Path_and_Generate_Transfer_Functions_Slot()));
	//QObject::connect(ui->action_Open_Path_and_Generate_Transfer_Functions_for_Region, SIGNAL(triggered()), this, SLOT(on_Open_Path_and_Generate_Transfer_Functions_for_Region_Slot()));
	//QObject::connect(ui->action_Compute_Distance_HSV, SIGNAL(triggered()), this, SLOT(onComputeDistanceHSVSlot()));
	//QObject::connect(ui->action_Compute_Squared_Distance_HSV, SIGNAL(triggered()), this, SLOT(onComputeSquaredDistanceHSVSlot()));

	// Create transfer mapping scalar value to opacity.
	opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	opacityTransferFunction->AddPoint(0.0, 0.0);
	opacityTransferFunction->AddPoint(36.0, 0.125);
	opacityTransferFunction->AddPoint(72.0, 0.25);
	opacityTransferFunction->AddPoint(108.0, 0.375);
	opacityTransferFunction->AddPoint(144.0, 0.5);
	opacityTransferFunction->AddPoint(180.0, 0.625);
	opacityTransferFunction->AddPoint(216.0, 0.75);
	opacityTransferFunction->AddPoint(255.0, 0.0);

	// Create transfer mapping scalar value to color.
	colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	colorTransferFunction->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
	colorTransferFunction->AddRGBPoint(36.0, 1.0, 0.0, 0.0);
	colorTransferFunction->AddRGBPoint(72.0, 1.0, 1.0, 0.0);
	colorTransferFunction->AddRGBPoint(108.0, 0.0, 1.0, 0.0);
	colorTransferFunction->AddRGBPoint(144.0, 0.0, 1.0, 1.0);
	colorTransferFunction->AddRGBPoint(180.0, 0.0, 0.0, 1.0);
	colorTransferFunction->AddRGBPoint(216.0, 1.0, 0.0, 1.0);
	colorTransferFunction->AddRGBPoint(255.0, 1.0, 1.0, 1.0);

	generate_default_transfer_function();

	volume_filename = "D:/_data/CT-Knee.mhd";
	transfer_function_filename = "../../voreen/CT-Knee.tfi";
	transfer_function_filename_save = "../../voreen/CT-Knee_balance.tfi";
	selected_region_filename = "../../voreen/CT-Knee_selection_only.png";

	//std::cout<<"map to range test "<<map_to_range(0.5, 0, 1, 0, 255)<<" "<<map_to_range(192, 0, 255, 0, 1)<<" "<<map_to_range(0.6, 0.5, 1, 128, 255)<<std::endl;

	// should not be used before initialization
	count_of_voxels = 0;
	volume_ptr = NULL;
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_frequencyButton_clicked()
{
	double height = ui->graphicsView->height();
	QGraphicsScene *scene = getGraphicsScene();
	scene->clear();
	scene->addText("Frequency " + QTime::currentTime().toString());
	if (frequency_list.size() > 0)
	{
		for (unsigned int itensity = 0; itensity < frequency_list.size(); itensity++) // 0 to 255
		{
			auto line = scene->addLine(itensity, height, itensity + 1, (1 - get_frequency(itensity) / y_max)*height);
			line->setFlag(QGraphicsItem::ItemIsMovable);
		}
	}
}

void MainWindow::on_opacityButton_clicked()
{
	double height = ui->graphicsView->height();
	QGraphicsScene *scene = getGraphicsScene();
	scene->clear();
	scene->addText("Visibility " + QTime::currentTime().toString());
	if (intensity_list.size() > 0)
	{
		for (unsigned int i = 0; i < intensity_list.size(); i++)
		{
			double intensity = denormalise_intensity(intensity_list[i]);
			auto line = scene->addLine(intensity, height, intensity + 1, (1 - get_opacity(i))*height);
			line->setFlag(QGraphicsItem::ItemIsMovable);
		}
	}
}

void MainWindow::on_visibilityButton_clicked()
{
	double height = ui->graphicsView->height();
	QGraphicsScene *scene = getGraphicsScene();
	scene->clear();
	scene->addText("Visibility " + QTime::currentTime().toString());
	if (intensity_list.size() > 0)
	{
		for (unsigned int i = 0; i < intensity_list.size(); i++)
		{
			double intensity = denormalise_intensity(intensity_list[i]);
			auto line = scene->addLine(intensity, height, intensity + 1, (1 - get_visibility(i) * 16)*height);
			line->setFlag(QGraphicsItem::ItemIsMovable);
			std::cout << "visibility=" << get_visibility(i) << " ymax=" << y_max << std::endl;
		}
	}
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
		//std::cout<<"entropy ";
		for (unsigned int intensity = 0; intensity < frequency_list.size(); intensity++) // 0 to 255
		{
			double entropy = get_entropy(intensity);
			//std::cout<<" "<<entropy;
			auto line = scene->addLine(intensity, height, intensity + 1, (1 - entropy * 10)*height);
			line->setFlag(QGraphicsItem::ItemIsMovable);

		}
		//std::cout<<std::endl;
	}
}

void MainWindow::on_entropyOpacityButton_clicked()
{
	double height = ui->graphicsView->height();
	QGraphicsScene *scene = getGraphicsScene();
	scene->clear();
	scene->addText("Entropy Opacity " + QTime::currentTime().toString());
	if (frequency_list.size() > 0)
	{
		unsigned int index = -1;
		//std::cout<<"entropy ";
		for (unsigned int intensity = 0; intensity < frequency_list.size(); intensity++) // 0 to 255
		{
			double entropy = get_entropy_opacity(intensity);
			//std::cout<<" "<<entropy;
			auto line = scene->addLine(intensity, height, intensity + 1, (1 - entropy * 16)*height);
			line->setFlag(QGraphicsItem::ItemIsMovable);

		}
		//std::cout<<std::endl;
	}
}

void MainWindow::on_balanceButton_clicked()
{
	updateTransferFunctionArraysFromWidgets();
	int n = ui->spinBox->value();
	if (n < 1 || n > max_iteration_count)
	{
		n = 1;
	}
	while (n-- > 0)
	{
		balance_transfer_function();
	}
	updateTransferFunctionWidgetsFromArrays();
	updateTransferFunctionArraysFromWidgets();
}

void MainWindow::on_lhHistogramButton_clicked()
{
	int size[3] = { 41, 41, 41 };
	get_index(0, 0, 0, size);
}

void MainWindow::on_reduceOpacityButton_clicked()
{
	updateTransferFunctionArraysFromWidgets();
	int n = ui->spinBox->value();
	if (n < 1 || n > max_iteration_count)
	{
		n = 1;
	}
	while (n-- > 0)
	{
		reduce_opacity();
	}
	updateTransferFunctionWidgetsFromArrays();
	updateTransferFunctionArraysFromWidgets();
}

void MainWindow::on_increaseOpacityButton_clicked()
{
	updateTransferFunctionArraysFromWidgets();
	int n = ui->spinBox->value();
	if (n < 1 || n > max_iteration_count)
	{
		n = 1;
	}
	while (n-- > 0)
	{
		increase_opacity();
	}
	updateTransferFunctionWidgetsFromArrays();
	updateTransferFunctionArraysFromWidgets();
}

void MainWindow::on_balanceOpacityButton_clicked()
{
	updateTransferFunctionArraysFromWidgets();
	int n = ui->spinBox->value();
	if (n < 1 || n > max_iteration_count)
	{
		n = 1;
	}
#ifdef OUTPUT_TO_FILE
	char filename[32] = "../energy_function.csv";
	std::cout << "energy function file " << filename << std::endl;
	std::ofstream out(filename);
	int iteration_count = 0;
#endif
	while (n-- > 0)
	{
#ifdef OUTPUT_TO_FILE
		out << iteration_count << "," << get_energy_function() << std::endl;
		iteration_count++;
#endif
		balance_opacity();
	}
#ifdef OUTPUT_TO_FILE
	out << iteration_count << "," << get_energy_function() << std::endl;
	out.close();
#endif
	updateTransferFunctionWidgetsFromArrays();
	updateTransferFunctionArraysFromWidgets();
}

void MainWindow::on_enhanceRegionButton_clicked()
{
	updateTransferFunctionArraysFromWidgets();
	int n = ui->spinBox->value();
	if (n < 1 || n > max_iteration_count)
	{
		n = 1;
	}
	while (n-- > 0)
	{
		increase_opacity_for_region();
	}
	updateTransferFunctionWidgetsFromArrays();
	updateTransferFunctionArraysFromWidgets();
}

void MainWindow::on_weakenRegionButton_clicked()
{
	updateTransferFunctionArraysFromWidgets();
	int n = ui->spinBox->value();
	if (n < 1 || n > max_iteration_count)
	{
		n = 1;
	}
	while (n-- > 0)
	{
		reduce_opacity_for_region();
	}
	updateTransferFunctionWidgetsFromArrays();
	updateTransferFunctionArraysFromWidgets();
}

void MainWindow::on_balanceRegionButton_clicked()
{
	updateTransferFunctionArraysFromWidgets();
	int n = ui->spinBox->value();
	if (n < 1 || n > max_iteration_count)
	{
		n = 1;
	}
#ifdef OUTPUT_TO_FILE
	char filename[32] = "../energy_function.csv";
	std::cout << "energy function file " << filename << std::endl;
	std::ofstream out(filename);
	char filename2[32] = "../energy_function_region.csv";
	std::cout << "energy function (region) file " << filename2 << std::endl;
	std::ofstream out2(filename2);
	int iteration_count = 0;
#endif
	while (n-- > 0)
	{
#ifdef OUTPUT_TO_FILE
		out << iteration_count << "," << get_energy_function() << std::endl;
		out2 << iteration_count << "," << get_energy_function_weighted_for_region() << std::endl;
		iteration_count++;
#endif
		balance_opacity_for_region();
	}
#ifdef OUTPUT_TO_FILE
	out << iteration_count << "," << get_energy_function() << std::endl;
	out.close();
	out2 << iteration_count << "," << get_energy_function_weighted_for_region() << std::endl;
	out2.close();
#endif
	updateTransferFunctionWidgetsFromArrays();
	updateTransferFunctionArraysFromWidgets();
}

void MainWindow::on_action_About_triggered()
{
	QMessageBox msgBox;
	msgBox.setText(QString::fromUtf8("Volume Renderer"));
	msgBox.exec();
}

void MainWindow::on_action_Exit_triggered()
{
	qApp->quit();
}

void MainWindow::on_action_Open_Volume_triggered()
{
	// show file dialog
	QString filter("Meta image file (*.mhd *.mha)");
	QString filename_backup = volume_filename;
	filename_backup = QFileDialog::getOpenFileName(this, QString(tr("Open a volume data set")), filename_backup, filter);
	if (!volume_filename.trimmed().isEmpty())
	{
		volume_filename = filename_backup;
	}
	else
	{
		return;
	}

	// show filename on window title
	this->setWindowTitle(QString::fromUtf8("Volume Renderer - ") + filename_backup);

	// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
	QByteArray ba = filename_backup.toLocal8Bit();
	const char *filename_str = ba.data();

#if 1
	// read Meta Image (.mhd or .mha) files
	auto reader = vtkSmartPointer<vtkMetaImageReader>::New();
	reader->SetFileName(filename_str);

	//// write Meta Image to file (convert from .mha to .mhd)
	//auto writer = vtkSmartPointer<vtkMetaImageWriter>::New();
	//std::cout << "filename=" << filename_str << std::endl;
	//char mhd_str[_MAX_PATH];
	//char raw_str[_MAX_PATH];
	//strcpy(mhd_str, filename_str);
	//strcpy(raw_str, filename_str);
	//auto p1 = strstr(mhd_str, ".mha");
	//auto p2 = strstr(raw_str, ".mha");
	//if (p1 && p2)
	//{
	//	strcpy(p1, ".mhd");
	//	strcpy(p2, ".raw");
	//	std::cout << p1 << "\n" << p2 << std::endl;
	//	std::cout << mhd_str << "\n" << raw_str << std::endl;
	//	writer->SetFileName(mhd_str);
	//	writer->SetRAWFileName(raw_str);
	//	writer->SetInputConnection(reader->GetOutputPort());
	//	writer->Write();
	//}
#elif 1
	// read a series of raw files in the specified folder
	auto reader = vtkSmartPointer<vtkVolume16Reader>::New();
	reader->SetDataDimensions(512, 512);
	reader->SetImageRange(1, 361);
	reader->SetDataByteOrderToBigEndian();
	reader->SetFilePrefix(filename_str);
	reader->SetFilePattern("%s%d");
	reader->SetDataSpacing(1, 1, 1);
#else
	// read NRRD files
	vtkNew<vtkNrrdReader> reader;
	if (!reader->CanReadFile(filename_str))
	{
		std::cerr << "Reader reports " << filename_str << " cannot be read.";
		exit(EXIT_FAILURE);
	}
	reader->SetFileName(filename_str);
	reader->Update();
#endif

	// scale the volume data to unsigned char (0-255) before passing it to volume mapper
	auto shiftScale = vtkSmartPointer<vtkImageShiftScale>::New();
	shiftScale->SetInputConnection(reader->GetOutputPort());
	shiftScale->SetOutputScalarTypeToUnsignedChar();

	// generate histograms
	generate_visibility_function(shiftScale);
	generate_LH_histogram(shiftScale);

	//// Create transfer mapping scalar value to opacity.
	//auto opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	//opacityTransferFunction->AddPoint(0.0,  0.0);
	//opacityTransferFunction->AddPoint(36.0,  0.125);
	//opacityTransferFunction->AddPoint(72.0,  0.25);
	//opacityTransferFunction->AddPoint(108.0, 0.375);
	//opacityTransferFunction->AddPoint(144.0, 0.5);
	//opacityTransferFunction->AddPoint(180.0, 0.625);
	//opacityTransferFunction->AddPoint(216.0, 0.75);
	//opacityTransferFunction->AddPoint(255.0, 0.875);

	//// Create transfer mapping scalar value to color.
	//auto colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	//colorTransferFunction->AddRGBPoint(0.0,  0.0, 0.0, 0.0);
	//colorTransferFunction->AddRGBPoint(36.0, 1.0, 0.0, 0.0);
	//colorTransferFunction->AddRGBPoint(72.0, 1.0, 1.0, 0.0);
	//colorTransferFunction->AddRGBPoint(108.0, 0.0, 1.0, 0.0);
	//colorTransferFunction->AddRGBPoint(144.0, 0.0, 1.0, 1.0);
	//colorTransferFunction->AddRGBPoint(180.0, 0.0, 0.0, 1.0);
	//colorTransferFunction->AddRGBPoint(216.0, 1.0, 0.0, 1.0);
	//colorTransferFunction->AddRGBPoint(255.0, 1.0, 1.0, 1.0);

	// set up volume property
	auto volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->SetColor(colorTransferFunction);
	volumeProperty->SetScalarOpacity(opacityTransferFunction);
	volumeProperty->ShadeOff();
	volumeProperty->SetInterpolationTypeToLinear();

	// assign volume property to the volume property widget
	volumePropertywidget.setVolumeProperty(volumeProperty);

	// The mapper that renders the volume data.
	auto volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	volumeMapper->SetRequestedRenderMode(vtkSmartVolumeMapper::GPURenderMode);
	volumeMapper->SetInputConnection(shiftScale->GetOutputPort());

	// The volume holds the mapper and the property and can be used to position/orient the volume.
	auto volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(volumeMapper);
	volume->SetProperty(volumeProperty);

	// add the volume into the renderer
	//auto renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->AddVolume(volume);
	renderer->SetBackground(1, 1, 1);

	// clean previous renderers and then add the current renderer
	auto window = widget.GetRenderWindow();
	auto collection = window->GetRenderers();
	auto item = collection->GetNextItem();
	while (item != NULL)
	{
		window->RemoveRenderer(item);
		item = collection->GetNextItem();
	}
	window->AddRenderer(renderer);
	window->Render();

	// initialize the interactor
	interactor->Initialize();
	interactor->Start();

	//generateHistogram(reader);
}

void MainWindow::on_action_Append_Volume_triggered()
{
	// show file dialog
	QString filter("Meta image file (*.mhd *.mha)");
	volume_filename = QFileDialog::getOpenFileName(this, QString(tr("Open a volume data set")), volume_filename, filter);
	if (volume_filename.isEmpty())
	{
		return;
	}

	// show filename on window title
	this->setWindowTitle(QString::fromUtf8("Volume Renderer - ") + volume_filename);

	// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
	QByteArray ba = volume_filename.toLocal8Bit();
	const char *filename_str = ba.data();

#if 1
	// read Meta Image (.mhd or .mha) files
	auto reader = vtkSmartPointer<vtkMetaImageReader>::New();
	reader->SetFileName(filename_str);
#elif 1
	// read a series of raw files in the specified folder
	auto reader = vtkSmartPointer<vtkVolume16Reader>::New();
	reader->SetDataDimensions(512, 512);
	reader->SetImageRange(1, 361);
	reader->SetDataByteOrderToBigEndian();
	reader->SetFilePrefix(filename_str);
	reader->SetFilePattern("%s%d");
	reader->SetDataSpacing(1, 1, 1);
#else
	// read NRRD files
	vtkNew<vtkNrrdReader> reader;
	if (!reader->CanReadFile(filename_str))
	{
		std::cerr << "Reader reports " << filename_str << " cannot be read.";
		exit(EXIT_FAILURE);
	}
	reader->SetFileName(filename_str);
	reader->Update();
#endif

	// scale the volume data to unsigned char (0-255) before passing it to volume mapper
	auto shiftScale = vtkSmartPointer<vtkImageShiftScale>::New();
	shiftScale->SetInputConnection(reader->GetOutputPort());
	shiftScale->SetOutputScalarTypeToUnsignedChar();

	// get existing volumeProperty from volumePropertywidget
	auto volumeProperty = volumePropertywidget.volumeProperty();

	// The mapper that renders the volume data.
	auto volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	volumeMapper->SetRequestedRenderMode(vtkSmartVolumeMapper::GPURenderMode);
	volumeMapper->SetInputConnection(shiftScale->GetOutputPort());

	// The volume holds the mapper and the property and can be used to position/orient the volume.
	auto volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(volumeMapper);
	volume->SetProperty(volumeProperty);

	// add the volume into the renderer
	renderer->AddVolume(volume);

	auto window = widget.GetRenderWindow();
	window->Render();

	// initialize the interactor
	interactor->Initialize();
	interactor->Start();
}

void MainWindow::on_action_Open_Transfer_Function_triggered()
{
	// show file dialog
	QString filter("Voreen transfer function (*.tfi)");
	QString filename_backup = transfer_function_filename;
	filename_backup = QFileDialog::getOpenFileName(this, QString(tr("Open a transfer function")), filename_backup, filter);
	if (!filename_backup.trimmed().isEmpty())
	{
		transfer_function_filename = filename_backup;
	}
	else
	{
		return;
	}

	//// show filename on window title
	//this->setWindowTitle(QString::fromUtf8("Volume Renderer - ") + volume_filename);

	// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
	QByteArray ba = filename_backup.toLocal8Bit();
	const char *filename_str = ba.data();

	std::cout << "transfer function file: " << filename_str << endl;

	openTransferFunctionFromXML(filename_str);
	updateTransferFunctionWidgetsFromArrays();
}

void MainWindow::on_action_Save_Transfer_Function_triggered()
{
	// show file dialog
	QString filter("transfer function file (*.tfi)");
	QString filename_backup = transfer_function_filename_save;
	filename_backup = QFileDialog::getSaveFileName(this, QString(tr("Save transfer function as")), filename_backup, filter);
	if (!filename_backup.trimmed().isEmpty())
	{
		transfer_function_filename_save = filename_backup;
	}
	else
	{
		return;
	}

	//// show filename on window title
	//this->setWindowTitle(QString::fromUtf8("Volume Renderer - ") + volume_filename);

	// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
	QByteArray ba = filename_backup.toLocal8Bit();
	const char *filename_str = ba.data();

	std::cout << "transfer function file: " << filename_str << endl;

	saveTransferFunctionToXML(filename_str);
	//updateTransferFunction();
}

void MainWindow::on_action_Open_Selected_Region_triggered()
{
	// show file dialog
	QString filter("PNG image (*.png)");
	QString filename_backup = selected_region_filename;
	filename_backup = QFileDialog::getOpenFileName(this, QString(tr("Open a PNG image")), filename_backup, filter);
	if (!filename_backup.trimmed().isEmpty())
	{
		selected_region_filename = filename_backup;
	}
	else
	{
		return;
	}

	read_region_image_and_compute_distance(1, 1);
}

void MainWindow::on_action_Compute_Squared_Distance_triggered()
{
	std::cout << "squared distance (RGB) is chosen" << std::endl;
	read_region_image_and_compute_distance(1);
}

void MainWindow::on_action_Compute_Distance_triggered()
{
	std::cout << "distance (RGB) is chosen" << std::endl;
	read_region_image_and_compute_distance();

}

void MainWindow::on_action_Compute_Squared_Distance_HSV_triggered()
{
	std::cout << "squared distance (HSV) is chosen" << std::endl;
	read_region_image_and_compute_distance(1, 1);
}

void MainWindow::on_action_Compute_Distance_HSV_triggered()
{
	std::cout << "distance (HSV) is chosen" << std::endl;
	read_region_image_and_compute_distance(0, 1);
}

void MainWindow::on_action_Default_Transfer_Function_triggered()
{
	generate_default_transfer_function();
	updateTransferFunctionWidgetsFromArrays();
}

void MainWindow::on_action_Spectrum_Transfer_Function_triggered()
{
	bool ok;
	int n = QInputDialog::getInt(this, tr("QInputDialog::getInteger()"), tr("Number of colours [1,256]:"), 8, 1, 256, 1, &ok);
	if (ok)
	{
		std::cout << "QInputDialog::getInteger() " << n << std::endl;
		generate_spectrum_transfer_function(n);
		updateTransferFunctionWidgetsFromArrays();
	}
}

void MainWindow::on_action_Open_Path_and_Generate_Transfer_Functions_triggered()
{
	bool ok;
	QString path = QInputDialog::getText(this, tr("QInputDialog::getText()"),
		tr("Path to open:"), QLineEdit::Normal,
		"D:/output/vortex/", &ok);
	if (ok && !path.isEmpty())
	{
		// get filename separator position
		// a valid path must contain either / or \\. if not, it's invalid
		int index = path.lastIndexOf("/");
		if (index == -1)
		{
			index = path.lastIndexOf("\\");
		}

		// a valid path must contain either / or \\. otherwise it's invalid.
		// split filename and path
		QString filepath = path;
		QString filename;
		if (index != -1)
		{
			filepath = path.left(index + 1);
			filename = path.right(path.length() - 1 - index);
			if (filename.isEmpty())
			{
				filename = "*.mhd";
			}
		}
		else
		{
			QMessageBox msgBox;
			msgBox.setText("Invalid path! Please put either a \\ or a / to the end of the path");
			int ret = msgBox.exec();
			return;
		}

		// get filenames under the folder
		QDir dir = QDir(filepath);
		QStringList files = dir.entryList(QStringList(filename),
			QDir::Files | QDir::NoSymLinks);

		// spectrum number for transfer function generation
		const int n = 6;

		for (int i = 0; i < files.size(); i++)
		{
			// load volume
			open_volume_no_rendering(filepath + files[i]);

			// generate a spectrum transfer function with n groups of control points
			generate_spectrum_transfer_function(n);
			updateTransferFunctionWidgetsFromArrays();

			// optimise the transfer function
			updateTransferFunctionArraysFromWidgets();
			int n = ui->spinBox->value();
			if (n < 1 || n > max_iteration_count)
			{
				n = 1;
			}

			while (n-- > 0)
			{
				balance_opacity();
			}

			updateTransferFunctionWidgetsFromArrays();
			updateTransferFunctionArraysFromWidgets();

			// split filename and extension
			QStringList list1 = files[i].split(".", QString::SkipEmptyParts);

			// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters)
			QByteArray ba = list1[0].toLocal8Bit();
			const char *filename_no_suffix = ba.data();

			QByteArray ba1 = filepath.toLocal8Bit();
			const char *path1 = ba1.data();

			// save the transfer function to file
			char filename_str[_MAX_PATH];
			sprintf(filename_str, "%s%s.tfi", path1, filename_no_suffix);
			std::cout << "transfer function file: " << filename_str << endl;
			saveTransferFunctionToXML(filename_str);
		}
	}
}

void MainWindow::on_action_Open_Path_and_Generate_Transfer_Functions_for_Region_triggered()
{
	bool ok;
	QString path = QInputDialog::getText(this, tr("QInputDialog::getText()"),
		tr("Path to open:"), QLineEdit::Normal,
		"D:/output/vortex/", &ok);
	if (ok && !path.isEmpty())
	{
		// get filename separator position
		int index = path.lastIndexOf("/");
		if (index == -1)
		{
			index = path.lastIndexOf("\\");
		}

		// a valid path must contain either / or \\. otherwise it's invalid.
		// split filename and path
		QString filepath = path;
		QString filename;
		if (index != -1)
		{
			filepath = path.left(index + 1);
			filename = path.right(path.length() - 1 - index);
			if (filename.isEmpty())
			{
				filename = "*.mhd";
			}
		}
		else
		{
			QMessageBox msgBox;
			msgBox.setText("Invalid path! Please put either a \\ or a / to the end of the path");
			int ret = msgBox.exec();
			return;
		}

		// get filenames under the folder
		QDir dir = QDir(filepath);
		QStringList files = dir.entryList(QStringList(filename),
			QDir::Files | QDir::NoSymLinks);

		// spectrum number for transfer function generation
		const int n = 6;

		for (int i = 0; i < files.size(); i++)
		{
			// load volume
			open_volume_no_rendering(filepath + files[i]);

			// generate a spectrum transfer function with n groups of control points
			generate_spectrum_transfer_function(n);
			updateTransferFunctionWidgetsFromArrays();

			// compute region-based difference factors
			read_region_image_and_compute_distance(1, 1);

			// optimise the transfer function
			updateTransferFunctionArraysFromWidgets();
			int n = ui->spinBox->value();
			if (n < 1 || n > max_iteration_count)
			{
				n = 1;
			}

			while (n-- > 0)
			{
				balance_opacity_for_region();
			}

			updateTransferFunctionWidgetsFromArrays();
			updateTransferFunctionArraysFromWidgets();

			// split filename and extension
			QStringList list1 = files[i].split(".", QString::SkipEmptyParts);

			// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters)
			QByteArray ba = list1[0].toLocal8Bit();
			const char *filename_no_suffix = ba.data();

			QByteArray ba1 = filepath.toLocal8Bit();
			const char *path1 = ba1.data();

			// save the transfer function to file
			char filename_str[_MAX_PATH];
			sprintf(filename_str, "%s%s.tfi", path1, filename_no_suffix);
			std::cout << "transfer function file: " << filename_str << endl;
			saveTransferFunctionToXML(filename_str);
		}
	}
}
