#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>

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
#include <vtkPNGReader.h>
#include <vtkMath.h>

#include "ctkTransferFunction.h"
#include "ctkVTKColorTransferFunction.h"
#include "ctkTransferFunctionView.h"
#include "ctkTransferFunctionGradientItem.h"
#include "ctkTransferFunctionControlPointsItem.h"
#include "ctkVTKVolumePropertyWidget.h"

#include "tinyxml2/tinyxml2.h"
#include "ui_mainwindow.h"

//#ifndef OUTPUT_TO_FILE
//#define OUTPUT_TO_FILE
//#endif // OUTPUT_TO_FILE

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
	QString selected_region_filename;
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
	double domain_x, domain_y;
	double x_max, x_min, y_max, y_min;
	int count_of_voxels;
	void* volume_ptr;
	std::vector<double> region_weight_list;
	const static int max_iteration_count = 65536;

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

	double denormalise_intensity_from_domain(double n)
	{
		return map_to_range(n, lower_bound, upper_bound, domain_x, domain_y);
	}

	double normalise_intensity(double n)
	{
		return map_to_range(n, 0, 255, lower_bound, upper_bound);
	}

	double normalise_intensity_to_domain(double n)
	{
		return map_to_range(n, domain_x, domain_y, lower_bound, upper_bound);
	}

	double normalise_rgba(int n)
	{
		return map_to_range(n, 0, 255, 0, 1);
	}

	int denormalise_rgba(double n)
	{
		return map_to_range(n, 0, 1, 0, 255);
	}

	double get_distance_between_colour_and_pixels_with_metric(double r, double g, double b, unsigned char * pixels, int count, int numComponents, int metric = 0)
	{
		if (metric == 1)
		{
			return get_squared_distance_between_colour_and_pixels(r, g, b, pixels, count, numComponents);
		}
		else
		{
			return get_distance_between_colour_and_pixels(r, g, b, pixels, count, numComponents);
		}
	}

	double get_distance_between_colour_and_pixels(double r, double g, double b, unsigned char * pixels, int count, int numComponents)
	{
		double distance = 0;
		for (int i=0; i<count; i++)
		{
			int index_base = i * numComponents;
			double dr = normalise_rgba(pixels[index_base + 0]) - r;
			double dg = normalise_rgba(pixels[index_base + 1]) - g;
			double db = normalise_rgba(pixels[index_base + 2]) - b;
			double d = sqrt(dr*dr + dg*dg + db*db);
			distance += d;
		}
		return distance;
	}

	double get_squared_distance_between_colour_and_pixels(double r, double g, double b, unsigned char * pixels, int count, int numComponents)
	{
		double distance = 0;
		for (int i=0; i<count; i++)
		{
			int index_base = i * numComponents;
			double dr = normalise_rgba(pixels[index_base + 0]) - r;
			double dg = normalise_rgba(pixels[index_base + 1]) - g;
			double db = normalise_rgba(pixels[index_base + 2]) - b;
			//double d = sqrt(dr*dr + dg*dg + db*db);
			double d = dr*dr + dg*dg + db*db;
			distance += d;
		}
		return distance;
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

	double get_area_entropy(int index)
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

		//std::cout<<"intensity "<<a<<" "<<b;
		a = denormalise_intensity(a);
		b = denormalise_intensity(b);
		//std::cout<<" map to [0, 255] "<<a<<" "<<b<<std::endl;

		double sum = 0;
		// int intensity belongs to [0,255]
		for (int intensity=(int)a; intensity<b; intensity++)
		{
			if (intensity >= a)
			{
				//std::cout<<intensity<<" ";
				sum += get_entropy_opacity_by_index(intensity, index);
			}
		}
		//std::cout<<std::endl;
		return sum;
	}

	/// int index is the index of control point.
	/// the control point 0 and (size-1) are the bounds
	double get_neighbour_area_entropy(int index)
	{
		return get_area_entropy(index) + get_area_entropy(index-1);
	}

	double get_control_point_weight(int index)
	{
		if (index >=0 && index < region_weight_list.size())
		{
			return region_weight_list[index];
		} 
		else
		{
			return 1;
		}
	}

	double get_neighbour_area_entropy_weighted_for_region(int index)
	{
		return get_neighbour_area_entropy(index) * get_control_point_weight(index);
	}

	// double intensity belongs to [0,255]
	double get_entropy_opacity_by_index(double intensity, int index)
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
			return probability * (-log(probability));
			//return get_opacity_interpolation_without_index(normalised) * probability * (-log(probability));
		}
		else
		{
			return 0;
		}
	}

	/// compute the area under an edge by integration
	/// double intensity belongs to [0,255]
	double get_entropy_opacity(double intensity)
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

	/// compute the geometric area (triangle, rectangle or trapezoid) under an edge
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

	// int index is the index of control points
	double get_neighbour_area(int index)
	{
		return get_area(index) + get_area(index - 1);
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

	void balance_transfer_function()
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
				if (area < min_area && colour_list[i][3] < 1)
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
			std::cout<<"balance TF max index="<<max_index<<" min index="<<min_index<<" opacity="<<height_max<<" new opacity="<<height_max_new<<" area="<<area<<" new area="<<new_area<<" height="<<height_min<<" new height="<<height_min_new<<endl;
		}
	}

	void balance_opacity()
	{
		//std::cout<<"colour_list size="<<colour_list.size()<<" intensity_list size="<<intensity_list.size()<<std::endl;
		int max_index = -1;
		int min_index = -1;
		double max_area = std::numeric_limits<int>::min();
		double min_area = std::numeric_limits<int>::max();
		const double epsilon = 1./256.;
		for (unsigned int i=0; i<intensity_list.size(); i++)
		{
			if (colour_list[i][3] > epsilon)
			{
				double area = get_neighbour_area_entropy(i);
				if (area > max_area)
				{
					max_index = i;
					max_area = area;
				}
				if (area < min_area && colour_list[i][3] < 1)
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
			double area = get_neighbour_area_entropy(max_index);
			colour_list[max_index][3] = height_max_new;
			double new_area = get_neighbour_area_entropy(max_index);
			double area_decreased = area - new_area;
			//double height_increased = get_height_given_area_increment(min_index, area_decreased);
			double height_min = colour_list[min_index][3];
			double height_min_new = height_min + step_size;
			height_min_new = height_min_new > 1 ? 1 : height_min_new;
			colour_list[min_index][3] = height_min_new;
			//std::cout<<"balance TF entropy max index="<<max_index<<" min index="<<min_index<<" opacity="<<height_max<<" new opacity="<<height_max_new<<" area="<<area<<" new area="<<new_area<<" height="<<height_min<<" new height="<<height_min_new<<endl;
		}
	}

	void reduce_opacity()
	{
		//std::cout<<"colour_list size="<<colour_list.size()<<" intensity_list size="<<intensity_list.size()<<std::endl;
		int max_index = -1;
		//int min_index = -1;
		double max_area = std::numeric_limits<int>::min();
		//double min_area = std::numeric_limits<int>::max();
		const double epsilon = 1./256.;
		for (unsigned int i=0; i<intensity_list.size(); i++)
		{
			if (colour_list[i][3] > epsilon)
			{
				double area = get_neighbour_area_entropy(i);
				if (area > max_area)
				{
					max_index = i;
					max_area = area;
				}
				//if (area < min_area && colour_list[i][3] < 1)
				//{
				//	min_index = i;
				//	min_area = area;
				//}
			}
		}
		if (-1 != max_index)
		{
			const double step_size = 1./255.;
			double height_max = colour_list[max_index][3];
			double height_max_new = height_max - step_size;
			height_max_new = height_max_new < 0 ? 0 : height_max_new;
			double area = get_neighbour_area_entropy(max_index);
			colour_list[max_index][3] = height_max_new;
			double new_area = get_neighbour_area_entropy(max_index);
			double area_decreased = area - new_area;
			//double height_increased = get_height_given_area_increment(min_index, area_decreased);
			//double height_min = colour_list[min_index][3];
			//double height_min_new = height_min + step_size;
			//height_min_new = height_min_new > 1 ? 1 : height_min_new;
			//colour_list[min_index][3] = height_min_new;
			//std::cout<<"reduceOpacity max index="<<max_index<<" opacity="<<height_max<<" new opacity="<<height_max_new<<" area="<<area<<" new area="<<new_area<<endl;
		}
	}

	void increase_opacity()
	{
		//std::cout<<"colour_list size="<<colour_list.size()<<" intensity_list size="<<intensity_list.size()<<std::endl;
		//int max_index = -1;
		int min_index = -1;
		//double max_area = std::numeric_limits<int>::min();
		double min_area = std::numeric_limits<int>::max();
		const double epsilon = 1./256.;
		for (unsigned int i=0; i<intensity_list.size(); i++)
		{
			if (colour_list[i][3] > epsilon)
			{
				double area = get_neighbour_area_entropy(i);
				//if (area > max_area)
				//{
				//	max_index = i;
				//	max_area = area;
				//}
				if (area < min_area && colour_list[i][3] < 1)
				{
					min_index = i;
					min_area = area;
				}
			}
		}
		if (min_index != -1)
		{
			const double step_size = 1./255.;
			//double height_max = colour_list[max_index][3];
			//double height_max_new = height_max - step_size;
			//height_max_new = height_max_new < 0 ? 0 : height_max_new;
			//double area = get_neighbour_area_entropy(max_index);
			//colour_list[max_index][3] = height_max_new;
			//double new_area = get_neighbour_area_entropy(max_index);
			//double area_decreased = area - new_area;
			//double height_increased = get_height_given_area_increment(min_index, area_decreased);
			double height_min = colour_list[min_index][3];
			double height_min_new = height_min + step_size;
			height_min_new = height_min_new > 1 ? 1 : height_min_new;
			colour_list[min_index][3] = height_min_new;
			//std::cout<<"increaseOpacity min index="<<min_index<<" height="<<height_min<<" new height="<<height_min_new<<endl;
		}
	}

	void balance_opacity_for_region()
	{
		//std::cout<<"colour_list size="<<colour_list.size()<<" intensity_list size="<<intensity_list.size()<<std::endl;
		int max_index = -1;
		int min_index = -1;
		double max_area = std::numeric_limits<int>::min();
		double min_area = std::numeric_limits<int>::max();
		const double epsilon = 1./256.;
		for (unsigned int i=0; i<intensity_list.size(); i++)
		{
			if (colour_list[i][3] > epsilon)
			{
				double area = get_neighbour_area_entropy_weighted_for_region(i);
				if (area > max_area)
				{
					max_index = i;
					max_area = area;
				}
				if (area < min_area && colour_list[i][3] < 1)
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
			double area = get_neighbour_area_entropy_weighted_for_region(max_index);
			colour_list[max_index][3] = height_max_new;
			double new_area = get_neighbour_area_entropy_weighted_for_region(max_index);
			double area_decreased = area - new_area;
			//double height_increased = get_height_given_area_increment(min_index, area_decreased);
			double height_min = colour_list[min_index][3];
			double height_min_new = height_min + step_size;
			height_min_new = height_min_new > 1 ? 1 : height_min_new;
			colour_list[min_index][3] = height_min_new;
			//std::cout<<"balance TF entropy max index="<<max_index<<" min index="<<min_index<<" opacity="<<height_max<<" new opacity="<<height_max_new<<" area="<<area<<" new area="<<new_area<<" height="<<height_min<<" new height="<<height_min_new<<endl;
		}
	}

	void reduce_opacity_for_region()
	{
		//std::cout<<"colour_list size="<<colour_list.size()<<" intensity_list size="<<intensity_list.size()<<std::endl;
		int max_index = -1;
		//int min_index = -1;
		double max_area = std::numeric_limits<int>::min();
		//double min_area = std::numeric_limits<int>::max();
		const double epsilon = 1./256.;
		for (unsigned int i=0; i<intensity_list.size(); i++)
		{
			if (colour_list[i][3] > epsilon)
			{
				double area = get_neighbour_area_entropy_weighted_for_region(i);
				if (area > max_area)
				{
					max_index = i;
					max_area = area;
				}
				//if (area < min_area && colour_list[i][3] < 1)
				//{
				//	min_index = i;
				//	min_area = area;
				//}
			}
		}
		if (-1 != max_index)
		{
			const double step_size = 1./255.;
			double height_max = colour_list[max_index][3];
			double height_max_new = height_max - step_size;
			height_max_new = height_max_new < 0 ? 0 : height_max_new;
			double area = get_neighbour_area_entropy_weighted_for_region(max_index);
			colour_list[max_index][3] = height_max_new;
			double new_area = get_neighbour_area_entropy_weighted_for_region(max_index);
			double area_decreased = area - new_area;
			//double height_increased = get_height_given_area_increment(min_index, area_decreased);
			//double height_min = colour_list[min_index][3];
			//double height_min_new = height_min + step_size;
			//height_min_new = height_min_new > 1 ? 1 : height_min_new;
			//colour_list[min_index][3] = height_min_new;
			//std::cout<<"reduceOpacity max index="<<max_index<<" opacity="<<height_max<<" new opacity="<<height_max_new<<" area="<<area<<" new area="<<new_area<<endl;
		}
	}

	void increase_opacity_for_region()
	{
		//std::cout<<"colour_list size="<<colour_list.size()<<" intensity_list size="<<intensity_list.size()<<std::endl;
		//int max_index = -1;
		int min_index = -1;
		//double max_area = std::numeric_limits<int>::min();
		double min_area = std::numeric_limits<int>::max();
		const double epsilon = 1./256.;
		for (unsigned int i=0; i<intensity_list.size(); i++)
		{
			if (colour_list[i][3] > epsilon)
			{
				double area = get_neighbour_area_entropy_weighted_for_region(i);
				//if (area > max_area)
				//{
				//	max_index = i;
				//	max_area = area;
				//}
				if (area < min_area && colour_list[i][3] < 1)
				{
					min_index = i;
					min_area = area;
				}
			}
		}
		if (min_index != -1)
		{
			const double step_size = 1./255.;
			//double height_max = colour_list[max_index][3];
			//double height_max_new = height_max - step_size;
			//height_max_new = height_max_new < 0 ? 0 : height_max_new;
			//double area = get_neighbour_area_entropy_weighted_for_region(max_index);
			//colour_list[max_index][3] = height_max_new;
			//double new_area = get_neighbour_area_entropy_weighted_for_region(max_index);
			//double area_decreased = area - new_area;
			//double height_increased = get_height_given_area_increment(min_index, area_decreased);
			double height_min = colour_list[min_index][3];
			double height_min_new = height_min + step_size;
			height_min_new = height_min_new > 1 ? 1 : height_min_new;
			colour_list[min_index][3] = height_min_new;
			//std::cout<<"increaseOpacity min index="<<min_index<<" height="<<height_min<<" new height="<<height_min_new<<endl;
		}
	}

	void saveTransferFunctionToXML(const char *filename)
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
		const double epsilon = 1e-6;
		if (abs(domain_x-0)>epsilon || abs(domain_y-255)>epsilon)
		{
			auto domain = doc.NewElement("domain");
			domain->SetAttribute("x", domain_x);
			domain->SetAttribute("y", domain_y);
			transFuncIntensity->InsertEndChild(domain);
		}
		voreenData->InsertEndChild(transFuncIntensity);
		doc.InsertEndChild(voreenData);

		auto r = doc.SaveFile(filename);
		if (r != tinyxml2::XML_NO_ERROR)
		{
			std::cout<<"failed to save file"<<endl;
		}
	}

	void openTransferFunctionFromXML(const char *filename)
	{
		tinyxml2::XMLDocument doc;
		auto r = doc.LoadFile(filename);

		if (r != tinyxml2::XML_NO_ERROR)
		{
			std::cout<<"failed to open file"<<endl;
			return;
		}

		auto transFuncIntensity = doc.FirstChildElement("VoreenData")->FirstChildElement("TransFuncIntensity");
		lower_bound = atof(transFuncIntensity->FirstChildElement("lower")->Attribute("value"));
		upper_bound = atof(transFuncIntensity->FirstChildElement("upper")->Attribute("value"));
		auto domain = transFuncIntensity->FirstChildElement("domain");
		if (domain != NULL)
		{
			domain_x = atof(domain->Attribute("x"));
			domain_y = atof(domain->Attribute("y"));
			std::cout<<"domain x="<<domain_x<<" y="<<domain_y<<std::endl;
		}
		else
		{
			domain_x = 0;
			domain_y = 1;
			std::cout<<"domain doesn't exist. default: "<<domain_x<<" "<<domain_y<<std::endl;
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
	}

	void generate_default_transfer_function()
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
		domain_x = lower_bound = 0;
		domain_y = upper_bound = 1;

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

	void generate_spectrum_transfer_function(int number_of_colours = 6)
	{
		if (number_of_colours < 1 || number_of_colours > 256)
		{
			std::cout<<"number_of_control_points should belongs to [1,256]"<<std::endl;
			return;
		}

		//double h, s, v, r, g, b;
		//h = s = v = 1;
		//vtkMath::HSVToRGB(h, s, v, &r, &g, &b);
		//std::cout<<"this should be red. hsv "<<h<<" "<<s<<" "<<v<<" rgb "<<r<<" "<<g<<" "<<b<<std::endl;
		//h = 1/3.0;
		//vtkMath::HSVToRGB(h, s, v, &r, &g, &b);
		//std::cout<<"this should be green. hsv "<<h<<" "<<s<<" "<<v<<" rgb "<<r<<" "<<g<<" "<<b<<std::endl;
		//h = 2/3.0;
		//vtkMath::HSVToRGB(h, s, v, &r, &g, &b);
		//std::cout<<"this should be blue. hsv "<<h<<" "<<s<<" "<<v<<" rgb "<<r<<" "<<g<<" "<<b<<std::endl;

		int n = number_of_colours; // 1 to 6 groups of control points
		std::vector<std::vector<double>> spectrum;
		for (int i=0; i<n; i++)
		{
			double h = i / (double)n;
			double s, v, r, g, b;
			s = v = 1;
			vtkMath::HSVToRGB(h, s, v, &r, &g, &b);
			std::vector<double> c;
			c.push_back(r);
			c.push_back(g);
			c.push_back(b);
			spectrum.push_back(c);
		}

		const int m = 3; // 3 control points each group
		//// red, yellow, green, cyan, blue, magenta
		//double colours[6][m] = {
		//	{1,0,0},
		//	{1,1,0},
		//	{0,1,0},
		//	{0,1,1},
		//	{0,0,1},
		//	{1,0,1}
		//};
		double interval = 1.0 / (m * n + 1);

		intensity_list.clear();
		colour_list.clear();
		domain_x = lower_bound = 0;
		domain_y = upper_bound = 1;

		intensity_list.push_back(0);
		{
			std::vector<double> v;
			v.push_back(0);
			v.push_back(0);
			v.push_back(0);
			v.push_back(0);
			colour_list.push_back(v);
		}

		for (int i=0; i<m*n; i++)
		{
			int colour_index = i / m;
			double opacity = (i % m == 1) ? 0.5 : 0;

			intensity_list.push_back((i + 1) * interval);
			{
				std::vector<double> v;
				v.push_back(spectrum[colour_index][0]);
				v.push_back(spectrum[colour_index][1]);
				v.push_back(spectrum[colour_index][2]);
				v.push_back(opacity);
				colour_list.push_back(v);
			}
		}

		intensity_list.push_back(1);
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
			generate_default_transfer_function();
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

		//std::cout<<"update transfer function from widget"<<std::endl;
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
			//std::cout<<"x & opacity "<<intensity<<" "<<opacity<<" "<<denormalise_intensity(opacity)<<std::endl;
		}
	}

	void generate_visibility_function(vtkSmartPointer<vtkImageAlgorithm> reader)
	{
		int ignoreZero = 0;
		int numComponents = reader->GetOutput()->GetNumberOfScalarComponents();
		std::cout<<"generateVisibilityFunction component number="<<numComponents<<endl;
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
				std::cout<<"histogram type is "<<histogram->GetOutput()->GetScalarTypeAsString()<<std::endl;
				auto pixels = static_cast<int *>(histogram->GetOutput()->GetScalarPointer());
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

	void generate_LH_histogram(vtkSmartPointer<vtkImageAlgorithm> reader)
	{
		int numComponents = reader->GetOutput()->GetNumberOfScalarComponents();
		std::cout<<"generate_LH_histogram component number="<<numComponents<<endl;
		if( numComponents > 1 )
		{
			std::cout << "Error: cannot process an image with " 
				<< numComponents << " components!" << std::endl;
			//return EXIT_FAILURE;
		}

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

			double intensity_min = range[0];
			double intensity_max = range[1];

			auto imageData = extract->GetOutput();
			int dimensions[3];
			imageData->GetDimensions(dimensions);
			int count_of_voxels = dimensions[0] * dimensions[1] * dimensions[2];
			std::cout<<"dimension "<<dimensions[0]<<" "<<dimensions[1]<<" "<<dimensions[2]<<" count="<<count_of_voxels<<std::endl;
			std::cout<<"voxel type is "<<imageData->GetScalarTypeAsString()<<std::endl;
			//vtkImageScalarTypeNameMacro(imageData->GetScalarType());
			auto voxels = static_cast<unsigned char *>(extract->GetOutput()->GetScalarPointer());
			volume_ptr = voxels;
#ifdef OUTPUT_TO_FILE
			char filename[32] = "../voxels.txt";
			std::cout<<"voxel file "<<filename<<std::endl;
			std::ofstream myfile(filename);
			myfile<<hex;
			for (int i=0; i<count_of_voxels; i++)
			{
				if (i % dimensions[0] == 0)
				{
					myfile<<std::endl;
					if (i % (dimensions[0] * dimensions[1]) == 0)
					{
						myfile<<std::endl;
					}
				}
				myfile.width(2);
				myfile<<(int)voxels[i];
			}
			myfile<<std::endl;
#endif
		}
	}

	/// The energy function is the variance of control point weights.
	double get_energy_function()
	{
		std::vector<double> weights;

		// compute the mean
		double sum = 0;
		for (unsigned int i=0; i<intensity_list.size(); i++)
		{
			double w = get_neighbour_area_entropy(i);
			sum += w;
			weights.push_back(w);
		}
		double mean = sum / weights.size();

		// compute and return the variance
		sum = 0;
		for (unsigned int i=0; i<weights.size(); i++)
		{
			double diff = weights[i] - mean;
			sum += diff * diff;
		}
		return sum;
	}

	/// The energy function is the variance of control point weights for region
	double get_energy_function_weighted_for_region()
	{
		std::vector<double> weights;

		// compute the mean
		double sum = 0;
		for (unsigned int i=0; i<intensity_list.size(); i++)
		{
			double w = get_neighbour_area_entropy_weighted_for_region(i);
			sum += w;
			weights.push_back(w);
		}
		double mean = sum / weights.size();

		// compute and return the variance
		sum = 0;
		for (unsigned int i=0; i<weights.size(); i++)
		{
			double diff = weights[i] - mean;
			sum += diff * diff;
		}
		return sum;
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

		void onAppendVolumeSlot()
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
			transfer_function_filename = QFileDialog::getOpenFileName(this, QString(tr("Open a transfer function")), transfer_function_filename, filter); 
			if (transfer_function_filename.isEmpty())
			{
				return;
			}

			//// show filename on window title
			//this->setWindowTitle(QString::fromUtf8("Volume Renderer - ") + volume_filename);

			// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
			QByteArray ba = transfer_function_filename.toLocal8Bit();  
			const char *filename_str = ba.data();

			std::cout<<"transfer function file: "<<filename_str<<endl;

			openTransferFunctionFromXML(filename_str);
			updateTransferFunctionWidgetsFromArrays();
		}

		void onSaveTransferFunctionSlot()
		{
			// show file dialog
			QString filter("transfer function file (*.tfi)");
			transfer_function_filename = QFileDialog::getSaveFileName(this, QString(tr("Save transfer function as")), transfer_function_filename, filter); 
			if (transfer_function_filename.isEmpty())
			{
				return;
			}

			//// show filename on window title
			//this->setWindowTitle(QString::fromUtf8("Volume Renderer - ") + volume_filename);

			// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
			QByteArray ba = transfer_function_filename.toLocal8Bit();  
			const char *filename_str = ba.data();

			std::cout<<"transfer function file: "<<filename_str<<endl;

			saveTransferFunctionToXML(filename_str);
			//updateTransferFunction();
		}

		/// if distance_metric==1 then compute distance with squared distance
		void read_region_image_and_compute_distance(int distance_metric = 0)
		{
			// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
			QByteArray ba = selected_region_filename.toLocal8Bit();  
			const char *filename_str = ba.data();

			std::cout<<"image file: "<<filename_str<<endl;

			auto reader = vtkSmartPointer<vtkPNGReader>::New();
			if( !reader->CanReadFile(filename_str))
			{
				std::cout << "Error: cannot read " << filename_str << std::endl;
				return;
			}
			reader->SetFileName(filename_str);
			reader->Update();

			int numComponents = reader->GetOutput()->GetNumberOfScalarComponents();
			std::cout<<"component number="<<numComponents<<endl;
			if( numComponents > 3 )
			{
				std::cout << "Error: cannot process an image with " 
					<< numComponents << " components!" << std::endl;
				return;
			}

			// get pixels from image
			auto imageData = reader->GetOutput();
			int dimensions[3];
			imageData->GetDimensions(dimensions);
			int count_of_pixels = dimensions[0] * dimensions[1] * dimensions[2];
			std::cout<<"dimension "<<dimensions[0]<<" "<<dimensions[1]<<" "<<dimensions[2]<<" count="<<count_of_pixels<<std::endl;
			std::cout<<"pixel type is "<<imageData->GetScalarTypeAsString()<<std::endl;
			auto pixels = static_cast<unsigned char *>(imageData->GetScalarPointer());

#ifdef OUTPUT_TO_FILE
			// compute and write the histogram to file
			int rgb_histogram[256][3];
			std::memset(rgb_histogram, 0, 256*3*sizeof(int));

			for (int i=0; i<count_of_pixels; i++)
			{
				for (int j=0; j<numComponents; j++)
				{
					rgb_histogram[pixels[i*numComponents+j]][j]++;
				}
			}

			char filename2[32] = "../rgb_histogram.csv";
			std::cout<<"rgb_histogram file "<<filename2<<std::endl;
			std::ofstream out2(filename2);

			for (int i=0; i<256; i++)
			{
				for (int j=0; j<numComponents; j++)
				{
					out2<<rgb_histogram[i][j];
					if (j<numComponents-1)
					{
						out2<<",";
					}
				}
				out2<<endl;
			}
			out2.close();
#endif

			// compute region weights based on the selected image
			region_weight_list.clear();
			double sum = 0;
			for (unsigned int i=0; i<colour_list.size(); i++)
			{
				double r = colour_list[i][0];
				double g = colour_list[i][1];
				double b = colour_list[i][2];
				double distance = get_distance_between_colour_and_pixels_with_metric(r, g, b, pixels, count_of_pixels, numComponents, distance_metric);
				region_weight_list.push_back(distance);
				sum += distance;
			}

			if (sum > 0)
			{
				// normalize the distances
				for (unsigned int i=0; i<region_weight_list.size(); i++)
				{
					region_weight_list[i] = region_weight_list[i] / sum;
				}
			}
		}

		void onOpenSelectedRegionSlot()
		{
			// show file dialog
			QString filter("PNG image (*.png)");
			selected_region_filename = QFileDialog::getOpenFileName(this, QString(tr("Open a PNG image")), selected_region_filename, filter); 
			if (selected_region_filename.isEmpty())
			{
				return;
			}

			read_region_image_and_compute_distance(1);
		}

		void onComputeDistanceSlot()
		{
			std::cout<<"distance (RGB) is chosen"<<std::endl;
			read_region_image_and_compute_distance();
		}

		void onComputeSquaredDistanceSlot()
		{
			std::cout<<"squared distance (RGB) is chosen"<<std::endl;
			read_region_image_and_compute_distance(1);
		}

		void onDefaultTransferFunctionSlot()
		{
			generate_default_transfer_function();
			updateTransferFunctionWidgetsFromArrays();
		}

		void onSpectrumTransferFunctionSlot()
		{
			bool ok;
			int n = QInputDialog::getInt(this, tr("QInputDialog::getInteger()"), tr("Number of colours [1,256]:"), 6, 1, 256, 1, &ok);
			if (ok)
			{
				std::cout<<"QInputDialog::getInteger() "<<n<<std::endl;
				generate_spectrum_transfer_function(n);
				updateTransferFunctionWidgetsFromArrays();
			}
		}

		void on_entropyButton_clicked();
		void on_frequencyButton_clicked();
		void on_opacityButton_clicked();
		void on_visibilityButton_clicked();
		void on_entropyOpacityButton_clicked();
		void on_balanceButton_clicked();
		void on_reduceOpacityButton_clicked();
		void on_lhHistogramButton_clicked();
		void on_balanceOpacityButton_clicked();
		void on_increaseOpacityButton_clicked();
		void on_enhanceRegionButton_clicked();
		void on_weakenRegionButton_clicked();
		void on_balanceRegionButton_clicked();
};

#endif // MAINWINDOW_H
