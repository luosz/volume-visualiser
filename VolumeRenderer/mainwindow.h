#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

#include <iostream>
#include <memory>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <limits>
#include <cmath>

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
#include <vtkImageExtractComponents.h>

#include "ctkTransferFunction.h"
#include "ctkVTKColorTransferFunction.h"
#include "ctkTransferFunctionView.h"
#include "ctkTransferFunctionGradientItem.h"
#include "ctkTransferFunctionControlPointsItem.h"
#include "ctkVTKVolumePropertyWidget.h"

#include "tinyxml2.h"
#include "ui_mainwindow.h"
#define OUTPUT_TO_FILE

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
	ctkVTKVolumePropertyWidget volumePropertywidget;
	std::vector<double> intensity_list;
	std::vector<std::vector<double>> colour_list;
	std::vector<double> frequency_list;
	vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction;
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction;
	double lower_bound, upper_bound;
	double x_max, x_min, y_max, y_min;
	int count_of_voxels;

	QGraphicsScene * getGraphicsScene()
	{
		QGraphicsScene *scene = ui->graphicsView->scene();
		if (scene == NULL)
		{
			scene = new QGraphicsScene();
			ui->graphicsView->setScene(scene);
			std::cout<<"create a new scene"<<std::endl;
		}
		return scene;
	}

	/// Re-maps a number from one range to another.
	double map_to_range(double n, double lower, double upper, double target_lower, double target_upper)
	{
		n = n < lower ? lower : n;
		n = n > upper ? upper : n;
		double normalised = (n - lower) / (upper - lower);
		return normalised * (target_upper - target_lower) + target_lower;
	}

	double denormalise_intensity(double n)
	{
		return map_to_range(n, lower_bound, upper_bound, 0, 255);
	}

	double normalise_intensity(double n)
	{
		return map_to_range(n, 0, 255, lower_bound, upper_bound);
	}
	
	double normalise_rgba(int n)
	{
		return map_to_range(n, 0, 255, 0, 1);
	}

	int denormalise_rgba(double n)
	{
		return map_to_range(n, 0, 1, 0, 255);
	}

	double get_opacity(int i)
	{
		return colour_list[i][3];
	}

	double get_frequency(double intensity) // intensity belongs to [0,255]
	{
		int intensity_int = (int)intensity;
		const int max = 255;
		if (intensity_int >= 0 && intensity_int < max)
		{
			// linear interpolation
			double t = intensity - intensity_int;
			double a = frequency_list[intensity_int];
			double b = frequency_list[intensity_int+1];
			return (a + (b - a) * t);
		} 
		else
		{
			if (intensity_int == max)
			{
				return frequency_list[intensity_int];
			} 
			else
			{
				std::cout<<"Errors occur in get_frequency()"<<std::endl;
				return 0;
			}
		}
	}

	double get_frequency_and_opacity(int i)
	{
		double intensity = denormalise_intensity(intensity_list[i]);
		return get_frequency(intensity) * get_opacity(i);
	}

	double get_noteworthiness(int i)
	{
		const double epsilon = 1e-6;
		double intensity = denormalise_intensity(intensity_list[i]);
		double probability = get_frequency(intensity) / count_of_voxels;
		if (probability > epsilon)
		{
			return get_opacity(i) * probability * (-log(probability));
		} 
		else
		{
			return 0;
		}
	}

	double get_visibility(int i)
	{
		return get_noteworthiness(i);
	}

	/// double intensity belongs to [0,1]
	double get_opacity_interpolation_without_index(double intensity)
	{
		int index;
		if (intensity < intensity_list[0])
		{
			index = -1;
		} 
		else
		{
			if (intensity > intensity_list[intensity_list.size()-1])
			{
				index = intensity_list.size() - 1;
			} 
			else
			{
				index = -2;
				for (int i=0; i<intensity_list.size()-1; i++)
				{
					if (intensity_list[i] <= intensity && intensity_list[i+1] >= intensity)
					{
						index = i;
						break;
					}
				}
				if (index == -2)
				{
					std::cout<<"Errors occur in get_opacity_interpolation_without_index(double intensity)"<<std::endl;
					return 0;
				}
			}
		}

		return get_opacity_interpolation(intensity, index);
	}

	/// double intensity belongs to [0,1]
	/// int index >=0 && index < intensity_list.size()
	double get_opacity_interpolation(double intensity, int index)
	{
		int i1 = index, i2 = index + 1;
		if (i1 >= 0 && i2 < intensity_list.size())
		{
			// linear interpolation
			double t = (intensity - intensity_list[i1]) / (intensity_list[i2] - intensity_list[i1]);
			double a = colour_list[i1][3];
			double b = colour_list[i2][3];
			return (a + (b - a) * t);
		}
		else
		{
			if (i1 == -1)
			{
				return intensity_list[i2];
			} 
			else
			{
				if (i1 == intensity_list.size() - 1)
				{
					return intensity_list[i1];
				} 
				else
				{
					std::cout<<"Errors occur in get_opacity_interpolation()"<<std::endl;
					return 0;
				}
			}
		}
	}

	double get_area_integral(int index)
	{
		double a, b;
		if (index >= 0 && index < intensity_list.size() - 1)
		{
			a = intensity_list[index];
			b = intensity_list[index+1];
		} 
		else
		{
			if (index == -1)
			{
				a = 0;
				b = intensity_list[index+1];
			} 
			else
			{
				if (index == intensity_list.size() - 1)
				{
					a = intensity_list[index];
					b = 1;
				} 
				else
				{
					std::cout<<"index out of range in get_area_integral()"<<endl;
					return 0;
				}
			}
		}

		std::cout<<"intensity "<<a<<" "<<b;
		a = denormalise_intensity(a);
		b = denormalise_intensity(b);
		std::cout<<" map to [0, 255] "<<a<<" "<<b<<std::endl;

		double sum = 0;
		// int intensity belongs to [0,255]
		for (int intensity=(int)a; intensity<b; intensity++)
		{
			if (intensity >= a)
			{
				std::cout<<intensity<<" ";
				sum += get_entropy(intensity, index);
			}
		}
		std::cout<<std::endl;
		return sum;
	}

	// double intensity belongs to [0,255]
	double get_entropy(double intensity, int index)
	{
		double frequency = get_frequency(intensity);
		const double epsilon = 1e-6;
		double probability = frequency / count_of_voxels;
		if (probability > epsilon)
		{
			double normalised = normalise_intensity(intensity);
			return get_opacity_interpolation(normalised, index) * probability * (-log(probability));
		}
		else
		{
			return 0;
		}
	}

	// double intensity belongs to [0,255]
	double get_entropy(double intensity)
	{
		double frequency = get_frequency(intensity);
		const double epsilon = 1e-6;
		double probability = frequency / count_of_voxels;
		if (probability > epsilon)
		{
			double normalised = normalise_intensity(intensity);
			return get_opacity_interpolation_without_index(normalised) * probability * (-log(probability));
		}
		else
		{
			return 0;
		}
	}

	double get_area(int i)
	{
		if (i >= 0 && i < intensity_list.size() - 1)
		{
			// area of a trapezoid
			double h = intensity_list[i+1] - intensity_list[i];
			double a = get_visibility(i);//colour_list[i][3];
			double b = get_visibility(i+1);//colour_list[i+1][3];
			return (a + b) * h / 2;
		} 
		else
		{
			if (i == -1)
			{
				return (intensity_list[i+1] - lower_bound) * get_visibility(i+1);//colour_list[i+1][3];
			} 
			else
			{
				if (i == intensity_list.size() - 1)
				{
					return (upper_bound - intensity_list[i]) * get_visibility(i);//colour_list[i][3];
				} 
				else
				{
					std::cout<<"index out of range in get_area()"<<endl;
					return 0;
				}
			}
		}
	}

	double get_neighbour_area(int i)
	{
		return get_area(i) + get_area(i - 1);
	}

	double get_height_given_area_increment(int i, double area_increment)
	{
		double visibility_increment = -1;
		if (i > 0  && i < intensity_list.size() - 1)
		{
			// area of two triangles
			double a = intensity_list[i] - intensity_list[i-1];
			double b = intensity_list[i+1] - intensity_list[i];
			visibility_increment = 2 * area_increment / (a + b);
		}
		else
		{
			if (i == 0)
			{
				// area of a rectangle (left) and a triangle (right)
				double a = intensity_list[i] - lower_bound;
				double b = intensity_list[i+1] - intensity_list[i];
				visibility_increment = area_increment / (a + b/2);
			} 
			else
			{
				if (i == intensity_list.size() - 1)
				{
					// area of a triangle (left) and a rectangle (right)
					double a = intensity_list[i] - intensity_list[i-1];
					double b = upper_bound - intensity_list[i];
					visibility_increment = area_increment / (a/2 + b);
				} 
				else
				{
					std::cout<<"index out of range in get_height_given_area_increment()"<<endl;
					visibility_increment = 0;
				}
			}
		}
		double visibility = get_visibility(i);
		double opacity = colour_list[i][3];
		return visibility_increment * opacity / visibility;
	}

	void optimiseTransferFunction()
	{
		std::cout<<"colour_list size="<<colour_list.size()
			<<" intensity_list size="<<intensity_list.size()<<std::endl;
		int max_index = -1;
		int min_index = -1;
		double max_area = std::numeric_limits<int>::min();
		double min_area = std::numeric_limits<int>::max();
		const double epsilon = 1./256.;
		for (unsigned int i=0; i<intensity_list.size(); i++)
		{
			if (colour_list[i][3] > epsilon)
			{
				double area = get_neighbour_area(i);
				if (area > max_area)
				{
					max_index = i;
					max_area = area;
				}
				if (area < min_area)
				{
					min_index = i;
					min_area = area;
				}
			}
		}
		if (min_index != max_index)
		{
			const double step_size = 1./255.;
			double height_max = colour_list[max_index][3];
			double height_max_new = height_max - step_size;
			height_max_new = height_max_new < 0 ? 0 : height_max_new;
			double area = get_neighbour_area(max_index);
			colour_list[max_index][3] = height_max_new;
			double new_area = get_neighbour_area(max_index);
			double area_decreased = area - new_area;
			double height_increased = get_height_given_area_increment(min_index, area_decreased);
			double height_min = colour_list[min_index][3];
			double height_min_new = height_min + height_increased;
			height_min_new = height_min_new > 1 ? 1 : height_min_new;
			colour_list[min_index][3] = height_min_new;
			std::cout<<"max index="<<max_index<<" min index="<<min_index<<" opacity="<<height_max<<" new opacity="<<height_max_new<<" area="<<area<<" new area="<<new_area<<" height="<<height_min<<" new height="<<height_min_new<<endl;
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
			colorL->SetAttribute("r", denormalise_rgba(colour_list[i][0]));
			colorL->SetAttribute("g", denormalise_rgba(colour_list[i][1]));
			colorL->SetAttribute("b", denormalise_rgba(colour_list[i][2]));
			colorL->SetAttribute("a", denormalise_rgba(colour_list[i][3]));
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
			std::vector<double> colour;
			colour.push_back(normalise_rgba(r));
			colour.push_back(normalise_rgba(g));
			colour.push_back(normalise_rgba(b));
			colour.push_back(normalise_rgba(a));
			colour_list.push_back(colour);

			bool split = (0 == strcmp("true", key->FirstChildElement("split")->Attribute("value")));
			std::cout<<"intensity="<<intensity;
			std::cout<<"\tsplit="<<(split?"true":"false");
			std::cout<<"\tcolorL r="<<r<<" g="<<g<<" b="<<b<<" a="<<a;
			const double epsilon = 1e-6;
			if (split)
			{
				intensity_list.push_back(intensity + epsilon);
				auto colorR = key->FirstChildElement("colorR");
				int r2 = atoi(colorR->Attribute("r"));
				int g2 = atoi(colorR->Attribute("g"));
				int b2 = atoi(colorR->Attribute("b"));
				int a2 = atoi(colorR->Attribute("a"));
				std::vector<double> colour2;
				colour2.push_back(normalise_rgba(r2));
				colour2.push_back(normalise_rgba(g2));
				colour2.push_back(normalise_rgba(b2));
				colour2.push_back(normalise_rgba(a2));
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

	void generateDefaultTransferFunction()
	{
		//// Create transfer mapping scalar value to opacity.
		//opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
		//opacityTransferFunction->AddPoint(0.0,  0.0);
		//opacityTransferFunction->AddPoint(36.0,  0.125);
		//opacityTransferFunction->AddPoint(72.0,  0.25);
		//opacityTransferFunction->AddPoint(108.0, 0.375);
		//opacityTransferFunction->AddPoint(144.0, 0.5);
		//opacityTransferFunction->AddPoint(180.0, 0.625);
		//opacityTransferFunction->AddPoint(216.0, 0.75);
		//opacityTransferFunction->AddPoint(255.0, 0.875);

		//// Create transfer mapping scalar value to color.
		//colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
		//colorTransferFunction->AddRGBPoint(0.0,  0.0, 0.0, 0.0);
		//colorTransferFunction->AddRGBPoint(36.0, 1.0, 0.0, 0.0);
		//colorTransferFunction->AddRGBPoint(72.0, 1.0, 1.0, 0.0);
		//colorTransferFunction->AddRGBPoint(108.0, 0.0, 1.0, 0.0);
		//colorTransferFunction->AddRGBPoint(144.0, 0.0, 1.0, 1.0);
		//colorTransferFunction->AddRGBPoint(180.0, 0.0, 0.0, 1.0);
		//colorTransferFunction->AddRGBPoint(216.0, 1.0, 0.0, 1.0);
		//colorTransferFunction->AddRGBPoint(255.0, 1.0, 1.0, 1.0);

		intensity_list.clear();
		colour_list.clear();
		lower_bound = 0;
		upper_bound = 1;

		intensity_list.push_back(normalise_intensity(0));
		intensity_list.push_back(normalise_intensity(36));
		intensity_list.push_back(normalise_intensity(72));
		intensity_list.push_back(normalise_intensity(108));
		intensity_list.push_back(normalise_intensity(144));
		intensity_list.push_back(normalise_intensity(180));
		intensity_list.push_back(normalise_intensity(216));
		intensity_list.push_back(normalise_intensity(255));
		{
			std::vector<double> v;
			v.push_back(0);
			v.push_back(0);
			v.push_back(0);
			v.push_back(0);
			colour_list.push_back(v);
		}
		{
			std::vector<double> v;
			v.push_back(1);
			v.push_back(0);
			v.push_back(0);
			v.push_back(0.125);
			colour_list.push_back(v);
		}
		{
			std::vector<double> v;
			v.push_back(1);
			v.push_back(1);
			v.push_back(0);
			v.push_back(0.25);
			colour_list.push_back(v);
		}
		{
			std::vector<double> v;
			v.push_back(0);
			v.push_back(1);
			v.push_back(0);
			v.push_back(0.375);
			colour_list.push_back(v);
		}
		{
			std::vector<double> v;
			v.push_back(0);
			v.push_back(1);
			v.push_back(1);
			v.push_back(0.5);
			colour_list.push_back(v);
		}
		{
			std::vector<double> v;
			v.push_back(0);
			v.push_back(0);
			v.push_back(1);
			v.push_back(0.625);
			colour_list.push_back(v);
		}
		{
			std::vector<double> v;
			v.push_back(1);
			v.push_back(0);
			v.push_back(1);
			v.push_back(0.75);
			colour_list.push_back(v);
		}
		{
			std::vector<double> v;
			v.push_back(1);
			v.push_back(1);
			v.push_back(1);
			v.push_back(0);
			colour_list.push_back(v);
		}
	}

	void updateTransferFunctionWidgetsFromArrays()
	{
		if (intensity_list.size() == 0 || colour_list.size() == 0)
		{
			generateDefaultTransferFunction();
		}
		if (intensity_list.size() > 0 && intensity_list.size() == colour_list.size())
		{
			opacityTransferFunction->RemoveAllPoints();
			colorTransferFunction->RemoveAllPoints();
			for (unsigned int i=0; i<intensity_list.size(); i++)
			{
				opacityTransferFunction->AddPoint(denormalise_intensity(intensity_list[i]), colour_list[i][3]);
				colorTransferFunction->AddRGBPoint(denormalise_intensity(intensity_list[i]), colour_list[i][0], colour_list[i][1], colour_list[i][2]);
			}
		}
	}

	void updateTransferFunctionArraysFromWidgets()
	{
		if (colorTransferFunction->GetSize() < 1)
		{
			QMessageBox msgBox;
			msgBox.setText("Error: vtkColorTransferFunction is empty.");
			int ret = msgBox.exec();
			return;
		}
		if (colorTransferFunction->GetSize() != opacityTransferFunction->GetSize())
		{
			QMessageBox msgBox;
			msgBox.setText("Error: vtkColorTransferFunction and vtkPiecewiseFunction should have the same size, but they do not.");
			int ret = msgBox.exec();
			return;
		}

		std::cout<<"update transfer function from widget"<<std::endl;
		colour_list.clear();
		intensity_list.clear();
		for (unsigned int i=0; i<colorTransferFunction->GetSize(); i++)
		{
			double xrgb[6];
			colorTransferFunction->GetNodeValue(i, xrgb);
			double xa[4];
			opacityTransferFunction->GetNodeValue(i, xa);
			double opacity = xa[1];
			double intensity = normalise_intensity(xrgb[0]);
			std::vector<double> c;
			c.push_back(xrgb[1]);
			c.push_back(xrgb[2]);
			c.push_back(xrgb[3]);
			c.push_back(opacity);
			colour_list.push_back(c);
			intensity_list.push_back(intensity);
			//std::cout<<"xrgba "<<xrgb[0]<<" "<<xrgb[1]<<" "<<xrgb[2]<<" "<<xrgb[3]<<" "<<opacity<<" "<<denormalise_intensity(opacity)<<std::endl;
			std::cout<<"x & opacity "<<intensity<<" "<<opacity<<" "<<denormalise_intensity(opacity)<<std::endl;
		}
	}

	void generateVisibilityFunction(vtkSmartPointer<vtkImageAlgorithm> reader)
	{
		int ignoreZero = 0;
		int numComponents = reader->GetOutput()->GetNumberOfScalarComponents();
		std::cout<<"component number="<<numComponents<<endl;
		if( numComponents > 3 )
		{
			std::cout << "Error: cannot process an image with " 
				<< numComponents << " components!" << std::endl;
			//return EXIT_FAILURE;
		}

		double xmax = 0.;
		double ymax = 0.;

		//vtkSmartPointer<vtkIntArray> redFrequencies = 
		//	vtkSmartPointer<vtkIntArray>::New();
		//vtkSmartPointer<vtkIntArray> greenFrequencies = 
		//	vtkSmartPointer<vtkIntArray>::New();
		//vtkSmartPointer<vtkIntArray> blueFrequencies = 
		//	vtkSmartPointer<vtkIntArray>::New();

		// Process the image, extracting and plotting a histogram for each
		// component
		for( int i = 0; i < numComponents; ++i )
		{
			vtkSmartPointer<vtkImageExtractComponents> extract = 
				vtkSmartPointer<vtkImageExtractComponents>::New();
			extract->SetInputConnection( reader->GetOutputPort() );
			extract->SetComponents( i );
			extract->Update();

			double range[2];
			extract->GetOutput()->GetScalarRange( range );
			std::cout<<"range "<<range[0]<<" "<<range[1]<<endl;

			vtkSmartPointer<vtkImageAccumulate> histogram = 
				vtkSmartPointer<vtkImageAccumulate>::New();
			histogram->SetInputConnection( extract->GetOutputPort() );
			histogram->SetComponentExtent(
				0,
				static_cast<int>(range[1])-static_cast<int>(range[0])-1,0,0,0,0 );
			histogram->SetComponentOrigin( range[0],0,0 );
			histogram->SetComponentSpacing( 1,0,0 );
			histogram->SetIgnoreZero( ignoreZero );
			histogram->Update();

			//vtkIntArray* currentArray = 0;
			//if( i == 0 )
			//{
			//	currentArray = redFrequencies;
			//}
			//else if( i == 1 )
			//{
			//	currentArray = greenFrequencies;
			//}
			//else
			//{
			//	currentArray = blueFrequencies;
			//}

			//currentArray->SetNumberOfComponents(1);
			//currentArray->SetNumberOfTuples( 256 );
			//int* output = static_cast<int*>(histogram->GetOutput()->GetScalarPointer());

			//for( int j = 0; j < 256; ++j )
			//{
			//	currentArray->SetTuple1( j, *output++ );
			//}

			x_min = range[0];
			x_max = range[1];
			if( range[1] > xmax ) 
			{ 
				xmax = range[1];
			}
			if( histogram->GetOutput()->GetScalarRange()[1] > ymax ) 
			{
				ymax = histogram->GetOutput()->GetScalarRange()[1];
			}
			std::cout<<"histogram range "<<histogram->GetOutput()->GetScalarRange()[0]<<" "<<histogram->GetOutput()->GetScalarRange()[1]<<endl;

//#if VTK_MAJOR_VERSION <= 5
//			plot->AddInput( histogram->GetOutput() );
//#else
//			plot->AddDataSetInputConnection( histogram->GetOutputPort() );
//#endif
//			if( numComponents > 1 )
//			{
//				plot->SetPlotColor(i,colors[i]);
//				plot->SetPlotLabel(i,labels[i]);
//				plot->LegendOn();
//			}

			//if (i == 0)
			{
				y_min = histogram->GetOutput()->GetScalarRange()[0];
				y_max = histogram->GetOutput()->GetScalarRange()[1];
				std::cout<<"min="<<y_min<<" max="<<y_max<<endl;

#ifdef OUTPUT_TO_FILE
				char buffer[32];
				itoa(i, buffer, 10);
				char filename[32];
				sprintf(filename, "../%s.csv", buffer);
				std::cout<<"output file "<<filename<<std::endl;
				std::ofstream myfile(filename);
#endif

				frequency_list.clear();
				frequency_list.reserve(256);
				int* pixels = static_cast<int*>(histogram->GetOutput()->GetScalarPointer());
				count_of_voxels = 0;
				const int max = 256;
				for (int j=0; j<max; j++)
				{
					int value = pixels[j];
					if (value < y_min || value > y_max)
					{
						value = 0;
					}
					count_of_voxels += value;
					frequency_list.push_back(value);
#ifdef OUTPUT_TO_FILE
					myfile<<j<<", "<<value<<std::endl;
#endif
				}
#ifdef OUTPUT_TO_FILE
				myfile.close();
#endif
			}
		}
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

			generateVisibilityFunction(shiftScale);

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
			QString filter("Voreen transfer function (*.tfi)");
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
			updateTransferFunctionWidgetsFromArrays();
		}

		void onSaveTransferFunctionSlot()
		{
			// show file dialog
			QString filter("transfer function file (*.tfi)");
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

        void on_optimiseButton_clicked();
        void on_pushButton1_clicked();
        void on_pushButton2_clicked();
        void on_pushButton3_clicked();
        void on_updateButton_clicked();
        void on_defaultButton_clicked();
        void on_entropyButton_clicked();
};

#endif // MAINWINDOW_H
