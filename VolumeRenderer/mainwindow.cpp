#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "voxel_utility.h"

//#ifndef OUTPUT_TO_FILE
//#define OUTPUT_TO_FILE
//#endif // OUTPUT_TO_FILE

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	Window_title(this->windowTitle());

	populateRenderingTechniqueComboBox();

	// turn off debug, warning or error messages
	//vtkObject::GlobalWarningDisplayOff();

	// add VTK widgets
	get_vtk_layout()->addWidget(&vtk_widget);
	get_property_layout()->addWidget(&volume_property_widget);
	get_histogram_layout()->addWidget(&ctkVTKScalarsToColorsWidget1);
	get_histogram_layout()->addWidget(&ctkVTKScalarsToColorsWidget2);

	// set up renderer
	renderer = vtkSmartPointer<vtkRenderer>::New();
	vtk_widget.GetRenderWindow()->AddRenderer(renderer);

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
	//scalar_opacity->AddPoint(0.0, 0.0);
	//scalar_opacity->AddPoint(36.0, 0.125);
	//scalar_opacity->AddPoint(72.0, 0.25);
	//scalar_opacity->AddPoint(108.0, 0.375);
	//scalar_opacity->AddPoint(144.0, 0.5);
	//scalar_opacity->AddPoint(180.0, 0.625);
	//scalar_opacity->AddPoint(216.0, 0.75);
	//scalar_opacity->AddPoint(255.0, 0.0);

	// Create transfer mapping scalar value to color.
	scalar_color = vtkSmartPointer<vtkColorTransferFunction>::New();
	//scalar_color->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
	//scalar_color->AddRGBPoint(36.0, 1.0, 0.0, 0.0);
	//scalar_color->AddRGBPoint(72.0, 1.0, 1.0, 0.0);
	//scalar_color->AddRGBPoint(108.0, 0.0, 1.0, 0.0);
	//scalar_color->AddRGBPoint(144.0, 0.0, 1.0, 1.0);
	//scalar_color->AddRGBPoint(180.0, 0.0, 0.0, 1.0);
	//scalar_color->AddRGBPoint(216.0, 1.0, 0.0, 1.0);
	//scalar_color->AddRGBPoint(255.0, 1.0, 1.0, 1.0);

	std::cout << "path.volume_filename=" << path.volume_filename << std::endl;
	volume_filename = QString::fromStdString(path.volume_filename);
	transfer_function_filename = QString::fromStdString(path.transfer_function_filename);
	transfer_function_filename_save = QString::fromStdString(path.transfer_function_filename_save);
	selected_region_filename = QString::fromStdString(path.selected_region_filename);
	batch_path = QString::fromStdString(path.batch_path);
	
	// there variables should not be used before initialization
	Domain_x(0);
	Domain_y(1);
	Threshold_x(0);
	Threshold_y(1);
	Range_x(0);
	Range_y(255);
	count_of_voxels = 0;
	volume_ptr = NULL;

	// generate default transfer function
	Number_of_colours_in_spectrum(16);
	generate_spectrum_ramp_transfer_function_and_check_menu_item();
	update_colour_palette();

	opacity_function_1 = vtkSmartPointer<vtkPiecewiseFunction>::New();
	opacity_function_1->DeepCopy(scalar_opacity);
	ctkVTKScalarsToColorsWidget1.view()->addOpacityFunction(opacity_function_1);
	opacity_function_2 = vtkSmartPointer<vtkPiecewiseFunction>::New();
	opacity_function_2->DeepCopy(scalar_opacity);
	ctkVTKScalarsToColorsWidget2.view()->addOpacityFunction(opacity_function_2);

	// use Slicer's renderer by default
	on_action_VtkSlicerGPURayCastVolumeMapper_triggered();

	// use HSV without squaring distance
	on_action_Compute_Distance_HSV_triggered();

	QObject::connect(get_GraphicsScene_for_spectrum(), SIGNAL(selectionChanged()), this, SLOT(slot_GraphicsScene_selectionChanged()));
	QObject::connect(get_GraphicsScene_for_spectrum(), SIGNAL(sceneRectChanged(const QRectF &)), this, SLOT(slot_GraphicsScene_sceneRectChanged(const QRectF &)));

	QObject::connect(&screenshot_widget, SIGNAL(region_selected(QString)), this, SLOT(slot_region_selected(QString)));

	ui->listView->setModel(&model_for_listview);
	ui->listView_2->setModel(&model_for_listview2);

	colour_for_optimization = Qt::blue;

	// maximize the window
	this->showMaximized();
}

MainWindow::~MainWindow()
{
	delete ui;
}

// frequency histogram
void MainWindow::on_frequencyButton_clicked()
{
	double height = ui->graphicsView->height();
	QGraphicsScene *scene = get_GraphicsScene();
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
	QGraphicsScene *scene = get_GraphicsScene();
	scene->clear();
	scene->addText("Visibility " + QTime::currentTime().toString());
	if (intensity_list_size() > 0)
	{
		for (int i = 0; i < intensity_list_size(); i++)
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
	QGraphicsScene *scene = get_GraphicsScene();
	scene->clear();
	scene->addText("Visibility " + QTime::currentTime().toString());
	if (intensity_list_size() > 0)
	{
		for (int i = 0; i < intensity_list_size(); i++)
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
	QGraphicsScene *scene = get_GraphicsScene();
	scene->clear();
	scene->addText("Entropy " + QTime::currentTime().toString());
	if (frequency_list.size() > 0)
	{
		unsigned int index = -1;
		for (unsigned int intensity = 0; intensity < frequency_list.size(); intensity++) // 0 to 255
		{
			double entropy = get_entropy(intensity);
			auto line = scene->addLine(intensity, height, intensity + 1, (1 - entropy * 10)*height);
			line->setFlag(QGraphicsItem::ItemIsMovable);

		}
	}
}

// integration of alpha(x)*p(x)*log(p(x)) over intensity range [x(i),x(i+1)]
void MainWindow::on_entropyOpacityButton_clicked()
{
	double height = ui->graphicsView->height();
	QGraphicsScene *scene = get_GraphicsScene();
	scene->clear();
	scene->addText("Entropy Opacity " + QTime::currentTime().toString());
	if (frequency_list.size() > 0)
	{
		unsigned int index = -1;
		for (unsigned int intensity = 0; intensity < frequency_list.size(); intensity++) // 0 to 255
		{
			double entropy = get_entropy_opacity(intensity);
			auto line = scene->addLine(intensity, height, intensity + 1, (1 - entropy * 16)*height);
			line->setFlag(QGraphicsItem::ItemIsMovable);

		}
	}
}

void MainWindow::on_balanceButton_clicked()
{
	updateOpacityArrayFromTFWidget();
	int n = ui->spinBox->value();
	if (n < 1 || n > max_iteration_count)
	{
		n = 1;
	}
	while (n-- > 0)
	{
		balance_transfer_function();
	}
	updateTFWidgetFromOpacityArrays();
	updateOpacityArrayFromTFWidget();
}

void MainWindow::on_lhHistogramButton_clicked()
{
	int size[3] = { 41, 41, 41 };
	get_index(0, 0, 0, size);
}

void MainWindow::on_reduceOpacityButton_clicked()
{
	updateOpacityArrayFromTFWidget();
	int n = ui->spinBox->value();
	if (n < 1 || n > max_iteration_count)
	{
		n = 1;
	}
	while (n-- > 0)
	{
		reduce_opacity();
	}
	updateTFWidgetFromOpacityArrays();
	updateOpacityArrayFromTFWidget();
}

void MainWindow::on_increaseOpacityButton_clicked()
{
	updateOpacityArrayFromTFWidget();
	int n = ui->spinBox->value();
	if (n < 1 || n > max_iteration_count)
	{
		n = 1;
	}
	while (n-- > 0)
	{
		increase_opacity();
	}
	updateTFWidgetFromOpacityArrays();
	updateOpacityArrayFromTFWidget();
}

void MainWindow::on_balanceEdgeButton_clicked()
{
	updateOpacityArrayFromTFWidget();
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

	clock_t begin = clock();

	while (n-- > 0)
	{
#ifdef OUTPUT_TO_FILE
		out << iteration_count << "," << get_energy_function_edge() << std::endl;
		iteration_count++;
#endif
		balance_transfer_function_edge();
	}

	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	std::cout << elapsed_secs << std::endl;

#ifdef OUTPUT_TO_FILE
	out << iteration_count << "," << get_energy_function_edge() << std::endl;
	out.close();
#endif
	updateTFWidgetFromOpacityArrays();
	updateOpacityArrayFromTFWidget();
}

void MainWindow::on_enhanceRegionButton_clicked()
{
	updateOpacityArrayFromTFWidget();
	int n = ui->spinBox->value();
	if (n < 1 || n > max_iteration_count)
	{
		n = 1;
	}
	while (n-- > 0)
	{
		increase_opacity_for_region();
	}
	updateTFWidgetFromOpacityArrays();
	updateOpacityArrayFromTFWidget();
}

void MainWindow::on_weakenRegionButton_clicked()
{
	updateOpacityArrayFromTFWidget();
	int n = ui->spinBox->value();
	if (n < 1 || n > max_iteration_count)
	{
		n = 1;
	}
	while (n-- > 0)
	{
		reduce_opacity_for_region();
	}
	updateTFWidgetFromOpacityArrays();
	updateOpacityArrayFromTFWidget();
}

void MainWindow::on_balanceRegionButton_clicked()
{
	updateOpacityArrayFromTFWidget();
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
		balance_transfer_function_edge_for_region();
	}
#ifdef OUTPUT_TO_FILE
	out << iteration_count << "," << get_energy_function_edge() << std::endl;
	out.close();
	out2 << iteration_count << "," << get_energy_function_edge_weighted_for_region() << std::endl;
	out2.close();
#endif
	updateTFWidgetFromOpacityArrays();
	updateOpacityArrayFromTFWidget();
}

void MainWindow::on_action_About_triggered()
{
	QMessageBox msgBox;
	msgBox.setText(QString::fromUtf8("Volume Visualiser \nCopyright (c) 2013-2016 by Shengzhou Luo at Trinity College Dublin. \nAll rights reserved."));
	msgBox.exec();
}

void MainWindow::on_action_Exit_triggered()
{
	qApp->quit();
}

void MainWindow::on_action_Open_Volume_triggered()
{
	// show file dialog. change volume_filename only when the new filename is not empty.
	QString filter("all supported (*.mhd *.mha *.nrrd *.nhdr);; UNC MetaImage (*.mhd *.mha);; Nearly Raw Raster Data(*.nrrd *.nhdr)");
	QString filename_backup = volume_filename;
	filename_backup = QFileDialog::getOpenFileName(this, QString(tr("Open a volume data set")), filename_backup, filter);
	if (!filename_backup.trimmed().isEmpty())
	{
		volume_filename = filename_backup;
		path.save();
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
	QString filter("UNC MetaImage (*.mhd *.mha)");
	QString filename_backup = volume_filename;
	filename_backup = QFileDialog::getOpenFileName(this, QString(tr("Open a volume data set")), filename_backup, filter);
	if (!filename_backup.trimmed().isEmpty())
	{
		volume_filename = filename_backup;
		path.save();
	}
	else
	{
		return;
	}

	// show filename on window title
	this->setWindowTitle(QString::fromUtf8("Volume Visualiser - ") + volume_filename);

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
	QString filter("all supported (*.tfi *.xml);; Voreen transfer function (*.tfi);; MITK transfer function (*.xml)");
	QString filename_backup = transfer_function_filename;
	filename_backup = QFileDialog::getOpenFileName(this, QString(tr("Open a transfer function")), filename_backup, filter);
	if (!filename_backup.trimmed().isEmpty())
	{
		transfer_function_filename = filename_backup;
		path.save();
	}
	else
	{
		return;
	}

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
		openTransferFunctionFromMITKXML(filename_str);
		updateOpacityArrayFromTFWidget();
	}

	updateTFWidgetFromOpacityArrays();
	update_colour_palette();

	std::cout << "updateTransferFunctionWidgetsFromArrays" << std::endl;

	std::cout << "gradient_opacity size=" << gradient_opacity->GetSize() << std::endl;
	for (int i = 0; i < gradient_opacity->GetSize(); i++)
	{
		double xa[4];
		gradient_opacity->GetNodeValue(i, xa);
		std::cout << xa[0] << " " << xa[1] << std::endl;
	}

	std::cout << "scalar_opacity size=" << scalar_opacity->GetSize() << std::endl;
	for (int i = 0; i < scalar_opacity->GetSize(); i++)
	{
		double xa[4];
		scalar_opacity->GetNodeValue(i, xa);
		std::cout << xa[0] << " " << xa[1] << std::endl;
	}

	std::cout << "color_tf size=" << scalar_color->GetSize() << std::endl;
	for (int i = 0; i < scalar_color->GetSize(); i++)
	{
		double xrgb[6];
		scalar_color->GetNodeValue(i, xrgb);
		std::cout << xrgb[0] << " " << xrgb[1] << " " << xrgb[2] << " " << xrgb[3] << std::endl;
	}
}

void MainWindow::on_action_Save_Transfer_Function_triggered()
{
	// show file dialog
	QString filter("Voreen transfer function (*.tfi);; MITK transfer function (*.xml)");
	QString filename_backup = transfer_function_filename_save;
	filename_backup = QFileDialog::getSaveFileName(this, QString(tr("Save transfer function as")), filename_backup, filter);
	if (!filename_backup.trimmed().isEmpty())
	{
		transfer_function_filename_save = filename_backup;
		path.save();
	}
	else
	{
		return;
	}

	// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
	QByteArray ba = filename_backup.toLocal8Bit();
	const char *filename_str = ba.data();

	std::cout << "transfer function file: " << filename_str << endl;

	auto p = strstr(filename_str, ".tfi");
	if (p)
	{
		std::cout << p << std::endl;
		saveTransferFunctionToVoreenXML(filename_str);
	}
	else
	{
		saveTransferFunctionToMITKXML(filename_str);
	}
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
	updateTFWidgetFromOpacityArrays();
}

void MainWindow::on_action_Spectrum_Transfer_Function_triggered()
{
	ui->action_Spectrum_Ramp_Transfer_Function->setChecked(false);
	ui->action_Spectrum_Transfer_Function->setChecked(true);
	enable_spectrum_ramp = 0;
	bool ok;
	int n = Number_of_colours_in_spectrum();
	n = QInputDialog::getInt(this, tr("Spectrum Transfer Function"), tr("Number of colours [1,256]:"), n, 1, 256, 1, &ok);
	if (ok)
	{
		generate_spectrum_transfer_function(n);
		updateTFWidgetFromOpacityArrays();
		Number_of_colours_in_spectrum(n);
		update_colour_palette();
	}
}

void MainWindow::on_action_Spectrum_Ramp_Transfer_Function_triggered()
{
	ui->action_Spectrum_Ramp_Transfer_Function->setChecked(true);
	ui->action_Spectrum_Transfer_Function->setChecked(false);
	enable_spectrum_ramp = 1;
	bool ok;
	int n = Number_of_colours_in_spectrum();
	n = QInputDialog::getInt(this, tr("Spectrum Ramp Transfer Function"), tr("Number of colours [1,256]:"), n, 1, 256, 1, &ok);
	if (ok)
	{
		generate_spectrum_ramp_transfer_function(n);
		updateTFWidgetFromOpacityArrays();
		Number_of_colours_in_spectrum(n);
		update_colour_palette();
	}
}

void MainWindow::on_action_Pick_a_colour_and_optimise_transfer_function_triggered()
{
	QColor colour = QColorDialog::getColor(colour_for_optimization, this);
	if (colour.isValid())
	{
		// reset transfer function before optimizing it
		reset_transfer_function();

		// optimise for a specific colour
		optimise_transfer_function_for_colour(colour);
	}
}

void MainWindow::on_action_Genearte_transfer_functions_for_spectrum_triggered()
{
	bool ok;
	int n = Number_of_colours_in_spectrum();
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

			//char c_str2[_MAX_PATH];
			//sprintf(c_str2, "D:/output/_tf/%02d.tfi", i);
			//saveTransferFunctionToVoreenXML(c_str2);
			//std::cout << "saved to file " << c_str2 << std::endl;
		}
	}
}

void MainWindow::on_action_Open_path_and_generate_transfer_functions_triggered()
{
	bool ok;
	QString path = batch_path;
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
		batch_path = path;

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
				balance_transfer_function_edge();
			}

			updateTFWidgetFromOpacityArrays();
			updateOpacityArrayFromTFWidget();

			// split filename and extension
			QStringList str_list = files[i].split(".", QString::SkipEmptyParts);
			if (str_list.size()>1)
			{
				str_list.removeLast();
			}
			// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters)
			QByteArray ba = str_list.join(".").toLocal8Bit();
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
	QString path = batch_path;
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
		batch_path = path;

		// get filenames under the folder
		QDir dir = QDir(filepath);
		QStringList files = dir.entryList(QStringList(filename), QDir::Files | QDir::NoSymLinks);

		// a QList to be put in QStandardItemModel
		QList<QStandardItem *> filename_list;

		for (int i = 0; i < files.size(); i++)
		{
			filename_list.append(new QStandardItem(QString(filepath + files[i])));

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
				balance_transfer_function_edge_for_region();
			}

			updateTFWidgetFromOpacityArrays();
			updateOpacityArrayFromTFWidget();

			// split filename and extension
			QStringList str_list = files[i].split(".", QString::SkipEmptyParts);
			if (str_list.size() > 1)
			{
				str_list.removeLast();
			}
			// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters)
			QByteArray ba = str_list.join(".").toLocal8Bit();
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
	QString path = batch_path;
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
		batch_path = path;

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
			QStringList str_list = files[i].split(".", QString::SkipEmptyParts);
			if (str_list.size() > 1)
			{
				str_list.removeLast();
			}
			// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters)
			QByteArray ba = str_list.join(".").toLocal8Bit();
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
	updateTFWidgetFromOpacityArrays();
	update_colour_palette();
}

void MainWindow::on_action_Test_triggered()
{
	renderer->ResetCamera();
	auto camera = renderer->GetActiveCamera();
	auto fp = camera->GetFocalPoint();
	auto p = camera->GetPosition();
	std::cout << "focal point " << fp[0] << " " << fp[1] << " " << fp[2] << "\tposition " << p[0] << " " << p[1] << " " << p[2] << std::endl;
	auto dist = sqrt((p[0] - fp[0])*(p[0] - fp[0]) + (p[1] - fp[1])*(p[1] - fp[1]) + (p[2] - fp[2])*(p[2] - fp[2]));
	camera->SetPosition(fp[0], fp[1], fp[2] + dist);
	camera->SetViewUp(0.0, 1.0, 0.0);
	fp = camera->GetFocalPoint();
	p = camera->GetPosition();
	std::cout << "focal point " << fp[0] << " " << fp[1] << " " << fp[2] << "\tposition " << p[0] << " " << p[1] << " " << p[2] << std::endl;
	std::cout << "distance " << dist << std::endl;

	// update vtk widget
	vtk_widget.repaint();
}

void MainWindow::on_drawWeightButton_clicked()
{
	double height = ui->graphicsView->height();
	QGraphicsScene *scene = get_GraphicsScene();
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
					auto line = scene->addLine(intensity, height, intensity + 1, (1 - 10 * get_control_point_weight_by_interpolation(normalised, index))*height);
				}
			}
		}
	}
}

void MainWindow::on_action_VtkSmartVolumeMapper_triggered()
{
	ui->action_VtkSmartVolumeMapper->setChecked(true);
	ui->action_VtkSlicerGPURayCastVolumeMapper->setChecked(false);
	ui->action_VtkSlicerGPURayCastMultiVolumeMapper->setChecked(false);
	Volume_mapper_index(0);
}

void MainWindow::on_action_VtkSlicerGPURayCastVolumeMapper_triggered()
{
	ui->action_VtkSmartVolumeMapper->setChecked(false);
	ui->action_VtkSlicerGPURayCastVolumeMapper->setChecked(true);
	ui->action_VtkSlicerGPURayCastMultiVolumeMapper->setChecked(false);
	Volume_mapper_index(1);
}

void MainWindow::on_action_VtkSlicerGPURayCastMultiVolumeMapper_triggered()
{
	ui->action_VtkSmartVolumeMapper->setChecked(false);
	ui->action_VtkSlicerGPURayCastVolumeMapper->setChecked(false);
	ui->action_VtkSlicerGPURayCastMultiVolumeMapper->setChecked(true);
	Volume_mapper_index(2);
}

void MainWindow::on_reloadButton_clicked()
{
	reload_transfer_function_from_file();
}

void MainWindow::on_pushButton_clicked()
{
	opacity_function_1->DeepCopy(scalar_opacity);
}

void MainWindow::on_pushButton_2_clicked()
{
	opacity_function_2->DeepCopy(scalar_opacity);
}

void MainWindow::on_pushButton_3_clicked()
{
	scalar_opacity->DeepCopy(opacity_function_1);
	updateOpacityArrayFromTFWidget();
	updateTFWidgetFromOpacityArrays();
}

void MainWindow::on_pushButton_4_clicked()
{
	scalar_opacity->DeepCopy(opacity_function_2);
	updateOpacityArrayFromTFWidget();
	updateTFWidgetFromOpacityArrays();
}

void MainWindow::on_pushButton_5_clicked()
{
	double rate1 = ui->doubleSpinBox->value();
	double rate2 = ui->doubleSpinBox_2->value();
	int size1 = opacity_function_1->GetSize();
	int size2 = opacity_function_2->GetSize();
	int size = size1 < size2 ? size1 : size2;
	std::cout << "merge opacity function size=" << size << " rate1=" << rate1 << " rate2=" << rate2 << std::endl;
	auto merger = vtkSmartPointer<vtkPiecewiseFunction>::New();
	const int N = 4;
	for (int i = 0; i < size; i++)
	{
		double xy1[N], xy2[N], xy[N];
		opacity_function_1->GetNodeValue(i, xy1);
		opacity_function_2->GetNodeValue(i, xy2);
		for (int j = 0; j < N; j++)
		{
			xy[j] = rate1 * xy1[j] + rate2 * xy2[j];
		}
		merger->AddPoint(xy[0], clamp(xy[1], 0, 1));
	}
	scalar_opacity->DeepCopy(merger);

	updateOpacityArrayFromTFWidget();
	updateTFWidgetFromOpacityArrays();
}

void MainWindow::on_pushButton_6_clicked()
{
	int n = 5;
	double sigma = 1;
	bool ok;
	QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
		tr("n and sigma for Gaussian kernel"), QLineEdit::Normal,
		"5 1", &ok);
	if (ok && !text.isEmpty())
	{
		auto list = text.split(QString(" "));
		if (list.size() >= 2)
		{
			n = atoi(list.at(0).toLocal8Bit().constData());
			sigma = atoi(list.at(1).toLocal8Bit().constData());
		}
		std::cout << "n=" << n << " sigma=" << sigma << std::endl;
	}

	int half = n / 2;
	auto kernel = gaussian_kernel_1d(n, sigma);
	std::vector<double> opacity_new = opacity_list;
	for (int i = half; i < opacity_new.size() - half; i++)
	{
		double sum = 0;
		for (int j = 0; j < n; j++)
		{
			int offset = j - half;
			int k = i + offset;
			sum += kernel[j] * get_opacity(k);
		}
		opacity_new[i] = sum;
	}
	opacity_list = opacity_new;

	updateTFWidgetFromOpacityArrays();
	updateOpacityArrayFromTFWidget();
}

void MainWindow::on_action_Screenshot_triggered()
{
	screenshot_widget.showFullScreen();
}

void MainWindow::on_action_Auto_open_selected_region_triggered()
{
	screenshot_widget.Auto_open_selected_image(ui->action_Auto_open_selected_region->isChecked());
}

void MainWindow::on_checkBox_clicked()
{
	auto volume = Volume();
	if (ui->checkBox->isChecked())
	{
		std::cout << "checked" << std::endl;
		if (volume)
		{
			double planes[6] = { 0, 100, 0, 100, 0, 100 };
			planes[0] = ui->doubleSpinBox_3->value();
			planes[1] = ui->doubleSpinBox_4->value();
			planes[2] = ui->doubleSpinBox_5->value();
			planes[3] = ui->doubleSpinBox_6->value();
			planes[4] = ui->doubleSpinBox_7->value();
			planes[5] = ui->doubleSpinBox_8->value();
			auto mapper = static_cast<vtkVolumeMapper *>(volume->GetMapper());
			mapper->SetCropping(1);
			mapper->SetCroppingRegionPlanes(planes);
		}
		else
		{
			std::cout << "empty volume" << std::endl;
		}
	}
	else
	{
		std::cout << "unchecked" << std::endl;
		if (volume)
		{
			auto mapper = static_cast<vtkVolumeMapper *>(volume->GetMapper());
			mapper->SetCropping(0);
		}
		else
		{
			std::cout << "empty volume" << std::endl;
		}
	}
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
	if (index != -1)
	{
		auto volume = Volume();
		if (volume)
		{
			auto mapper = dynamic_cast<vtkSlicerGPURayCastVolumeMapper *>(volume->GetMapper());
			if (mapper)
			{
				int technique = RenderingTechniqueComboBox()->itemData(index).toInt();
				std::cout << "technique=" << technique << std::endl;
				mapper->SetTechnique(technique);
				vtk_widget.repaint();
			}
		}
	}
	std::cout << "RenderingTechniqueComboBox currentIndex=" << RenderingTechniqueComboBox()->currentIndex() << std::endl;
}

void MainWindow::on_newtonButton_clicked()
{
	updateOpacityArrayFromTFWidget();
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
		balance_transfer_function_edge_newton();
	}
#ifdef OUTPUT_TO_FILE
	out << iteration_count << "," << get_energy_function_edge() << std::endl;
	out.close();
#endif
	updateTFWidgetFromOpacityArrays();
	updateOpacityArrayFromTFWidget();
}

void MainWindow::on_gradientDescentButton_clicked()
{
	updateOpacityArrayFromTFWidget();
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
		balance_transfer_function_edge_gradient_descent();
	}
#ifdef OUTPUT_TO_FILE
	out << iteration_count << "," << get_energy_function_edge() << std::endl;
	out.close();
#endif
	updateTFWidgetFromOpacityArrays();
	updateOpacityArrayFromTFWidget();
}

void MainWindow::on_fixedStepButton_clicked()
{
	updateOpacityArrayFromTFWidget();
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
		balance_transfer_function_edge_fixed_step_size();
	}
#ifdef OUTPUT_TO_FILE
	out << iteration_count << "," << get_energy_function_edge() << std::endl;
	out.close();
#endif
	updateTFWidgetFromOpacityArrays();
	updateOpacityArrayFromTFWidget();
}

void MainWindow::on_listView_clicked(const QModelIndex &index)
{
	std::cout << std::endl << "on_listView_clicked row=" << index.row() << " column=" << index.column() << std::endl;
	
	auto item = model_for_listview.itemFromIndex(index);
	if (item != NULL)
	{
		auto filename = item->text();
		// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
		QByteArray ba = filename.toLocal8Bit();
		const char *filename_str = ba.data();
		std::cout << "text=" << filename_str << std::endl;
		open_volume(filename);
	}
	else
	{
		std::cout << "invalid index" << std::endl;
	}
}

void MainWindow::on_listView_2_clicked(const QModelIndex &index)
{
	std::cout << std::endl << "on_listView_2_clicked row=" << index.row() << " column=" << index.column() << std::endl;
	
    auto item = model_for_listview2.itemFromIndex(index);
    if (item != NULL)
    {
        auto filename = item->text();
        // get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters)
        QByteArray ba = filename.toLocal8Bit();
        const char *filename_str = ba.data();
        std::cout << "text=" << filename_str << std::endl;
		openTransferFunctionFromVoreenXML(filename_str);

		updateTFWidgetFromOpacityArrays();
		updateOpacityArrayFromTFWidget();
    }
    else
    {
        std::cout << "invalid index" << std::endl;
    }
}

void MainWindow::on_listView_2_doubleClicked(const QModelIndex &index)
{
	std::cout << std::endl << model_for_listview2.rowCount() << std::endl;
	for (int i = 0; i < model_for_listview2.rowCount();i++)
	{
		on_listView_2_clicked(model_for_listview2.index(i, 0));
		Sleeper::msleep(100);
	}
}

void MainWindow::on_listView_doubleClicked(const QModelIndex &index)
{
	std::cout << std::endl << model_for_listview.rowCount() << std::endl;
	for (int i = 0; i < model_for_listview.rowCount(); i++)
	{
		on_listView_clicked(model_for_listview.index(i, 0));
		Sleeper::msleep(100);
	}
}

void MainWindow::on_listView_activated(const QModelIndex &index)
{
    on_listView_clicked(index);
}

void MainWindow::on_listView_2_activated(const QModelIndex &index)
{
    on_listView_2_clicked(index);
}

void MainWindow::on_action_Reset_Renderer_triggered()
{
	auto window = vtk_widget.GetRenderWindow();
	auto collection = window->GetRenderers();
	auto item = collection->GetNextItem();
	int rc = 0;
	std::cout << "\nvtkRendererCollection\n";
	while (item != NULL)
	{
		item->ResetCamera();
		item->RemoveAllViewProps();
		item = collection->GetNextItem();
		std::cout << "\n reset vtkRenderer " << ++rc << std::endl;
	}
	window->Render();
}

void MainWindow::on_action_Remove_Renderer_triggered()
{
	// clean previous renderers and then add the current renderer
	auto window = vtk_widget.GetRenderWindow();
	auto collection = window->GetRenderers();
	auto item = collection->GetNextItem();
	int rc = 0;
	std::cout << "\nvtkRendererCollection\n";
	while (item != NULL)
	{
		window->RemoveRenderer(item);
		item = collection->GetNextItem();
		std::cout << "\n remove vtkRenderer " << ++rc << std::endl;
	}
	window->AddRenderer(renderer);
	window->Render();
}

void MainWindow::on_action_Open_dynamically_optimized_vortex_triggered()
{
    // a QList to be put in QStandardItemModel
    QList<QStandardItem *> filename_list, tf_list;
    DynamicVortex tvdata;

    for (auto i = tvdata.min_index(); i <= tvdata.max_index(); i++)
    {
        filename_list.append(new QStandardItem(QString::fromStdString(tvdata.volume(i))));
        tf_list.append(new QStandardItem(QString::fromStdString(tvdata.transferfunction(i))));

        // load volume
        open_volume_no_rendering(QString::fromStdString(tvdata.volume(i)));

        openTransferFunctionFromVoreenXML(tvdata.transferfunction(i).c_str());

        updateTFWidgetFromOpacityArrays();
        updateOpacityArrayFromTFWidget();

        Sleeper::msleep(100);
    }

    model_for_listview.clear();
    model_for_listview.appendColumn(filename_list);
    model_for_listview2.clear();
    model_for_listview2.appendColumn(tf_list);
}

void MainWindow::on_action_Open_vortex_triggered()
{
	// a QList to be put in QStandardItemModel
	QList<QStandardItem *> filename_list, tf_list;
	NaiveVortex tvdata;
	filename_list.append(new QStandardItem(QString::fromStdString(tvdata.volume(tvdata.min_index()))));
	tf_list.append(new QStandardItem(QString::fromStdString(tvdata.transferfunction(tvdata.min_index()))));

	for (auto i = tvdata.min_index(); i <= tvdata.max_index(); i++)
	{
		// load volume
		open_volume_no_rendering(QString::fromStdString(tvdata.volume(i)));

		openTransferFunctionFromVoreenXML(tvdata.transferfunction(i).c_str());

		updateTFWidgetFromOpacityArrays();
		updateOpacityArrayFromTFWidget();

		Sleeper::msleep(100);
	}

	model_for_listview.clear();
	model_for_listview.appendColumn(filename_list);
	model_for_listview2.clear();
	model_for_listview2.appendColumn(tf_list);
}

void MainWindow::on_action_Open_statically_optimized_vortex_triggered()
{
	// a QList to be put in QStandardItemModel
	QList<QStandardItem *> filename_list, tf_list;
	StaticVortex tvdata;
	filename_list.append(new QStandardItem(QString::fromStdString(tvdata.volume(tvdata.min_index()))));
	tf_list.append(new QStandardItem(QString::fromStdString(tvdata.transferfunction(tvdata.min_index()))));

	for (auto i = tvdata.min_index(); i <= tvdata.max_index(); i++)
	{
		// load volume
		open_volume_no_rendering(QString::fromStdString(tvdata.volume(i)));

		openTransferFunctionFromVoreenXML(tvdata.transferfunction(i).c_str());

		updateTFWidgetFromOpacityArrays();
		updateOpacityArrayFromTFWidget();

		Sleeper::msleep(100);
	}

	model_for_listview.clear();
	model_for_listview.appendColumn(filename_list);
	model_for_listview2.clear();
	model_for_listview2.appendColumn(tf_list);
}
