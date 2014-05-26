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
	ui->verticalLayout->addWidget(&vtk_widget);
	ui->verticalLayout_2->addWidget(&volume_property_widget);

	// set up interactor
	interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	interactor->SetRenderWindow(vtk_widget.GetRenderWindow());

	// allow the user to interactively manipulate (rotate, pan, etc.) the camera, the viewpoint of the scene.
	auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	interactor->SetInteractorStyle(style);
	interactor->SetStillUpdateRate(1);

	// create gradient scalar mapping
	gradient_opacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	gradient_opacity->AddPoint(0, 1);
	gradient_opacity->AddPoint(1, 1);

	// Create transfer mapping scalar value to opacity.
	scalar_opacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	scalar_opacity->AddPoint(0.0, 0.0);
	scalar_opacity->AddPoint(36.0, 0.125);
	scalar_opacity->AddPoint(72.0, 0.25);
	scalar_opacity->AddPoint(108.0, 0.375);
	scalar_opacity->AddPoint(144.0, 0.5);
	scalar_opacity->AddPoint(180.0, 0.625);
	scalar_opacity->AddPoint(216.0, 0.75);
	scalar_opacity->AddPoint(255.0, 0.0);

	// Create transfer mapping scalar value to color.
	scalar_color = vtkSmartPointer<vtkColorTransferFunction>::New();
	scalar_color->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
	scalar_color->AddRGBPoint(36.0, 1.0, 0.0, 0.0);
	scalar_color->AddRGBPoint(72.0, 1.0, 1.0, 0.0);
	scalar_color->AddRGBPoint(108.0, 0.0, 1.0, 0.0);
	scalar_color->AddRGBPoint(144.0, 0.0, 1.0, 1.0);
	scalar_color->AddRGBPoint(180.0, 0.0, 0.0, 1.0);
	scalar_color->AddRGBPoint(216.0, 1.0, 0.0, 1.0);
	scalar_color->AddRGBPoint(255.0, 1.0, 1.0, 1.0);

	generate_default_transfer_function();

	volume_filename = "../../data/CT-Knee.mhd";
	transfer_function_filename = "../../transferfuncs/CT-AAA.xml";
	transfer_function_filename_save = "../../transfer_function/save_as.tfi";
	selected_region_filename = "../../voreen/CT-Knee_spectrum_6_balance_1000_selection_only.png";

	//std::cout<<"map to range test "<<map_to_range(0.5, 0, 1, 0, 255)<<" "<<map_to_range(192, 0, 255, 0, 1)<<" "<<map_to_range(0.6, 0.5, 1, 128, 255)<<std::endl;

	////////////////////////////////////////////////////////////////////////////
	//// colour test
	//std::cout << "HSV and RGB colour conversion test" << std::endl;

	//double h, s, v, r, g, b;
	//QColor c;
	//c = Qt::black;
	//std::cout << "black rgb " << c.red() << " " << c.green() << " " << c.blue() << " hsv " << c.hue() << " " << c.saturation() << " " << c.value() << "\t";

	//r = c.red();
	//g = c.green();
	//b = c.blue();
	//vtkMath::RGBToHSV(normalise_rgba(r), normalise_rgba(g), normalise_rgba(b), &h, &s, &v);
	//std::cout << "after vtkMath::RGBToHSV " << h << " " << s << " " << v << std::endl;

	//c = Qt::white;
	//std::cout << "white rgb " << c.red() << " " << c.green() << " " << c.blue() << " hsv " << c.hue() << " " << c.saturation() << " " << c.value() << "\t";

	//r = c.red();
	//g = c.green();
	//b = c.blue();
	//vtkMath::RGBToHSV(normalise_rgba(r), normalise_rgba(g), normalise_rgba(b), &h, &s, &v);
	//std::cout << "after vtkMath::RGBToHSV " << h << " " << s << " " << v << std::endl;

	////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////
	//// MITK XML test
	//std::cout << "MITK transfer function XML test" << std::endl;
	//openTransferFunctionFromMITKXML("../../transferfuncs/MITK/CT Generic.xml");
	//
	//// Slicer XML test
	//std::cout << "Slicer transfer function XML test" << std::endl;
	//openTransferFunctionFromSlicerXML("../../transferfuncs/Slicer/presets.xml");
	////////////////////////////////////////////////////////////////////////////

	// there variables should not be used before initialization
	count_of_voxels = 0;
	volume_ptr = NULL;
	batch_patch = "D:/_uchar/vortex/";

	set_colour_number_in_spectrum(16);
	generate_spectrum_ramp_transfer_function_and_check_menu_item();

	on_action_Compute_Distance_HSV_triggered();


	QObject::connect(getGraphicsScene_for_spectrum(), SIGNAL(selectionChanged()), this, SLOT(slot_GraphicsScene_selectionChanged()));
	QObject::connect(getGraphicsScene_for_spectrum(), SIGNAL(sceneRectChanged(const QRectF &)), this, SLOT(slot_GraphicsScene_sceneRectChanged(const QRectF &)));

	QObject::connect(ui->listView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slot_ListView_activated(const QModelIndex &)));
	QObject::connect(ui->listView, SIGNAL(activated(const QModelIndex &)), this, SLOT(slot_ListView_activated(const QModelIndex &)));

	ui->listView->setModel(&model_for_listview);

	colour_for_optimization = Qt::blue;

	//// maximize the window
	//this->showMaximized();
}

MainWindow::~MainWindow()
{
	delete ui;
}

// frequency histogram
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

// opacity
void MainWindow::on_opacityButton_clicked()
{
	double height = ui->graphicsView->height();
	QGraphicsScene *scene = getGraphicsScene();
	scene->clear();
	scene->addText("Visibility " + QTime::currentTime().toString());
	if (intensity_list_size() > 0)
	{
		for (unsigned int i = 0; i < intensity_list_size(); i++)
		{
			double intensity = denormalise_intensity(get_intensity(i));
			auto line = scene->addLine(intensity, height, intensity + 1, (1 - get_opacity(i))*height);
			line->setFlag(QGraphicsItem::ItemIsMovable);
		}
	}
}

// noteworthiness=alpha(x)*p(x)*log(p(x))
void MainWindow::on_visibilityButton_clicked()
{
	double height = ui->graphicsView->height();
	QGraphicsScene *scene = getGraphicsScene();
	scene->clear();
	scene->addText("Visibility " + QTime::currentTime().toString());
	if (intensity_list_size() > 0)
	{
		for (unsigned int i = 0; i < intensity_list_size(); i++)
		{
			double intensity = denormalise_intensity(get_intensity(i));
			auto line = scene->addLine(intensity, height, intensity + 1, (1 - get_visibility(i) * 16)*height);
			line->setFlag(QGraphicsItem::ItemIsMovable);
			std::cout << "visibility=" << get_visibility(i) << " ymax=" << y_max << std::endl;
		}
	}
}

// entropy=p(x)*log(p(x))
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

// integration of alpha(x)*p(x)*log(p(x)) over intensity range [x(i),x(i+1)]
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
		out << iteration_count << "," << get_energy_function_edge() << std::endl;
		iteration_count++;
#endif
		//balance_opacity();
		balance_transfer_function_edge();
	}
#ifdef OUTPUT_TO_FILE
	out << iteration_count << "," << get_energy_function_edge() << std::endl;
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
		out << iteration_count << "," << get_energy_function_edge() << std::endl;
		out2 << iteration_count << "," << get_energy_function_edge_weighted_for_region() << std::endl;
		iteration_count++;
#endif
		//balance_opacity_for_region();
		balance_transfer_function_edge_for_region();
	}
#ifdef OUTPUT_TO_FILE
	out << iteration_count << "," << get_energy_function_edge() << std::endl;
	out.close();
	out2 << iteration_count << "," << get_energy_function_edge_weighted_for_region() << std::endl;
	out2.close();
#endif
	updateTransferFunctionWidgetsFromArrays();
	updateTransferFunctionArraysFromWidgets();
}

void MainWindow::on_action_About_triggered()
{
	QMessageBox msgBox;
	msgBox.setText(QString::fromUtf8("Volume Renderer\nCopyright (c) 2014 by Shengzhou Luo at Trinity College Dublin.\nAll rights reserved."));
	msgBox.exec();
}

void MainWindow::on_action_Exit_triggered()
{
	qApp->quit();
}

void MainWindow::on_action_Open_Volume_triggered()
{
	// show file dialog. change volume_filename only when the new filename is not empty.
	QString filter("Meta image file (*.mhd *.mha)");
	QString filename_backup = volume_filename;
	filename_backup = QFileDialog::getOpenFileName(this, QString(tr("Open a volume data set")), filename_backup, filter);
	if (!filename_backup.trimmed().isEmpty())
	{
		volume_filename = filename_backup;
	}
	else
	{
		return;
	}

	// open the volume file
	open_volume(filename_backup);
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
	auto volumeProperty = volume_property_widget.volumeProperty();

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

	auto window = vtk_widget.GetRenderWindow();
	window->Render();

	// initialize the interactor
	interactor->Initialize();
	interactor->Start();
}

void MainWindow::on_action_Open_Transfer_Function_triggered()
{
	// show file dialog
	QString filter("Voreen transfer function (*.tfi);;3D Slicer transfer function (*.xml)");
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

	auto p = strstr(filename_str, ".tfi");
	if (p)
	{
		std::cout << p << std::endl;
		openTransferFunctionFromVoreenXML(filename_str);
	}
	else
	{
		//Domain_x(0);
		//Domain_y(65535);
		openTransferFunctionFromSlicerXML(filename_str);
		updateTransferFunctionArraysFromWidgets();
	}

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

	saveTransferFunctionToVoreenXML(filename_str);
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

	read_region_image_and_compute_distance(1);
}

void MainWindow::on_action_Compute_Squared_Distance_triggered()
{
	ui->action_Compute_Distance->setChecked(false);
	ui->action_Compute_Squared_Distance->setChecked(true);
	ui->action_Compute_Squared_Distance_HSV->setChecked(false);
	ui->action_Compute_Distance_HSV->setChecked(false);
	enable_squared_distance = 1;
	enable_hsv_distance = 0;
	std::cout << "squared distance (RGB) is chosen" << std::endl;
	read_region_image_and_compute_distance(enable_squared_distance, enable_hsv_distance);
}

void MainWindow::on_action_Compute_Distance_triggered()
{
	ui->action_Compute_Distance->setChecked(true);
	ui->action_Compute_Squared_Distance->setChecked(false);
	ui->action_Compute_Squared_Distance_HSV->setChecked(false);
	ui->action_Compute_Distance_HSV->setChecked(false);
	enable_squared_distance = 0;
	enable_hsv_distance = 0;
	std::cout << "distance (RGB) is chosen" << std::endl;
	read_region_image_and_compute_distance(enable_squared_distance, enable_hsv_distance);
}

void MainWindow::on_action_Compute_Squared_Distance_HSV_triggered()
{
	ui->action_Compute_Distance->setChecked(false);
	ui->action_Compute_Squared_Distance->setChecked(false);
	ui->action_Compute_Squared_Distance_HSV->setChecked(true);
	ui->action_Compute_Distance_HSV->setChecked(false);
	enable_squared_distance = 1;
	enable_hsv_distance = 1;
	std::cout << "squared distance (HSV) is chosen" << std::endl;
	read_region_image_and_compute_distance(enable_squared_distance, enable_hsv_distance);
}

void MainWindow::on_action_Compute_Distance_HSV_triggered()
{
	ui->action_Compute_Distance->setChecked(false);
	ui->action_Compute_Squared_Distance->setChecked(false);
	ui->action_Compute_Squared_Distance_HSV->setChecked(false);
	ui->action_Compute_Distance_HSV->setChecked(true);
	enable_squared_distance = 0;
	enable_hsv_distance = 1;
	std::cout << "distance (HSV) is chosen" << std::endl;
	read_region_image_and_compute_distance(enable_squared_distance, enable_hsv_distance);
}

void MainWindow::on_action_Default_Transfer_Function_triggered()
{
	generate_default_transfer_function();
	updateTransferFunctionWidgetsFromArrays();
}

void MainWindow::on_action_Spectrum_Transfer_Function_triggered()
{
	ui->action_Spectrum_Ramp_Transfer_Function->setChecked(false);
	ui->action_Spectrum_Transfer_Function->setChecked(true);
	enable_spectrum_ramp = 0;
	bool ok;
	int n = number_of_colours_in_spectrum;
	n = QInputDialog::getInt(this, tr("Spectrum Transfer Function"), tr("Number of colours [1,256]:"), n, 1, 256, 1, &ok);
	if (ok)
	{
		//std::cout << "QInputDialog::getInteger() " << n << std::endl;
		generate_spectrum_transfer_function(n);
		updateTransferFunctionWidgetsFromArrays();
		set_colour_number_in_spectrum(n);
	}
}

void MainWindow::on_action_Spectrum_Ramp_Transfer_Function_triggered()
{
	ui->action_Spectrum_Ramp_Transfer_Function->setChecked(true);
	ui->action_Spectrum_Transfer_Function->setChecked(false);
	enable_spectrum_ramp = 1;
	bool ok;
	int n = number_of_colours_in_spectrum;
	n = QInputDialog::getInt(this, tr("Spectrum Ramp Transfer Function"), tr("Number of colours [1,256]:"), n, 1, 256, 1, &ok);
	if (ok)
	{
		//std::cout << "QInputDialog::getInteger() " << n << std::endl;
		generate_spectrum_ramp_transfer_function(n);
		updateTransferFunctionWidgetsFromArrays();
		set_colour_number_in_spectrum(n);
	}
}

void MainWindow::on_action_Pick_a_colour_and_optimise_transfer_function_triggered()
{
	QColor colour = QColorDialog::getColor(colour_for_optimization, this);
	if (colour.isValid())
	{
		//pick_colour_and_compute_distance(colour.red(), colour.green(), colour.blue());
		//std::cout << "picked colour (RGB) " << colour.red() << " " << colour.green() << " " << colour.blue() << std::endl;
		// reset transfer function before optimizing it
		reset_transfer_function();

		// optimise for a specific colour
		optimise_transfer_function_for_colour(colour);
	}

	//// optimise the transfer function for the selected colour
	//updateTransferFunctionArraysFromWidgets();
	//int n = ui->spinBox->value();
	//if (n < 1 || n > max_iteration_count)
	//{
	//	n = 1;
	//}
	//while (n-- > 0)
	//{
	//	balance_opacity_for_region();
	//}
	//updateTransferFunctionWidgetsFromArrays();
	//updateTransferFunctionArraysFromWidgets();
}

void MainWindow::on_action_Genearte_transfer_functions_for_spectrum_triggered()
{
	bool ok;
	int n = number_of_colours_in_spectrum;
	n = QInputDialog::getInt(this, tr("Number of Colours"), tr("Number of colours [1,256]:"), n, 1, 256, 1, &ok);
	if (ok)
	{
		for (int i = 0; i < n; i++)
		{
			QColor colour;
			colour.setHsv(i * 360 / n, 255, 255);

			// reset transfer function before optimizing it
			reset_transfer_function();

			// optimize for a specific colour
			optimise_transfer_function_for_colour(colour);

			char c_str2[_MAX_PATH];
			sprintf(c_str2, "D:/output/_tf/%02d.tfi", i);
			//char str0[_MAX_PATH];
			//sprintf(str0, ".%03d.tfi", i);
			//QString str1 = volume_filename + QString(str0);
			//QByteArray ba = str1.toLocal8Bit();
			//const char *c_str2 = ba.data();
			saveTransferFunctionToVoreenXML(c_str2);
			std::cout << "saved to file " << c_str2 << std::endl;
		}
	}
}

void MainWindow::on_action_Open_path_and_generate_transfer_functions_triggered()
{
	bool ok;
	QString path = batch_patch;
	path = QInputDialog::getText(this, tr("Global optimization"),
		tr("Path to open:"), QLineEdit::Normal,
		path, &ok);
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

		// save the path for next time
		batch_patch = path;

		// get filenames under the folder
		QDir dir = QDir(filepath);
		QStringList files = dir.entryList(QStringList(filename), QDir::Files | QDir::NoSymLinks);

		// a QList to be put in QStandardItemModel
		QList<QStandardItem *> filename_list;

		for (int i = 0; i < files.size(); i++)
		{
			filename_list.append(new QStandardItem(QString(filepath + files[i])));

			// load volume
			open_volume_no_rendering(filepath + files[i]);

			reset_transfer_function();

			int n = ui->spinBox->value();
			if (n < 1 || n > max_iteration_count)
			{
				n = 1;
			}

			while (n-- > 0)
			{
				//balance_opacity();
				balance_transfer_function_edge();
			}

			updateTransferFunctionWidgetsFromArrays();
			updateTransferFunctionArraysFromWidgets();

			// split filename and extension
			QStringList str_list = files[i].split(".", QString::SkipEmptyParts);

			// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters)
			QByteArray ba = str_list[0].toLocal8Bit();
			const char *filename_no_suffix = ba.data();

			QByteArray ba1 = filepath.toLocal8Bit();
			const char *path1 = ba1.data();

			// save the transfer function to file
			char filename_str[_MAX_PATH];
			sprintf(filename_str, "%s%s.tfi", path1, filename_no_suffix);
			std::cout << "transfer function file: " << filename_str << endl;
			saveTransferFunctionToVoreenXML(filename_str);
		}

		model_for_listview.clear();
		model_for_listview.appendColumn(filename_list);
	}
}

void MainWindow::on_action_Open_path_and_generate_transfer_functions_for_region_triggered()
{
	bool ok;
	QString path = batch_patch;
	path = QInputDialog::getText(this, tr("Region-based optimization"),
		tr("Path to open:"), QLineEdit::Normal,
		path, &ok);
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

		// save the path for next time
		batch_patch = path;

		// get filenames under the folder
		QDir dir = QDir(filepath);
		QStringList files = dir.entryList(QStringList(filename), QDir::Files | QDir::NoSymLinks);

		// a QList to be put in QStandardItemModel
		QList<QStandardItem *> filename_list;

		for (int i = 0; i < files.size(); i++)
		{
			filename_list.append(new QStandardItem(QString(filepath + files[i])));

			// load volume
			open_volume_no_rendering(filepath + files[i]);

			reset_transfer_function();

			// compute region-based difference factors
			read_region_image_and_compute_distance(enable_squared_distance, enable_hsv_distance);

			int n = ui->spinBox->value();
			if (n < 1 || n > max_iteration_count)
			{
				n = 1;
			}

			while (n-- > 0)
			{
				//balance_opacity_for_region();
				balance_transfer_function_edge_for_region();
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
			saveTransferFunctionToVoreenXML(filename_str);
		}

		model_for_listview.clear();
		model_for_listview.appendColumn(filename_list);
	}
}

void MainWindow::on_action_Open_path_and_generate_transfer_functions_for_colour_triggered()
{
	bool ok;
	QString path = batch_patch;
	path = QInputDialog::getText(this, tr("Colour-based optimization"),
		tr("Path to open:"), QLineEdit::Normal,
		path, &ok);
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

		// save the path for next time
		batch_patch = path;

		// get filenames under the folder
		QDir dir = QDir(filepath);
		QStringList files = dir.entryList(QStringList(filename), QDir::Files | QDir::NoSymLinks);

		// a QList to be put in QStandardItemModel
		QList<QStandardItem *> filename_list;

		for (int i = 0; i < files.size(); i++)
		{
			filename_list.append(new QStandardItem(QString(filepath + files[i])));

			// load volume
			open_volume_no_rendering(filepath + files[i]);

			reset_transfer_function();

			optimise_transfer_function_for_colour(colour_for_optimization);

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
			saveTransferFunctionToVoreenXML(filename_str);
		}

		model_for_listview.clear();
		model_for_listview.appendColumn(filename_list);
	}
}

void MainWindow::on_resetButton_clicked()
{
	reset_transfer_function();
	updateTransferFunctionWidgetsFromArrays();
	draw_spectrum_in_graphicsview(number_of_colours_in_spectrum);
}

void MainWindow::on_action_Test_triggered()
{
	//QGraphicsScene *scene = getGraphicsScene_for_spectrum();
	//std::cout << "width=" << scene->width() << " height=" << scene->height() << std::endl;

	//QList<QStandardItem *> list1;
	//auto i0 = new QStandardItem("D:\\_uchar\\vortex\\00.mhd");
	//i0->setData(QVariant("D:\\_uchar\\vortex\\00.mhd"));
	//list1.append(i0);
	//auto i1 = new QStandardItem("D:/_uchar/vortex/01.mhd");
	//i1->setData(QVariant("D:/_uchar/vortex/01.mhd"));
	//list1.append(i1);
	//auto i2 = new QStandardItem("D:\\_uchar\\vortex\\02.mhd");
	//i2->setData(QVariant("D:\\_uchar\\vortex\\02.mhd"));
	//list1.append(i2);
	//model_for_listview.clear();
	//model_for_listview.appendColumn(list1);

	std::cout << "weights" << std::endl;
	for (int i = 0; i < control_point_weight_list.size(); i++)
	{
		std::cout << control_point_weight_list[i] << std::endl;
	}

	double a, b;
	for (int index = 0; index < intensity_list_size() - 1; index++)
	{
		std::cout << "interpolation index=" << index << std::endl;
		if (index == intensity_list_size() - 1)
		{
			a = get_intensity(index);
			b = 1;
		}
		else
		{
			a = get_intensity(index);
			b = get_intensity(index + 1);
		}
		a = denormalise_intensity(a);
		b = denormalise_intensity(b);

		for (int intensity = (int)a; intensity < b; intensity++)
		{
			if (intensity >= a)
			{
				double normalised = normalise_intensity(intensity);
				std::cout << "intensity=" << intensity << " normalised=" << normalised << " weight=" << get_control_point_weight_by_interpolation(normalised, index) << std::endl;
			}
		}
	}
}

void MainWindow::on_drawWeightButton_clicked()
{
	double height = ui->graphicsView->height();
	QGraphicsScene *scene = getGraphicsScene();
	scene->clear();
	scene->addText("Weights " + QTime::currentTime().toString());

	if (control_point_weight_list.size() > 0)
	{
		double a, b;
		// 0 to 255
		for (int index = 0; index < intensity_list_size() - 1; index++)
		{
			if (index == intensity_list_size() - 1)
			{
				a = get_intensity(index);
				b = 1;
			}
			else
			{
				a = get_intensity(index);
				b = get_intensity(index + 1);
			}
			a = denormalise_intensity(a);
			b = denormalise_intensity(b);

			for (int intensity = (int)a; intensity < b; intensity++)
			{
				if (intensity >= a)
				{
					double normalised = normalise_intensity(intensity);
					//std::cout << "intensity=" << intensity << " normalised=" << normalised << " weight=" << get_control_point_weight_by_interpolation(normalised, index) << std::endl;

					auto line = scene->addLine(intensity, height, intensity + 1, (1 - 10 * get_control_point_weight_by_interpolation(normalised, index))*height);
				}
			}
		}
	}
}
