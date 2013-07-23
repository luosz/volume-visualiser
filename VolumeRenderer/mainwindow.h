#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

#include <iostream>
#include <memory>
#include <cstdlib>
#include <vector>

#include <QVTKWidget.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkMetaImageReader.h>
#include <vtkVolume16Reader.h>
#include <vtkNew.h>
#include <vtkNrrdReader.h>
#include <vtkImageShiftScale.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRendererCollection.h>
#include <vtkImageReader2.h>
#include <vtkImageAccumulate.h>
#include <vtkIntArray.h>
#include <vtkBarChartActor.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkProperty2D.h>
#include <vtkLegendBoxActor.h>

#include "ctkTransferFunction.h"
#include "ctkVTKColorTransferFunction.h"
#include "ctkTransferFunctionView.h"
#include "ctkTransferFunctionGradientItem.h"
#include "ctkTransferFunctionControlPointsItem.h"
#include "ctkVTKVolumePropertyWidget.h"

#include "tinyxml2.h"

#include "ui_mainwindow.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	Ui::MainWindow *ui;

	QString volume_filename;
	QString transfer_function_filename;
	vtkSmartPointer<vtkRenderWindowInteractor> interactor;
	vtkSmartPointer<vtkRenderer> renderer;
	QVTKWidget widget;
	QVTKWidget histogramWidget;
	ctkVTKVolumePropertyWidget volumePropertywidget;
	std::vector<double> intensity_list;
	std::vector<std::vector<int>> colour_list;
	vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction;
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction;
	double lower_bound;
	double upper_bound;
	double epsilon;

	double get_area(int i)
	{
		if (i >= 0 && i+1 < intensity_list.size())
		{
			double i_a = intensity_list[i]; // intensity of point a
			double i_b = intensity_list[i+1]; // intensity of point b
			double o_a = colour_list[i][3]; // opacity of point a
			double o_b = colour_list[i+1][3]; // opacity of point b
			return (o_a + o_b) * (i_b - i_a) / 2;
		} 
		else
		{
			return 0;
		}
	}

	double get_neighbour_area(int i)
	{
		return get_area(i) + get_area(i-1);
	}

	double get_area_right_new(int i, double opacity_new)
	{
		if (i >= 0 && i+1 < intensity_list.size())
		{
			double i_a = intensity_list[i]; // intensity of point a
			double i_b = intensity_list[i+1]; // intensity of point b
			double o_a = opacity_new; // opacity of point a
			double o_b = colour_list[i+1][3]; // opacity of point b
			return (o_a + o_b) * (i_b - i_a) / 2;
		} 
		else
		{
			return 0;
		}
	}

	double get_area_left_new(int i, double opacity_new)
	{
		if (i >= 0 && i+1 < intensity_list.size())
		{
			double i_a = intensity_list[i]; // intensity of point a
			double i_b = intensity_list[i+1]; // intensity of point b
			double o_a = colour_list[i][3]; // opacity of point a
			double o_b = opacity_new; // opacity of point b
			return (o_a + o_b) * (i_b - i_a) / 2;
		} 
		else
		{
			return 0;
		}
	}

	double get_neighbour_area_new(int i, double opacity_new)
	{
		return get_area_right_new(i, opacity_new) + get_area_left_new(i-1, opacity_new);
	}

	void optimiseTransferFunction()
	{
		std::cout<<"list size="<<colour_list.size()<<endl;
		std::vector<double> area_list;
		for (unsigned int i=0; i<intensity_list.size()-1; i++)
		{
			double area = get_area(i);
			std::cout<<"area"<<i<<"="<<area<<endl;
			area_list.push_back(area);
		}
		if (area_list.size() > 0)
		{
			int max_index = 0;
			double max_area = area_list[0];
			int min_index = 0;
			double min_area = area_list[0];
			for (unsigned int i=1; i<area_list.size(); i++)
			{
				if (area_list[i] > max_area)
				{
					max_area = area_list[i];
					max_index = i;
				}
				if (area_list[i] > 0 && area_list[i] < min_area)
				{
					min_area = area_list[i];
					min_index = i;
				}
			}
			std::cout<<"max index="<<max_index<<" area="<<max_area<<endl;
			std::cout<<"min index="<<min_index<<" area="<<min_area<<endl;
			if (min_area > 0 && max_area > min_area)
			{
				int a = min_index;
				int b = min_index + 1;
				int c = max_index;
				int d = max_index + 1;
				int min_point_index = colour_list[b][3] < colour_list[a][3] ? b : a;
				int max_point_index = colour_list[d][3] > colour_list[c][3] ? d : c;
				const int step = 1;
				double new_max = colour_list[max_point_index][3] - step;
				double area_decreased = get_neighbour_area(max_point_index) - get_neighbour_area_new(max_point_index, new_max);
				std::cout<<"opacity="<<colour_list[max_point_index][3]<<" new opacity="<<new_max<<" area decreased="<<area_decreased<<endl;
			}
		}
	}

	void saveTransferFunction(const char *filename)
	{
		tinyxml2::XMLDocument doc;

		auto declaration = doc.NewDeclaration();
		doc.InsertEndChild(declaration);
		auto voreenData = doc.NewElement("VoreenData");
		voreenData->SetAttribute("version", 1);
		auto transFuncIntensity = doc.NewElement("TransFuncIntensity");
		transFuncIntensity->SetAttribute("type", "TransFuncIntensity");
		auto keys = doc.NewElement("Keys");
		for (unsigned int i=0; i<intensity_list.size(); i++)
		{
			auto key = doc.NewElement("key");
			key->SetAttribute("type", "TransFuncMappingKey");
			auto intensity = doc.NewElement("intensity");
			intensity->SetAttribute("value", intensity_list[i]);
			auto split = doc.NewElement("split");
			split->SetAttribute("value", false);
			auto colorL = doc.NewElement("colorL");
			colorL->SetAttribute("r", colour_list[i][0]);
			colorL->SetAttribute("g", colour_list[i][1]);
			colorL->SetAttribute("b", colour_list[i][2]);
			colorL->SetAttribute("a", colour_list[i][3]);
			key->InsertEndChild(intensity);
			key->InsertEndChild(split);
			key->InsertEndChild(colorL);
			keys->InsertEndChild(key);
		}
		auto lower = doc.NewElement("lower");
		lower->SetAttribute("value", lower_bound);
		auto upper = doc.NewElement("upper");
		upper->SetAttribute("value", upper_bound);
		transFuncIntensity->InsertEndChild(keys);
		transFuncIntensity->InsertEndChild(lower);
		transFuncIntensity->InsertEndChild(upper);
		voreenData->InsertEndChild(transFuncIntensity);
		doc.InsertEndChild(voreenData);

		auto r = doc.SaveFile(filename);
		if (r != tinyxml2::XML_NO_ERROR)
		{
			std::cout<<"failed to save file"<<endl;
		}
	}

	void openTransferFunction(const char *filename)
	{
		tinyxml2::XMLDocument doc;
		auto r = doc.LoadFile(filename);

		if (r != tinyxml2::XML_NO_ERROR)
		{
			std::cout<<"failed to open file"<<endl;
			return;
		}

		auto key = doc.FirstChildElement("VoreenData")->FirstChildElement("TransFuncIntensity")->FirstChildElement("Keys")->FirstChildElement("key");

		intensity_list.clear();
		colour_list.clear();

		do 
		{
			double intensity = atof(key->FirstChildElement("intensity")->Attribute("value"));
			intensity_list.push_back(intensity);
			int r = atoi(key->FirstChildElement("colorL")->Attribute("r"));
			int g = atoi(key->FirstChildElement("colorL")->Attribute("g"));
			int b = atoi(key->FirstChildElement("colorL")->Attribute("b"));
			int a = atoi(key->FirstChildElement("colorL")->Attribute("a"));
			std::vector<int> colour;
			colour.push_back(r);
			colour.push_back(g);
			colour.push_back(b);
			colour.push_back(a);
			colour_list.push_back(colour);

			bool split = (0 == strcmp("true", key->FirstChildElement("split")->Attribute("value")));
			std::cout<<"intensity="<<intensity;
			std::cout<<"\tsplit="<<(split?"true":"false");
			std::cout<<"\tcolorL r="<<r<<" g="<<g<<" b="<<b<<" a="<<a;
			if (split)
			{
				intensity_list.push_back(intensity + epsilon);
				auto colorR = key->FirstChildElement("colorR");
				int r2 = atoi(colorR->Attribute("r"));
				int g2 = atoi(colorR->Attribute("g"));
				int b2 = atoi(colorR->Attribute("b"));
				int a2 = atoi(colorR->Attribute("a"));
				std::vector<int> colour2;
				colour2.push_back(r2);
				colour2.push_back(g2);
				colour2.push_back(b2);
				colour2.push_back(a2);
				colour_list.push_back(colour2);
				std::cout<<"\tcolorR r="<<r2<<" g="<<g2<<" b="<<b2<<" a="<<a2;
			}
			std::cout<<endl;

			key = key->NextSiblingElement();
		} while (key);

		auto transFuncIntensity = doc.FirstChildElement("VoreenData")->FirstChildElement("TransFuncIntensity");
		lower_bound = atof(transFuncIntensity->FirstChildElement("lower")->Attribute("value"));
		upper_bound = atof(transFuncIntensity->FirstChildElement("upper")->Attribute("value"));
	}

	void updateTransferFunction()
	{
		if (intensity_list.size() > 0 && intensity_list.size() == colour_list.size())
		{
			opacityTransferFunction->RemoveAllPoints();
			colorTransferFunction->RemoveAllPoints();
			const double MAX = 255;
			for (unsigned int i=0; i<intensity_list.size(); i++)
			{
				opacityTransferFunction->AddPoint(intensity_list[i]*MAX, colour_list[i][3]/MAX);
				colorTransferFunction->AddRGBPoint(intensity_list[i]*MAX, colour_list[i][0]/MAX, colour_list[i][1]/MAX, colour_list[i][2]/MAX);
			}
		}
		else
		{
			opacityTransferFunction->RemoveAllPoints();
			colorTransferFunction->RemoveAllPoints();

			opacityTransferFunction->AddPoint(0.0,  0.0);
			opacityTransferFunction->AddPoint(36.0,  0.125);
			opacityTransferFunction->AddPoint(72.0,  0.25);
			opacityTransferFunction->AddPoint(108.0, 0.375);
			opacityTransferFunction->AddPoint(144.0, 0.5);
			opacityTransferFunction->AddPoint(180.0, 0.625);
			opacityTransferFunction->AddPoint(216.0, 0.75);
			opacityTransferFunction->AddPoint(255.0, 0.875);

			colorTransferFunction->AddRGBPoint(0.0,  0.0, 0.0, 0.0);
			colorTransferFunction->AddRGBPoint(36.0, 1.0, 0.0, 0.0);
			colorTransferFunction->AddRGBPoint(72.0, 1.0, 1.0, 0.0);
			colorTransferFunction->AddRGBPoint(108.0, 0.0, 1.0, 0.0);
			colorTransferFunction->AddRGBPoint(144.0, 0.0, 1.0, 1.0);
			colorTransferFunction->AddRGBPoint(180.0, 0.0, 0.0, 1.0);
			colorTransferFunction->AddRGBPoint(216.0, 1.0, 0.0, 1.0);
			colorTransferFunction->AddRGBPoint(255.0, 1.0, 1.0, 1.0);
		}
	}

	void generateHistogram(vtkSmartPointer<vtkImageReader2> reader)
	{
		int bins = 16;
		vtkSmartPointer<vtkImageAccumulate> histogram = vtkSmartPointer<vtkImageAccumulate>::New();
		histogram->SetInputConnection(reader->GetOutputPort());
		histogram->SetComponentExtent(0, bins-1, 0, 0, 0, 0);
		histogram->SetComponentOrigin(0, 0, 0);
		histogram->SetComponentSpacing(256/bins, 0, 0);
		histogram->Update();
		vtkSmartPointer<vtkIntArray> frequencies = vtkSmartPointer<vtkIntArray>::New();
		frequencies->SetNumberOfComponents(1);
		frequencies->SetNumberOfTuples(bins);
		int * output = static_cast<int *>(histogram->GetOutput()->GetScalarPointer());

		for(int j = 0; j < bins; ++j)
		{
			frequencies->SetTuple1(j, *output++);
		}

		vtkSmartPointer<vtkDataObject> dataObject = vtkSmartPointer<vtkDataObject>::New();
		dataObject->GetFieldData()->AddArray( frequencies );
		vtkSmartPointer<vtkBarChartActor> barChart = vtkSmartPointer<vtkBarChartActor>::New();

		barChart->SetInput(dataObject);
		barChart->SetTitle("Histogram");
		barChart->GetPositionCoordinate()->SetValue(0.05,0.05,0.0);
		barChart->GetPosition2Coordinate()->SetValue(0.95,0.95,0.0);
		barChart->GetProperty()->SetColor(1,1,1);
		barChart->GetLegendActor()->SetNumberOfEntries(dataObject->GetFieldData()->GetArray(0)->GetNumberOfTuples());
		barChart->LegendVisibilityOff();
		barChart->LabelVisibilityOff();

		double red[3] = {1, 0, 0 };
		for(int i = 0; i < bins; ++i)
		{
			barChart->SetBarColor(i, red );
		}

		vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
		renderer->AddActor(barChart);
		vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
		//auto renderWindow = histogramWidget.GetRenderWindow();
		renderWindow->AddRenderer(renderer);
		renderWindow->SetSize(640, 480);
		vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
		interactor->SetRenderWindow(renderWindow);
		//interactor->SetInteractorStyle(vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New());
		renderWindow->Render();
		interactor->Initialize();
		interactor->Start();
	}

	private slots:
		void onAboutSlot()
		{
			QMessageBox msgBox;
			msgBox.setText(QString::fromUtf8("Volume Renderer"));
			msgBox.exec();
		}

		void onExitSlot()
		{
			qApp->quit();
		}

		void onOpenVolumeSlot()
		{
			// show file dialog
			QString filter("Meta image file (*.mhd *.mha)");
			volume_filename = QFileDialog::getOpenFileName(this, QString(tr("Open a volume data set")), volume_filename, filter); 
			if (volume_filename.isEmpty())
				return;

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
			reader->SetDataDimensions (512, 512);
			reader->SetImageRange (1, 361);
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

			generateHistogram(reader);
		}

		void onAppendVolumeSlot()
		{
			// show file dialog
			QString filter("Meta image file (*.mhd *.mha)");
			volume_filename = QFileDialog::getOpenFileName(this, QString(tr("Open a volume data set")), volume_filename, filter); 
			if (volume_filename.isEmpty())
				return;

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
			reader->SetDataDimensions (512, 512);
			reader->SetImageRange (1, 361);
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

		void onOpenTransferFunctionSlot()
		{
			// show file dialog
			QString filter("transfer function file (*.tfi *.tfig)");
			transfer_function_filename = QFileDialog::getOpenFileName(this, QString(tr("Open a volume data set")), transfer_function_filename, filter); 
			if (transfer_function_filename.isEmpty())
				return;

			//// show filename on window title
			//this->setWindowTitle(QString::fromUtf8("Volume Renderer - ") + volume_filename);

			// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
			QByteArray ba = transfer_function_filename.toLocal8Bit();  
			const char *filename_str = ba.data();

			std::cout<<"transfer function file: "<<filename_str<<endl;

			openTransferFunction(filename_str);
			updateTransferFunction();
		}

		void onSaveTransferFunctionSlot()
		{
			// show file dialog
			QString filter("transfer function file (*.tfi *.tfig)");
			transfer_function_filename = QFileDialog::getSaveFileName(this, QString(tr("Open a volume data set")), transfer_function_filename, filter); 
			if (transfer_function_filename.isEmpty())
				return;

			//// show filename on window title
			//this->setWindowTitle(QString::fromUtf8("Volume Renderer - ") + volume_filename);

			// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
			QByteArray ba = transfer_function_filename.toLocal8Bit();  
			const char *filename_str = ba.data();

			std::cout<<"transfer function file: "<<filename_str<<endl;

			saveTransferFunction(filename_str);
			//updateTransferFunction();
		}

		void onOptimiseTransferFunctionSlot()
		{
			optimiseTransferFunction();
		}
};

#endif // MAINWINDOW_H
