#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QColorDialog>
#include <QTime>
#include <QStandardItemModel>
#include <QVector>
#include <QDesktopWidget>

#include <iostream>
#include <memory>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <limits>
#include <string>
#include <string.h>
#include <cmath>
#include <ctime>

// cereal - A C++11 library for serialization
#include <cereal/archives/xml.hpp>
#include <cereal/types/vector.hpp>

#include <QVTKWidget.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkGPUVolumeRayCastMapper.h>
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
#include <vtkMetaImageWriter.h>
#include <vtkBoxWidget2.h>
#include <vtkBoxRepresentation.h>
#include <vtkVolumeCollection.h>
#include <vtkCamera.h>

// CTK
#include "ctkTransferFunction.h"
#include "ctkVTKColorTransferFunction.h"
#include "ctkTransferFunctionView.h"
#include "ctkTransferFunctionGradientItem.h"
#include "ctkTransferFunctionControlPointsItem.h"
#include "ctkVTKVolumePropertyWidget.h"

#include "ctkVTKHistogram.h"

#include "ctkVTKMatrixWidget.h"
#include "ctkVTKPropertyWidget.h"
#include "ctkVTKScalarBarWidget.h"
#include "ctkVTKSurfaceMaterialPropertyWidget.h"
#include "ctkVTKTextPropertyWidget.h"
#include "ctkVTKScalarsToColorsWidget.h"
#include "ctkVTKScalarsToColorsView.h"

// Slicer
#include "vtkSlicerGPURayCastVolumeMapper.h"
#include "vtkSlicerGPURayCastMultiVolumeMapper.h"

// TinyXML-2
#include "tinyxml2/tinyxml2.h"

#include "ui_mainwindow.h"
#include "screenshotwidget.h"

#include "RayCastType.h"
#include "transfer_function_xml.h"
#include "serialization.h"

//#ifndef OUTPUT_TO_FILE
//#define OUTPUT_TO_FILE
//#endif // OUTPUT_TO_FILE

//// http://stackoverflow.com/questions/3831439/how-to-give-a-delay-in-loop-execution-using-qt
#include <QThread>    

struct Sleeper : public QThread
{
	static const bool enable_sleep = false;

	static void usleep(unsigned long usecs)
	{
		if (enable_sleep) QThread::usleep(usecs);
	}

	static void msleep(unsigned long msecs)
	{ 
		if (enable_sleep) QThread::msleep(msecs); 
	}

	static void sleep(unsigned long secs)
	{ 
		if (enable_sleep) QThread::sleep(secs);
	}
};

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

protected:
	virtual void resizeEvent(QResizeEvent * event)
	{
		QMainWindow::resizeEvent(event);
		//vtk_widget.repaint();
		std::cout << "MainWindow::resizeEvent()\n";
	}

private:
	Ui::MainWindow *ui;

	ScreenshotWidget screenshot_widget;

	ctkVTKScalarsToColorsWidget ctkVTKScalarsToColorsWidget1;
	ctkVTKScalarsToColorsWidget ctkVTKScalarsToColorsWidget2;
	vtkSmartPointer<vtkPiecewiseFunction> opacity_function_1;
	vtkSmartPointer<vtkPiecewiseFunction> opacity_function_2;

	QString volume_filename;
	QString transfer_function_filename;
	QString transfer_function_filename_save;
	QString selected_region_filename;
	Path path;
	vtkSmartPointer<vtkRenderWindowInteractor> interactor;
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkVolume> volume0;
	vtkSmartPointer<vtkVolume> Volume() const { return volume0; }
	void Volume(vtkSmartPointer<vtkVolume> val) { volume0 = val; }
	QVTKWidget vtk_widget;
	ctkVTKVolumePropertyWidget volume_property_widget;
	std::vector<double> intensity_list;
	std::vector<double> opacity_list;
	std::vector<double> frequency_list;
	std::vector<double> control_point_weight_list;
	vtkSmartPointer<vtkPiecewiseFunction> scalar_opacity;
	vtkSmartPointer<vtkPiecewiseFunction> gradient_opacity;
	vtkSmartPointer<vtkColorTransferFunction> scalar_color;
	double x_max, x_min, y_max, y_min;
	int count_of_voxels;
	void* volume_ptr;
	const static int max_iteration_count = USHRT_MAX;
	int enable_squared_distance;
	int enable_hsv_distance;
	int number_of_colours_in_spectrum;
	int Number_of_colours_in_spectrum() const { return number_of_colours_in_spectrum; }
	void Number_of_colours_in_spectrum(int val) { number_of_colours_in_spectrum = val; }
	QString batch_path;
	int enable_spectrum_ramp;
	QStandardItemModel model_for_listview, model_for_listview2;
	QColor colour_for_optimization;
	QString window_title;
	QString Window_title() const { return window_title; }
	void Window_title(QString val) { window_title = val; }

	// threshold and domain as in voreen transfer function (.tfi) files
	double threshold_x, threshold_y;
	double domain_x, domain_y;

	// range of voxel values
	double range_x, range_y;

	// encapsulated fields generated by VA X
	double Threshold_x() const { return threshold_x; }
	void Threshold_x(double val) { threshold_x = val; }
	double Threshold_y() const { return threshold_y; }
	void Threshold_y(double val) { threshold_y = val; }
	double Domain_x() const { return domain_x; }
	void Domain_x(double val) { domain_x = val; }
	double Domain_y() const { return domain_y; }
	void Domain_y(double val) { domain_y = val; }
	double Range_x() const { return range_x; }
	void Range_x(double val) { range_x = val; }
	double Range_y() const { return range_y; }
	void Range_y(double val) { range_y = val; }

	int volume_mapper_index;
	int Volume_mapper_index() const { return volume_mapper_index; }
	void Volume_mapper_index(int val) { volume_mapper_index = val; }

	QComboBox * RenderingTechniqueComboBox()
	{
		return ui->comboBox;
	}

	static double EPSILON()
	{
		return 1e-6;
	}

	QGraphicsScene * get_GraphicsScene()
	{
		QGraphicsScene *scene = ui->graphicsView->scene();
		if (scene == NULL)
		{
			scene = new QGraphicsScene();
			ui->graphicsView->setScene(scene);
			std::cout << "create a new scene" << std::endl;
		}
		return scene;
	}

	QGraphicsScene * get_GraphicsScene_for_spectrum()
	{
		QGraphicsScene *scene = ui->graphicsView_2->scene();
		if (scene == NULL)
		{
			scene = new QGraphicsScene();
			ui->graphicsView_2->setScene(scene);
			std::cout << "create a new scene for drawing spectrums" << std::endl;
		}
		return scene;
	}

	QLayout * get_vtk_layout()
	{
		return ui->verticalLayout;
	}

	QLayout * get_property_layout()
	{
		return ui->verticalLayout_2;
	}

	QLayout * get_histogram_layout()
	{
		return ui->verticalLayout_5;
	}

	double clamp(double val, double minimum, double maximun)
	{
		if (val < minimum)
		{
			return minimum;
		}
		if (val > maximun)
		{
			return maximun;
		}
		return val;
	}

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif
	/**
	http://stackoverflow.com/questions/11209115/creating-gaussian-filter-of-required-length-in-python
	from math import pi, sqrt, exp
	def gauss(n=11,sigma=1):
	r = range(-int(n/2),int(n/2)+1)
	return [1 / (sigma * sqrt(2*pi)) * exp(-float(x)**2/(2*sigma**2)) for x in r]
	*/
	std::vector<double> gaussian_kernel_1d(int n = 11, double sigma = 1)
	{
		std::vector<double> kernel;
		const double pi = M_PI;
		std::cout << "gaussian_kernel_1d" << std::endl;
		for (int x = -(n / 2); x < (n / 2) + 1; x++)
		{
			double val = 1 / (sigma * sqrt(2 * pi)) * exp(-x*x / (2. * sigma*sigma));
			kernel.push_back(val);
			std::cout << val << " ";
		}
		std::cout << std::endl;
		return kernel;
	}

	/// Re-maps a number from one range to another.
	double map_to_range(double val, double src_lower, double src_upper, double target_lower, double target_upper)
	{
		val = val < src_lower ? src_lower : val;
		val = val > src_upper ? src_upper : val;
		double normalised = (val - src_lower) / (src_upper - src_lower);
		return normalised * (target_upper - target_lower) + target_lower;
	}

	double denormalise_intensity(double val)
	{
		return map_to_range(val, Domain_x(), Domain_y(), Range_x(), Range_y());
	}

	double normalise_intensity(double val)
	{
		return map_to_range(val, Range_x(), Range_y(), Domain_x(), Domain_y());
	}

	double normalise_rgba(int n)
	{
		return map_to_range(n, 0, 255, 0, 1);
	}

	int denormalise_rgba(double n)
	{
		return static_cast<int>(map_to_range(n, 0, 1, 0, 255));
	}

	template<class T>
	unsigned char denormalise_rgba(double n)
	{
		return static_cast<T>(map_to_range(n, 0, 1, 0, 255));
	}

	double get_distance_between_colour_and_pixels(double r, double g, double b, unsigned char * pixels, int count, int numComponents)
	{
		double distance = 0;
		for (int i = 0; i < count; i++)
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
		for (int i = 0; i < count; i++)
		{
			int index_base = i * numComponents;
			double dr = normalise_rgba(pixels[index_base + 0]) - r;
			double dg = normalise_rgba(pixels[index_base + 1]) - g;
			double db = normalise_rgba(pixels[index_base + 2]) - b;
			double d = dr*dr + dg*dg + db*db;
			distance += d;
		}
		return distance;
	}

	double get_distance_between_colour_and_pixels_hsv(double r, double g, double b, unsigned char * pixels, int count, int numComponents)
	{
		double distance = 0;
		for (int i = 0; i < count; i++)
		{
			int index_base = i * numComponents;
			// convert rgb to hsv and get the difference in hue
			double h, s, v, h1, s1, v1;
			vtkMath::RGBToHSV(r, g, b, &h, &s, &v);
			vtkMath::RGBToHSV(normalise_rgba(pixels[index_base + 0]), normalise_rgba(pixels[index_base + 1]), normalise_rgba(pixels[index_base + 2]), &h1, &s1, &v1);
			double d = abs(h - h1);
			if (abs(s) < EPSILON() || abs(s1) < EPSILON())
			{
				d = 1;
			}
			distance += d;
		}
		return distance;
	}

	double get_squared_distance_between_colour_and_pixels_hsv(double r, double g, double b, unsigned char * pixels, int count, int numComponents)
	{
		double distance = 0;
		for (int i = 0; i < count; i++)
		{
			int index_base = i * numComponents;
			// convert rgb to hsv and get the difference in hue
			double h, s, v, h1, s1, v1;
			vtkMath::RGBToHSV(r, g, b, &h, &s, &v);
			vtkMath::RGBToHSV(normalise_rgba(pixels[index_base + 0]), normalise_rgba(pixels[index_base + 1]), normalise_rgba(pixels[index_base + 2]), &h1, &s1, &v1);
			double d = (h - h1) * (h1 - h);
			if (abs(s) < EPSILON() || abs(s1) < EPSILON())
			{
				d = 1;
			}
			distance += d;
		}
		return distance;
	}

	double get_distance_between_colour_and_pixels_selector(double r, double g, double b, unsigned char * pixels, int count, int numComponents, int squared = 0, int hsv = 0)
	{
		if (squared == 1)
		{
			if (hsv == 1)
			{
				return get_squared_distance_between_colour_and_pixels_hsv(r, g, b, pixels, count, numComponents);
			}
			else
			{
				return get_squared_distance_between_colour_and_pixels(r, g, b, pixels, count, numComponents);
			}
		}
		else
		{
			if (hsv == 1)
			{
				return get_distance_between_colour_and_pixels_hsv(r, g, b, pixels, count, numComponents);
			}
			else
			{
				return get_distance_between_colour_and_pixels(r, g, b, pixels, count, numComponents);
			}
		}
	}

	double get_opacity(int i)
	{
		return opacity_list[i];
	}

	void set_opacity(int i, double v)
	{
		opacity_list[i] = v;
	}

	double get_colour_r(int i)
	{
		double xrgb[6];
		scalar_color->GetNodeValue(i, xrgb);
		return xrgb[1];
	}
	double get_colour_g(int i)
	{
		double xrgb[6];
		scalar_color->GetNodeValue(i, xrgb);
		return xrgb[2];
	}
	double get_colour_b(int i)
	{
		double xrgb[6];
		scalar_color->GetNodeValue(i, xrgb);
		return xrgb[3];
	}

	void get_color_by_intensity(double x, double rgb[3])
	{
		scalar_color->GetColor(x, rgb);
	}

	int intensity_list_size()
	{
		return intensity_list.size();
	}

	void set_intensity(int i, double v)
	{
		intensity_list[i] = v;
	}

	double get_intensity(int i)
	{
		return intensity_list[i];
	}

	void intensity_list_clear()
	{
		intensity_list.clear();
	}

	void intensity_list_push_back(double v)
	{
		intensity_list.push_back(v);
	}

	void opacity_list_push_back(double v)
	{
		opacity_list.push_back(v);
	}

	void opacity_list_clear()
	{
		opacity_list.clear();
	}

	void colour_list_push_back(std::vector<double> v, double x = -1)
	{
		// if x == -1, which means parameter x is not provided, then the last value in intensity_list is used
		if (abs(x + 1) < EPSILON())
		{
			if (intensity_list_size() > 0)
			{
				x = get_intensity(intensity_list_size() - 1);
			}
			else
			{
				std::cerr << "An error occurred in colour_list_push_back(). intensity_list is empty" << std::endl;
			}
		}
		scalar_color->AddRGBPoint(denormalise_intensity(x), v[0], v[1], v[2]);
	}

	void colour_list_clear()
	{
		scalar_color->RemoveAllPoints();
	}

	int colour_list_size()
	{
		return scalar_color->GetSize();
	}

	double get_frequency(double intensity) // intensity belongs to [0,255]
	{
		int intensity_int = (int)intensity;
		const int max = static_cast<int>(Range_y() + EPSILON());
		if (intensity_int >= 0 && intensity_int < max)
		{
			// linear interpolation
			double t = intensity - intensity_int;
			double a = frequency_list[intensity_int];
			double b = frequency_list[intensity_int + 1];
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
				std::cout << "Errors occur in get_frequency()" << std::endl;
				return 0;
			}
		}
	}

	double get_frequency_and_opacity(int i)
	{
		double intensity = denormalise_intensity(get_intensity(i));
		return get_frequency(intensity) * get_opacity(i);
	}

	double get_noteworthiness(int i)
	{
		const double epsilon = EPSILON();
		double intensity = denormalise_intensity(get_intensity(i));
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
		if (intensity < get_intensity(0))
		{
			index = -1;
		}
		else
		{
			if (intensity > get_intensity(intensity_list_size() - 1))
			{
				index = intensity_list_size() - 1;
			}
			else
			{
				index = -2;
				for (auto i = 0; i < intensity_list_size() - 1; i++)
				{
					if (get_intensity(i) <= intensity && get_intensity(i + 1) >= intensity)
					{
						index = i;
						break;
					}
				}
				if (index == -2)
				{
					std::cout << "Errors occur in get_opacity_interpolation_without_index(double intensity)" << std::endl;
					return 0;
				}
			}
		}

		return get_opacity_by_interpolation(intensity, index);
	}

	/// double intensity belongs to [0,1]
	/// int index >=0 && index < intensity_list_size()
	double get_opacity_by_interpolation(double intensity, int index)
	{
		int i1 = index, i2 = index + 1;
		if (i1 >= 0 && i2 < intensity_list_size())
		{
			// linear interpolation
			double t = (intensity - get_intensity(i1)) / (get_intensity(i2) - get_intensity(i1));

			double a = get_opacity(i1);
			double b = get_opacity(i2);
			return (a + (b - a) * t);
		}
		else
		{
			if (i1 == -1)
			{
				return get_opacity(i2);
			}
			else
			{
				if (i1 == intensity_list_size() - 1)
				{
					return get_opacity(i1);
				}
				else
				{
					std::cout << "Errors occur in get_opacity_by_interpolation()" << std::endl;
					return 0;
				}
			}
		}
	}

	/// double intensity belongs to [0,1]
	/// int index >=0 && index < intensity_list_size()
	double get_control_point_weight_by_interpolation(double intensity, int index)
	{
		int i1 = index, i2 = index + 1;
		if (i1 >= 0 && i2 < intensity_list_size())
		{
			// linear interpolation
			double t = (intensity - get_intensity(i1)) / (get_intensity(i2) - get_intensity(i1));

			// get control point weights
			double a = get_control_point_weight(i1);
			double b = get_control_point_weight(i2);

			return (a + (b - a) * t);
		}
		else
		{
			if (i1 == -1)
			{
				return get_control_point_weight(i2);
			}
			else
			{
				if (i1 == intensity_list_size() - 1)
				{
					return get_control_point_weight(i1);
				}
				else
				{
					std::cout << "Errors occur in get_control_point_weight_by_interpolation()" << std::endl;
					return 0;
				}
			}
		}
	}

	double get_area_entropy(int index)
	{
		double a, b;
		if (index >= 0 && index < intensity_list_size() - 1)
		{
			a = get_intensity(index);
			b = get_intensity(index + 1);
		}
		else
		{
			if (index == -1)
			{
				a = 0;
				b = get_intensity(index + 1);
			}
			else
			{
				if (index == intensity_list_size() - 1)
				{
					a = get_intensity(index);
					b = 1;
				}
				else
				{
					std::cout << "index out of range in get_area_integral()" << endl;
					return 0;
				}
			}
		}

		a = denormalise_intensity(a);
		b = denormalise_intensity(b);

		double sum = 0;
		// int intensity belongs to [0,255]
		for (int intensity = static_cast<int>(a + EPSILON()); intensity < b; intensity++)
		{
			if (intensity >= a)
			{
				sum += get_entropy_opacity_by_index(intensity, index);
			}
		}
		return sum;
	}

	// a weighted version of get_area_entropy. Its value is multiplied by control point weights
	double get_weighted_area_entropy(int index)
	{
		double a, b;
		if (index >= 0 && index < intensity_list_size() - 1)
		{
			a = get_intensity(index);
			b = get_intensity(index + 1);
		}
		else
		{
			if (index == -1)
			{
				a = 0;
				b = get_intensity(index + 1);
			}
			else
			{
				if (index == intensity_list_size() - 1)
				{
					a = get_intensity(index);
					b = 1;
				}
				else
				{
					std::cout << "index out of range in get_area_integral()" << endl;
					return 0;
				}
			}
		}

		a = denormalise_intensity(a);
		b = denormalise_intensity(b);

		double sum = 0;
		// int intensity belongs to [0,255]
		for (int intensity = (int)a; intensity < b; intensity++)
		{
			if (intensity >= a)
			{
				sum += get_weighted_entropy_opacity_by_index(intensity, index);
			}
		}
		return sum;
	}

	/// int index is the index of control point.
	/// the control point 0 and (size-1) are the bounds
	double get_neighbour_area_entropy(int index)
	{
		return get_area_entropy(index) + get_area_entropy(index - 1);
	}

	double get_weighted_neighbour_area_entropy(int index)
	{
		return get_weighted_area_entropy(index) + get_weighted_area_entropy(index - 1);
	}

	double get_control_point_weight(int index)
	{
		if (index >= 0 && index < control_point_weight_list.size())
		{
			return control_point_weight_list[index];
		}
		else
		{
			return 1;
		}
	}

	// double intensity belongs to [0,255]
	double get_entropy_opacity_by_index(double intensity, int index)
	{
		double frequency = get_frequency(intensity);
		const double epsilon = EPSILON();
		double probability = frequency / count_of_voxels;
		if (probability > epsilon)
		{
			double normalised = normalise_intensity(intensity);
			return get_opacity_by_interpolation(normalised, index) * probability * (-log(probability));
		}
		else
		{
			return 0;
		}
	}

	// a weighted version of get_entropy_opacity_by_index. Its value is multiplied by control point weights
	// double intensity belongs to [0,255]
	double get_weighted_entropy_opacity_by_index(double intensity, int index)
	{
		double frequency = get_frequency(intensity);
		double probability = frequency / count_of_voxels;
		if (probability > EPSILON())
		{
			double normalised = normalise_intensity(intensity);
			return get_control_point_weight_by_interpolation(normalised, index) * get_opacity_by_interpolation(normalised, index) * probability * (-log(probability));
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
		const double epsilon = EPSILON();
		double probability = frequency / count_of_voxels;
		if (probability > epsilon)
		{
			double normalised = normalise_intensity(intensity);
			return probability * (-log(probability));
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
		const double epsilon = EPSILON();
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
		if (i >= 0 && i < intensity_list_size() - 1)
		{
			// area of a trapezoid
			double h = get_intensity(i + 1) - get_intensity(i);
			double a = get_visibility(i);
			double b = get_visibility(i + 1);
			return (a + b) * h / 2;
		}
		else
		{
			if (i == -1)
			{
				return (get_intensity(i + 1) - Domain_x()) * get_visibility(i + 1);
			}
			else
			{
				if (i == intensity_list_size() - 1)
				{
					return (Domain_y() - get_intensity(i)) * get_visibility(i);
				}
				else
				{
					std::cout << "index out of range in get_area()" << endl;
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
		if (i > 0 && i < intensity_list_size() - 1)
		{
			// area of two triangles
			double a = get_intensity(i) - get_intensity(i - 1);
			double b = get_intensity(i + 1) - get_intensity(i);
			visibility_increment = 2 * area_increment / (a + b);
		}
		else
		{
			if (i == 0)
			{
				// area of a rectangle (left) and a triangle (right)
				double a = get_intensity(i) - Domain_x();
				double b = get_intensity(i + 1) - get_intensity(i);
				visibility_increment = area_increment / (a + b / 2);
			}
			else
			{
				if (i == intensity_list_size() - 1)
				{
					// area of a triangle (left) and a rectangle (right)
					double a = get_intensity(i) - get_intensity(i - 1);
					double b = Domain_y() - get_intensity(i);
					visibility_increment = area_increment / (a / 2 + b);
				}
				else
				{
					std::cout << "index out of range in get_height_given_area_increment()" << endl;
					visibility_increment = 0;
				}
			}
		}
		double visibility = get_visibility(i);
		double opacity = get_opacity(i);
		return visibility_increment * opacity / visibility;
	}

	// old and deprecated. it's kept for testing purpose.
	void balance_transfer_function()
	{
		int max_index = -1;
		int min_index = -1;
		double max_area = std::numeric_limits<int>::min();
		double min_area = std::numeric_limits<int>::max();
		for (int i = 0; i<intensity_list_size(); i++)
		{
			if (get_opacity(i) > EPSILON())
			{
				double area = get_neighbour_area(i);
				if (area > max_area)
				{
					max_index = i;
					max_area = area;
				}
				if (area < min_area && get_opacity(i) < 1)
				{
					min_index = i;
					min_area = area;
				}
			}
		}
		if (min_index != max_index)
		{
			const double step_size = get_stepsize();
			double height_max = get_opacity(max_index);
			double height_max_new = height_max - step_size;
			height_max_new = height_max_new < EPSILON() ? EPSILON() : height_max_new;
			double area = get_neighbour_area(max_index);
			set_opacity(max_index, height_max_new);

			double new_area = get_neighbour_area(max_index);
			double area_decreased = area - new_area;
			double height_increased = get_height_given_area_increment(min_index, area_decreased);
			double height_min = get_opacity(min_index);
			double height_min_new = height_min + height_increased;
			height_min_new = height_min_new > 1 ? 1 : height_min_new;
			set_opacity(min_index, height_min_new);
		}
	}

	// global optimization
	// old and deprecated. it's kept for testing purpose.
	// please use balance_transfer_function_edge() instead.
	void balance_opacity()
	{
		int max_index = -1;
		int min_index = -1;
		double max_area = std::numeric_limits<int>::min();
		double min_area = std::numeric_limits<int>::max();

		for (int i = 0; i<intensity_list_size(); i++)
		{
			if (get_opacity(i) > EPSILON())
			{
				double area = get_neighbour_area_entropy(i);
				if (area > max_area)
				{
					max_index = i;
					max_area = area;
				}
				if (area < min_area && get_opacity(i) < 1)
				{
					min_index = i;
					min_area = area;
				}
			}
		}
		if (min_index != max_index)
		{
			const double step_size = get_stepsize();
			double height_max = get_opacity(max_index);
			double height_max_new = height_max - step_size;
			height_max_new = height_max_new < EPSILON() ? EPSILON() : height_max_new;
			double area = get_neighbour_area_entropy(max_index);
			set_opacity(max_index, height_max_new); // update opacity
			double new_area = get_neighbour_area_entropy(max_index); // calculate new area using new opacity
			double area_decreased = area - new_area;
			double height_min = get_opacity(min_index);
			double height_min_new = height_min + step_size;
			height_min_new = height_min_new > 1 ? 1 : height_min_new;
			set_opacity(min_index, height_min_new); // update opacity
		}
	}

	double get_stepsize()
	{
		return 1. / UCHAR_MAX;
	}

	// region-based or hue-based optimization
	// old and deprecated. it's kept for testing purpose.
	// please use balance_transfer_function_edge_for_region() instead.
	void balance_opacity_for_region()
	{
		int max_index = -1;
		int min_index = -1;
		double max_area = std::numeric_limits<int>::min();
		double min_area = std::numeric_limits<int>::max();

		for (int i = 0; i<intensity_list_size(); i++)
		{
			if (get_opacity(i) > EPSILON())
			{
				double area = get_weighted_neighbour_area_entropy(i);
				if (area > max_area)
				{
					max_index = i;
					max_area = area;
				}
				if (area < min_area && get_opacity(i) < 1)
				{
					min_index = i;
					min_area = area;
				}
			}
		}
		if (min_index != max_index)
		{
			const double step_size = get_stepsize();
			double height_max = get_opacity(max_index);
			double height_max_new = height_max - step_size;
			height_max_new = height_max_new < EPSILON() ? EPSILON() : height_max_new;
			double area = get_weighted_neighbour_area_entropy(max_index);
			set_opacity(max_index, height_max_new); // update opacity
			double new_area = get_weighted_neighbour_area_entropy(max_index); // calculate new area using new opacity
			double area_decreased = area - new_area;
			double height_min = get_opacity(min_index);
			double height_min_new = height_min + step_size;
			height_min_new = height_min_new > 1 ? 1 : height_min_new;
			set_opacity(min_index, height_min_new); // update opacity
		}
	}

	// a replacement of balance_opacity
	// global optimization
	void balance_transfer_function_edge()
	{
		int max_index = -1;
		int min_index = -1;
		double max_area = std::numeric_limits<int>::min();
		double min_area = std::numeric_limits<int>::max();

		for (int i = 0; i<intensity_list_size() - 1; i++)
		{
			if (get_opacity(i) > EPSILON())
			{
				double area = get_area_entropy(i);
				if (area > max_area)
				{
					max_index = i;
					max_area = area;
				}
				if (area < min_area && get_opacity(i) < 1)
				{
					min_index = i;
					min_area = area;
				}
			}
		}
		if (min_index != max_index)
		{
			// get the upper vertex of an edge
			int max_index_next = max_index + 1;
			double weight_max_1 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(max_index)), max_index);
			double weight_max_2 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(max_index_next)), max_index_next);
			if (get_opacity(max_index_next) > EPSILON() && get_opacity(max_index_next) < 1 && weight_max_2 > weight_max_1)
			{
				max_index++;
			}

			// get the lower vertex of an edge
			int min_index_next = min_index + 1;
			double weight_min_1 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(min_index)), min_index);
			double weight_min_2 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(min_index_next)), min_index_next);
			if (get_opacity(min_index_next) > EPSILON() && get_opacity(min_index_next) < 1 && weight_min_2 < weight_min_1)
			{
				min_index++;
			}

			const double step_size = get_stepsize();
			double height_max = get_opacity(max_index);
			double height_max_new = height_max - step_size;
			height_max_new = height_max_new < EPSILON() ? EPSILON() : height_max_new;
			double area = get_neighbour_area_entropy(max_index);
			set_opacity(max_index, height_max_new); // update opacity
			double new_area = get_neighbour_area_entropy(max_index); // calculate new area using new opacity
			double area_decreased = area - new_area;
			double height_min = get_opacity(min_index);
			double height_min_new = height_min + step_size;
			height_min_new = height_min_new > 1 ? 1 : height_min_new;
			set_opacity(min_index, height_min_new); // update opacity
		}
	}

	void balance_transfer_function_edge_newton()
	{
		std::vector<double> areas;
		std::vector<double> result_opacity_list = opacity_list;
		double mean_area = 0;
		for (int i = 0; i < intensity_list_size() - 1; i++)
		{
			auto area = get_area_entropy(i);
			areas.push_back(area);
			mean_area += area;
		}
		mean_area = mean_area / areas.size();
		double carry = 0;
		// for use in Newton's method
		auto h = 1e-9;
		for (int i = 0; i < areas.size(); i++)
		{
			// move only non-zero control points
			//if (get_opacity(i) > EPSILON())
			{
				if (areas[i] > mean_area)
				{
					// get the upper vertex of an edge
					auto index = i;
					int index_next = index + 1;
					double weight1 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(index)), index);
					double weight2 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(index_next)), index_next);
					if (get_opacity(index_next) > EPSILON() && get_opacity(index_next) < 1 && weight2 > weight1)
					{
						index++;
						carry = 0;
					}

					const double step_size = get_stepsize();
					double height = get_opacity(index);
					double height_new = height - step_size;
					height_new = height_new < EPSILON() ? EPSILON() : height_new;
					set_opacity(index, height_new); // update opacity

					auto area_diff = areas[i] - get_area_entropy(i);
					if (area_diff > 0)
					{
						// Newton's method in optimization
						// http://en.wikipedia.org/wiki/Newton's_method_in_optimization
						auto f_x = areas[i] - mean_area;
						set_opacity(index, height - h);
						auto f_x_minus_h = get_area_entropy(i) - mean_area;
						set_opacity(index, height + h);
						auto f_x_plus_h = get_area_entropy(i) - mean_area;
						set_opacity(index, height - h / 2);
						auto f_x_minus_half_h = get_area_entropy(i) - mean_area;
						set_opacity(index, height + h / 2);
						auto f_x_plus_half_h = get_area_entropy(i) - mean_area;
						auto step = h * (f_x_plus_half_h - f_x_minus_half_h) / (f_x_plus_h - 2 * f_x + f_x_minus_h);
						height_new = height - step;
						height_new = std::max(height_new, EPSILON());
						height_new = std::min(height_new, 1.);
						set_opacity(index, height); // reset opacity for later use
						result_opacity_list[index] = height_new; // store result opacity here

						auto gradient = 2 * (areas[i] - mean_area);
						auto slope = (f_x_plus_half_h - f_x_minus_half_h) / h;
						std::cout << "max slope=" << slope*step_size << " gradient=" << gradient*step_size << " newton=" << step << " step_size=" << step_size << std::endl;
					}

					if (carry > 0)
					{
						// if carry>0 then i>0 so i-1>=0
						auto a = std::abs(areas[i - 1] - mean_area);
						auto b = std::abs(areas[i] - mean_area);
						auto sum = a + b;
						auto lerp = (carry*a + height_new*b) / sum;
						result_opacity_list[index] = lerp;
					}
					carry = index > i ? height_new : 0;
				}
				if (areas[i] < mean_area)
				{
					auto index = i;
					// get the lower vertex of an edge
					int index_next = index + 1;
					double weight1 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(index)), index);
					double weight2 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(index_next)), index_next);
					if (get_opacity(index_next) > EPSILON() && get_opacity(index_next) < 1 && weight2 < weight1)
					{
						index++;
						carry = 0;
					}

					const double step_size = get_stepsize();
					double height = get_opacity(index);
					double height_new = height + step_size;
					height_new = height_new > 1 ? 1 : height_new;
					set_opacity(index, height_new); // update opacity

					auto area_diff = get_area_entropy(i) - areas[i];
					if (area_diff > 0)
					{
						// Newton's method in optimization
						// http://en.wikipedia.org/wiki/Newton's_method_in_optimization
						auto f_x = areas[i] - mean_area;
						set_opacity(index, height - h);
						auto f_x_minus_h = get_area_entropy(i) - mean_area;
						set_opacity(index, height + h);
						auto f_x_plus_h = get_area_entropy(i) - mean_area;
						set_opacity(index, height - h / 2);
						auto f_x_minus_half_h = get_area_entropy(i) - mean_area;
						set_opacity(index, height + h / 2);
						auto f_x_plus_half_h = get_area_entropy(i) - mean_area;
						auto step = h * (f_x_plus_half_h - f_x_minus_half_h) / (f_x_plus_h - 2 * f_x + f_x_minus_h);
						height_new = height - step;
						height_new = std::max(height_new, EPSILON());
						height_new = std::min(height_new, 1.);
						set_opacity(index, height); // reset opacity for later use
						result_opacity_list[index] = height_new; // store result opacity here

						auto gradient = 2 * (areas[i] - mean_area);
						auto slope = (f_x_plus_half_h - f_x_minus_half_h) / h;
						std::cout << "min slope=" << slope*step_size << " gradient=" << gradient*step_size << " newton=" << step << " step_size=" << step_size << std::endl;
					}

					if (carry > 0)
					{
						// if carry>0 then i>0 so i-1>=0
						auto a = std::abs(areas[i - 1] - mean_area);
						auto b = std::abs(areas[i] - mean_area);
						auto sum = a + b;
						auto lerp = (carry*a + height_new*b) / sum;
						result_opacity_list[index] = lerp;
					}
					carry = index > i ? height_new : 0;
				}
			}
		}
		opacity_list = result_opacity_list;
	}

	void balance_transfer_function_edge_gradient_descent()
	{
		std::vector<double> areas;
		std::vector<double> result_opacity_list = opacity_list;
		double mean_area = 0;
		for (int i = 0; i < intensity_list_size() - 1; i++)
		{
			auto area = get_area_entropy(i);
			areas.push_back(area);
			mean_area += area;
		}
		mean_area = mean_area / areas.size();
		double carry = 0;
		auto h = 1e-9;
		for (int i = 0; i < areas.size(); i++)
		{
			// move only non-zero control points
			//if (get_opacity(i) > EPSILON())
			{
				if (areas[i] > mean_area)
				{
					// get the upper vertex of an edge
					auto index = i;
					int max_next = index + 1;
					double weight1 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(index)), index);
					double weight2 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(max_next)), max_next);
					if (get_opacity(max_next) > EPSILON() && get_opacity(max_next) < 1 && weight2 > weight1)
					{
						index++;
						carry = 0;
					}

					const double step_size = get_stepsize();
					double height = get_opacity(index);
					//double height_max_new = height_max - step_size;
					set_opacity(index, height - h / 2);
					auto f_x_minus_half_h = get_area_entropy(i) - mean_area;
					set_opacity(index, height + h / 2);
					auto f_x_plus_half_h = get_area_entropy(i) - mean_area;
					auto gradient = 2 * (areas[i] - mean_area);
					auto slope = (f_x_plus_half_h - f_x_minus_half_h) / h;
					std::cout << "max slope=" << slope << " gradient=" << gradient << " step_size=" << step_size << std::endl;
					//double height_max_new = height_max - gradient;
					double height_new = height - slope + step_size * 2; // should be height - slope*step where step is 1 here
					//height_max_new = height_max_new < EPSILON() ? EPSILON() : height_max_new;
					height_new = std::max(height_new, EPSILON());
					height_new = std::min(height_new, 1.);
					set_opacity(index, height); // reset to original opacity
					result_opacity_list[index] = height_new; // update opacity

					if (carry > 0)
					{
						// if carry>0 then i>0 so i-1>=0
						auto a = std::abs(areas[i - 1] - mean_area);
						auto b = std::abs(areas[i] - mean_area);
						auto sum = a + b;
						auto lerp = (carry*a + height_new*b) / sum;
						result_opacity_list[index] = lerp;
					}
					carry = index > i ? height_new : 0;
				}
				if (areas[i] < mean_area)
				{
					auto index = i;
					// get the lower vertex of an edge
					int index_next = index + 1;
					double weight1 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(index)), index);
					double weight2 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(index_next)), index_next);
					if (get_opacity(index_next) > EPSILON() && get_opacity(index_next) < 1 && weight2 < weight1)
					{
						index++;
						carry = 0;
					}

					const double step_size = get_stepsize();
					double height = get_opacity(index);
					//double height_min_new = height_min + step_size;
					set_opacity(index, height - h / 2);
					auto f_x_minus_half_h = get_area_entropy(i) - mean_area;
					set_opacity(index, height + h / 2);
					auto f_x_plus_half_h = get_area_entropy(i) - mean_area;
					auto gradient = 2 * (areas[i] - mean_area);
					auto slope = (f_x_plus_half_h - f_x_minus_half_h) / h;
					std::cout << "min slope=" << slope << " gradient=" << gradient << " step_size=" << step_size << std::endl;
					//double height_min_new = height_min - gradient;
					double height_new = height - slope + step_size * 2; // should be height - slope*step where step is 1 here
					//height_min_new = height_min_new > 1 ? 1 : height_min_new;
					height_new = std::max(height_new, EPSILON());
					height_new = std::min(height_new, 1.);
					set_opacity(index, height); // reset to original opacity
					result_opacity_list[index] = height_new; // update opacity

					if (carry > 0)
					{
						// if carry>0 then i>0 so i-1>=0
						auto a = std::abs(areas[i - 1] - mean_area);
						auto b = std::abs(areas[i] - mean_area);
						auto sum = a + b;
						auto lerp = (carry*a + height_new*b) / sum;
						result_opacity_list[index] = lerp;
					}
					carry = index > i ? height_new : 0;
				}
			}
		}
		opacity_list = result_opacity_list;
	}

	void balance_transfer_function_edge_fixed_step_size()
	{
		std::vector<double> areas;
		double mean_area = 0;
		for (int i = 0; i < intensity_list_size() - 1; i++)
		{
			auto area = get_area_entropy(i);
			areas.push_back(area);
			mean_area += area;
		}
		mean_area = mean_area / areas.size();
		double carry = 0;
		for (int i = 0; i < areas.size(); i++)
		{
			// move only non-zero control points
			if (get_opacity(i) > EPSILON())
			{
				if (areas[i] > mean_area)
				{
					// get the upper vertex of an edge
					auto max_index = i;
					int max_index_next = max_index + 1;
					double weight_max_1 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(max_index)), max_index);
					double weight_max_2 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(max_index_next)), max_index_next);
					if (get_opacity(max_index_next) > EPSILON() && get_opacity(max_index_next) < 1 && weight_max_2 > weight_max_1)
					{
						max_index++;
						carry = 0;
					}

					const double step_size = get_stepsize();
					double height_max = get_opacity(max_index);
					double height_max_new = height_max - step_size;
					height_max_new = height_max_new < EPSILON() ? EPSILON() : height_max_new;
					//if (carry == 0)
					{
						set_opacity(max_index, height_max_new); // update opacity
					}

					//auto area_diff = areas[i] - get_area_entropy(i);
					//if (area_diff > 0)
					//{
					//	auto slope = (mean_area - areas[i]) / area_diff;
					//	//height_max_new = height_max + step_size * slope;
					//	auto gradient = -2 * (areas[i] - mean_area);
					//	height_max_new = height_max + gradient;
					//	std::cout << "max step_size*slope=" << step_size * slope << " gradient=" << gradient << " step_size=" << step_size << std::endl;
					//	height_max_new = height_max_new < EPSILON() ? EPSILON() : height_max_new;
					//	set_opacity(max_index, height_max_new); // update opacity
					//}

					//if (carry > 0)
					//{
					//	// if carry>0 then i>0 so i-1>=0
					//	auto a = std::abs(areas[i - 1] - mean_area);
					//	auto b = std::abs(areas[i] - mean_area);
					//	auto sum = a + b;
					//	auto lerp = (carry*a + height_max_new*b) / sum;
					//	set_opacity(max_index, lerp);
					//}
					carry = max_index > i ? height_max_new : 0;
				}
				if (areas[i] < mean_area)
				{
					auto min_index = i;
					// get the lower vertex of an edge
					int min_index_next = min_index + 1;
					double weight_min_1 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(min_index)), min_index);
					double weight_min_2 = get_entropy_opacity_by_index(denormalise_intensity(get_intensity(min_index_next)), min_index_next);
					if (get_opacity(min_index_next) > EPSILON() && get_opacity(min_index_next) < 1 && weight_min_2 < weight_min_1)
					{
						min_index++;
						carry = 0;
					}

					const double step_size = get_stepsize();
					double height_min = get_opacity(min_index);
					double height_min_new = height_min + step_size;
					height_min_new = height_min_new > 1 ? 1 : height_min_new;
					//if (carry == 0)
					{
						set_opacity(min_index, height_min_new); // update opacity
					}

					//auto area_diff = get_area_entropy(i) - areas[i];
					//if (area_diff > 0)
					//{
					//	auto slope = (mean_area - areas[i]) / area_diff;
					//	//height_min_new = height_min + step_size * slope;
					//	auto gradient = -2 * (areas[i] - mean_area);
					//	height_min_new = height_min + gradient;
					//	std::cout << "min step_size*slope=" << step_size * slope << " gradient=" << gradient << " step_size=" << step_size << std::endl;
					//	height_min_new = height_min_new > 1 ? 1 : height_min_new;
					//	set_opacity(min_index, height_min_new); // update opacity
					//}

					//if (carry > 0)
					//{
					//	// if carry>0 then i>0 so i-1>=0
					//	auto a = std::abs(areas[i - 1] - mean_area);
					//	auto b = std::abs(areas[i] - mean_area);
					//	auto sum = a + b;
					//	auto lerp = (carry*a + height_min_new*b) / sum;
					//	set_opacity(min_index, lerp);
					//}
					carry = min_index > i ? height_min_new : 0;
				}
			}
		}
	}

	// a replacement of balance_opacity_for_region
	// region-based or hue-based optimization
	void balance_transfer_function_edge_for_region()
	{
		int max_index = -1;
		int min_index = -1;
		double max_area = std::numeric_limits<int>::min();
		double min_area = std::numeric_limits<int>::max();

		for (int i = 0; i<intensity_list_size() - 1; i++)
		{
			if (get_opacity(i) > EPSILON())
			{
				double area = get_weighted_area_entropy(i);
				if (area > max_area)
				{
					max_index = i;
					max_area = area;
				}
				if (area < min_area && get_opacity(i) < 1)
				{
					min_index = i;
					min_area = area;
				}
			}
		}
		if (min_index != max_index)
		{
			// get the upper vertex of an edge
			int max_index_next = max_index + 1;
			double weight_max_1 = get_weighted_entropy_opacity_by_index(denormalise_intensity(get_intensity(max_index)), max_index);
			double weight_max_2 = get_weighted_entropy_opacity_by_index(denormalise_intensity(get_intensity(max_index_next)), max_index_next);
			if (get_opacity(max_index_next) > EPSILON() && get_opacity(max_index_next) < 1 && weight_max_2 > weight_max_1)
			{
				max_index++;
			}

			// get the lower vertex of an edge
			int min_index_next = min_index + 1;
			double weight_min_1 = get_weighted_entropy_opacity_by_index(denormalise_intensity(get_intensity(min_index)), min_index);
			double weight_min_2 = get_weighted_entropy_opacity_by_index(denormalise_intensity(get_intensity(min_index_next)), min_index_next);
			if (get_opacity(min_index_next) > EPSILON() && get_opacity(min_index_next) < 1 && weight_min_2 < weight_min_1)
			{
				min_index++;
			}

			const double step_size = get_stepsize();
			double height_max = get_opacity(max_index);
			double height_max_new = height_max - step_size;
			height_max_new = height_max_new < EPSILON() ? EPSILON() : height_max_new;
			double area = get_weighted_neighbour_area_entropy(max_index);
			set_opacity(max_index, height_max_new); // update opacity
			double new_area = get_weighted_neighbour_area_entropy(max_index); // calculate new area using new opacity
			double area_decreased = area - new_area;
			//double height_increased = get_height_given_area_increment(min_index, area_decreased);
			double height_min = get_opacity(min_index);
			double height_min_new = height_min + step_size;
			height_min_new = height_min_new > 1 ? 1 : height_min_new;
			set_opacity(min_index, height_min_new); // update opacity
			//std::cout<<"balance TF entropy max index="<<max_index<<" min index="<<min_index<<" opacity="<<height_max<<" new opacity="<<height_max_new<<" area="<<area<<" new area="<<new_area<<" height="<<height_min<<" new height="<<height_min_new<<endl;
		}
	}

	void reduce_opacity()
	{
		int max_index = -1;
		double max_area = std::numeric_limits<int>::min();

		for (int i = 0; i<intensity_list_size(); i++)
		{
			if (get_opacity(i) > EPSILON())
			{
				double area = get_neighbour_area_entropy(i);
				if (area > max_area)
				{
					max_index = i;
					max_area = area;
				}
			}
		}
		if (-1 != max_index)
		{
			const double step_size = get_stepsize();
			double height_max = get_opacity(max_index);
			double height_max_new = height_max - step_size;
			height_max_new = height_max_new < EPSILON() ? EPSILON() : height_max_new;
			double area = get_neighbour_area_entropy(max_index);
			set_opacity(max_index, height_max_new);
			double new_area = get_neighbour_area_entropy(max_index);
			double area_decreased = area - new_area;
		}
	}

	void increase_opacity()
	{
		int min_index = -1;
		double min_area = std::numeric_limits<int>::max();


		for (int i = 0; i<intensity_list_size(); i++)
		{
			if (get_opacity(i) > EPSILON())
			{
				double area = get_neighbour_area_entropy(i);
				if (area < min_area && get_opacity(i) < 1)
				{
					min_index = i;
					min_area = area;
				}
			}
		}
		if (min_index != -1)
		{
			const double step_size = get_stepsize();
			double height_min = get_opacity(min_index);
			double height_min_new = height_min + step_size;
			height_min_new = height_min_new > 1 ? 1 : height_min_new;
			set_opacity(min_index, height_min_new);
		}
	}

	void reduce_opacity_for_region()
	{
		int max_index = -1;
		double max_area = std::numeric_limits<int>::min();

		for (int i = 0; i<intensity_list_size(); i++)
		{
			if (get_opacity(i) > EPSILON())
			{
				double area = get_weighted_neighbour_area_entropy(i);
				if (area > max_area)
				{
					max_index = i;
					max_area = area;
				}
			}
		}
		if (-1 != max_index)
		{
			const double step_size = get_stepsize();
			double height_max = get_opacity(max_index);
			double height_max_new = height_max - step_size;
			height_max_new = height_max_new < EPSILON() ? EPSILON() : height_max_new;
			double area = get_weighted_neighbour_area_entropy(max_index);
			set_opacity(max_index, height_max_new);
			double new_area = get_weighted_neighbour_area_entropy(max_index);
			double area_decreased = area - new_area;
		}
	}

	void increase_opacity_for_region()
	{
		int min_index = -1;
		double min_area = std::numeric_limits<int>::max();

		for (int i = 0; i<intensity_list_size(); i++)
		{
			if (get_opacity(i) > EPSILON())
			{
				double area = get_weighted_neighbour_area_entropy(i);
				if (area < min_area && get_opacity(i) < 1)
				{
					min_index = i;
					min_area = area;
				}
			}
		}
		if (min_index != -1)
		{
			const double step_size = get_stepsize();
			double height_min = get_opacity(min_index);
			double height_min_new = height_min + step_size;
			height_min_new = height_min_new > 1 ? 1 : height_min_new;
			set_opacity(min_index, height_min_new);
		}
	}

	void saveTransferFunctionToVoreenXML(const char *filename)
	{
		tinyxml2::XMLDocument doc;

		auto declaration = doc.NewDeclaration();
		doc.InsertEndChild(declaration);
		auto voreenData = doc.NewElement("VoreenData");
		voreenData->SetAttribute("version", 1);
		auto transFuncIntensity = doc.NewElement("TransFuncIntensity");
		transFuncIntensity->SetAttribute("type", "TransFuncIntensity");

		// add alphaMode
		auto alphaMode = doc.NewElement("alphaMode");
		alphaMode->SetAttribute("value", 1);
		transFuncIntensity->InsertEndChild(alphaMode);

		// add gammaValue
		auto gammaValue = doc.NewElement("gammaValue");
		gammaValue->SetAttribute("value", 1);
		transFuncIntensity->InsertEndChild(gammaValue);

		// add domain
		auto domain = doc.NewElement("domain");
		domain->SetAttribute("x", Domain_x());
		domain->SetAttribute("y", Domain_y());
		transFuncIntensity->InsertEndChild(domain);

		// add threshold
		auto threshold = doc.NewElement("threshold");
		threshold->SetAttribute("x", Threshold_x());
		threshold->SetAttribute("y", Threshold_y());
		transFuncIntensity->InsertEndChild(threshold);

		// add Keys
		auto keys = doc.NewElement("Keys");
		for (int i = 0; i < intensity_list_size(); i++)
		{
			auto key = doc.NewElement("key");
			key->SetAttribute("type", "TransFuncMappingKey");
			auto intensity = doc.NewElement("intensity");
			intensity->SetAttribute("value", get_intensity(i));
			auto split = doc.NewElement("split");
			split->SetAttribute("value", "false");
			auto colorL = doc.NewElement("colorL");
			double rgb[3];
			get_color_by_intensity(denormalise_intensity(get_intensity(i)), rgb);
			colorL->SetAttribute("r", denormalise_rgba(rgb[0]));
			colorL->SetAttribute("g", denormalise_rgba(rgb[1]));
			colorL->SetAttribute("b", denormalise_rgba(rgb[2]));
			colorL->SetAttribute("a", denormalise_rgba(get_opacity(i)));
			key->InsertEndChild(intensity);
			key->InsertEndChild(split);
			key->InsertEndChild(colorL);
			keys->InsertEndChild(key);
		}
		transFuncIntensity->InsertEndChild(keys);

		voreenData->InsertEndChild(transFuncIntensity);
		doc.InsertEndChild(voreenData);

		auto r = doc.SaveFile(filename);
		if (r != tinyxml2::XML_NO_ERROR)
		{
			std::cout << "failed to save file " << filename << endl;
		}
	}

	void saveTransferFunctionToMITKXML(const char *filename)
	{
		tinyxml2::XMLDocument doc;

		auto declaration = doc.NewDeclaration();
		auto version = doc.NewElement("Version");
		version->SetAttribute("TransferfunctionVersion", 1);
		auto transferFunction = doc.NewElement("TransferFunction");
		auto scalarOpacity = doc.NewElement("ScalarOpacity");
		auto gradientOpacity = doc.NewElement("GradientOpacity");
		auto color = doc.NewElement("Color");

		for (int i = 0; i < scalar_opacity->GetSize(); i++)
		{
			double xa[4];
			scalar_opacity->GetNodeValue(i, xa);
			auto point = doc.NewElement("point");
			point->SetAttribute("x", xa[0]);
			point->SetAttribute("y", xa[1]);
			scalarOpacity->InsertEndChild(point);
		}

		for (int i = 0; i < gradient_opacity->GetSize(); i++)
		{
			double xa[4];
			gradient_opacity->GetNodeValue(i, xa);
			auto point = doc.NewElement("point");
			point->SetAttribute("x", xa[0]);
			point->SetAttribute("y", xa[1]);
			gradientOpacity->InsertEndChild(point);
		}

		for (int i = 0; i < scalar_color->GetSize(); i++)
		{
			double xrgb[6];
			scalar_color->GetNodeValue(i, xrgb);
			auto point = doc.NewElement("point");
			point->SetAttribute("x", xrgb[0]);
			point->SetAttribute("r", xrgb[1]);
			point->SetAttribute("g", xrgb[2]);
			point->SetAttribute("b", xrgb[3]);
			point->SetAttribute("midpoint", xrgb[4]);
			point->SetAttribute("sharpness", xrgb[5]);
			color->InsertEndChild(point);
		}

		transferFunction->InsertEndChild(scalarOpacity);
		transferFunction->InsertEndChild(gradientOpacity);
		transferFunction->InsertEndChild(color);

		doc.InsertEndChild(declaration);
		doc.InsertEndChild(version);
		doc.InsertEndChild(transferFunction);

		auto r = doc.SaveFile(filename);
		if (r != tinyxml2::XML_NO_ERROR)
		{
			std::cout << "failed to save file " << filename << endl;
		}
	}

	/// open MITK/Slicer transfer functions
	void openTransferFunctionFromMITKXML(const char *filename)
	{
		intensity_list_clear();
		colour_list_clear();
		opacity_list_clear();

		TransferFunctionXML xml;
		xml.parse(filename);
		auto gradient = xml.Volume()->GetGradientOpacity();
		auto scalar = xml.Volume()->GetScalarOpacity();
		auto color = xml.Volume()->GetRGBTransferFunction();

		gradient_opacity->RemoveAllPoints();
		//std::cout << "GetGradientOpacity size=" << gradient->GetSize() << std::endl;
		for (int i = 0; i < gradient->GetSize(); i++)
		{
			double xa[4];
			gradient->GetNodeValue(i, xa);
			//std::cout << xa[0] << " " << xa[1] << std::endl;
			gradient_opacity->AddPoint(xa[0], xa[1]);
		}

		scalar_opacity->RemoveAllPoints();
		//std::cout << "GetScalarOpacity size=" << scalar->GetSize() << std::endl;
		for (int i = 0; i < scalar->GetSize(); i++)
		{
			double xa[4];
			scalar->GetNodeValue(i, xa);
			//std::cout << xa[0] << " " << xa[1] << std::endl;
			scalar_opacity->AddPoint(xa[0], xa[1]);
		}

		scalar_color->RemoveAllPoints();
		//std::cout << "GetRGBTransferFunction size=" << color->GetSize() << std::endl;
		for (int i = 0; i < color->GetSize(); i++)
		{
			double xrgb[6];
			color->GetNodeValue(i, xrgb);
			//std::cout << xrgb[0] << " " << xrgb[1] << " " << xrgb[2] << " " << xrgb[3] << std::endl;
			scalar_color->AddRGBPoint(xrgb[0], xrgb[1], xrgb[2], xrgb[3]);
		}
	}

	/// open Voreen transfer functions
	void openTransferFunctionFromVoreenXML(const char *filename)
	{
		ui->statusBar->showMessage(QString(filename));

		tinyxml2::XMLDocument doc;
		auto r = doc.LoadFile(filename);

		if (r != tinyxml2::XML_NO_ERROR)
		{
			std::cout << "failed to open file " << filename << endl;
			return;
		}

		auto transFuncIntensity = doc.FirstChildElement("VoreenData")->FirstChildElement("TransFuncIntensity");

		auto threshold = transFuncIntensity->FirstChildElement("threshold");
		if (threshold != NULL)
		{
			Threshold_x(atof(threshold->Attribute("x")));
			Threshold_y(atof(threshold->Attribute("y")));
		}
		else
		{
			Threshold_x(atof(transFuncIntensity->FirstChildElement("lower")->Attribute("value")));
			Threshold_y(atof(transFuncIntensity->FirstChildElement("upper")->Attribute("value")));
		}

		auto domain = transFuncIntensity->FirstChildElement("domain");
		if (domain != NULL)
		{
			Domain_x(atof(domain->Attribute("x")));
			Domain_y(atof(domain->Attribute("y")));
			//std::cout << "domain x=" << Domain_x() << " y=" << Domain_y() << std::endl;
		}
		else
		{
			Domain_x(0);
			Domain_y(1);
			//std::cout << "domain doesn't exist. default: " << Domain_x() << " " << Domain_y() << std::endl;
		}

		auto key = doc.FirstChildElement("VoreenData")->FirstChildElement("TransFuncIntensity")->FirstChildElement("Keys")->FirstChildElement("key");
		intensity_list_clear();
		colour_list_clear();
		opacity_list_clear();
		do
		{
			double intensity = atof(key->FirstChildElement("intensity")->Attribute("value"));
			intensity_list_push_back(intensity);
			int r = atoi(key->FirstChildElement("colorL")->Attribute("r"));
			int g = atoi(key->FirstChildElement("colorL")->Attribute("g"));
			int b = atoi(key->FirstChildElement("colorL")->Attribute("b"));
			int a = atoi(key->FirstChildElement("colorL")->Attribute("a"));
			std::vector<double> colour;
			colour.push_back(normalise_rgba(r));
			colour.push_back(normalise_rgba(g));
			colour.push_back(normalise_rgba(b));
			colour_list_push_back(colour);
			opacity_list_push_back(normalise_rgba(a));

			bool split = (0 == strcmp("true", key->FirstChildElement("split")->Attribute("value")));
			//std::cout << "intensity=" << intensity;
			//std::cout << "\tsplit=" << (split ? "true" : "false");
			//std::cout << "\tcolorL r=" << r << " g=" << g << " b=" << b << " a=" << a;
			const double epsilon = 1e-6;
			if (split)
			{
				intensity_list_push_back(intensity + epsilon);
				auto colorR = key->FirstChildElement("colorR");
				int r2 = atoi(colorR->Attribute("r"));
				int g2 = atoi(colorR->Attribute("g"));
				int b2 = atoi(colorR->Attribute("b"));
				int a2 = atoi(colorR->Attribute("a"));
				std::vector<double> colour2;
				colour2.push_back(normalise_rgba(r2));
				colour2.push_back(normalise_rgba(g2));
				colour2.push_back(normalise_rgba(b2));
				colour_list_push_back(colour2);
				opacity_list_push_back(normalise_rgba(a2));
				//std::cout << "\tcolorR r=" << r2 << " g=" << g2 << " b=" << b2 << " a=" << a2;
			}
			//std::cout << endl;

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

		intensity_list_clear();
		colour_list_clear();
		opacity_list_clear();
		Domain_x(0);
		Threshold_x(0);
		Domain_y(1);
		Threshold_y(1);

		{
			intensity_list_push_back(normalise_rgba(0));

			std::vector<double> v;
			v.push_back(0);
			v.push_back(0);
			v.push_back(0);
			colour_list_push_back(v);
			opacity_list_push_back(0);
		}
		{
			intensity_list_push_back(normalise_rgba(36));

			std::vector<double> v;
			v.push_back(1);
			v.push_back(0);
			v.push_back(0);
			colour_list_push_back(v);
			opacity_list_push_back(0.125);
		}
		{
			intensity_list_push_back(normalise_rgba(72));

			std::vector<double> v;
			v.push_back(1);
			v.push_back(1);
			v.push_back(0);
			colour_list_push_back(v);
			opacity_list_push_back(0.25);
		}
		{
			intensity_list_push_back(normalise_rgba(108));

			std::vector<double> v;
			v.push_back(0);
			v.push_back(1);
			v.push_back(0);
			colour_list_push_back(v);
			opacity_list_push_back(0.375);
		}
		{
			intensity_list_push_back(normalise_rgba(144));

			std::vector<double> v;
			v.push_back(0);
			v.push_back(1);
			v.push_back(1);
			colour_list_push_back(v);
			opacity_list_push_back(0.5);
		}
		{
			intensity_list_push_back(normalise_rgba(180));

			std::vector<double> v;
			v.push_back(0);
			v.push_back(0);
			v.push_back(1);
			colour_list_push_back(v);
			opacity_list_push_back(0.625);
		}
		{
			intensity_list_push_back(normalise_rgba(216));

			std::vector<double> v;
			v.push_back(1);
			v.push_back(0);
			v.push_back(1);
			colour_list_push_back(v);
			opacity_list_push_back(0.75);
		}
		{
			intensity_list_push_back(normalise_rgba(255));

			std::vector<double> v;
			v.push_back(1);
			v.push_back(1);
			v.push_back(1);
			colour_list_push_back(v);
			opacity_list_push_back(0);
		}
	}

	void generate_spectrum_transfer_function(int number_of_colours = 6)
	{
		if (number_of_colours < 1 || number_of_colours > 256)
		{
			std::cout << "number_of_control_points should belongs to [1,256]" << std::endl;
			return;
		}

		int n = number_of_colours; // 1 to 6 groups of control points
		std::vector<std::vector<double>> spectrum;
		for (int i = 0; i < n; i++)
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

		intensity_list_clear();
		colour_list_clear();
		opacity_list_clear();
		Domain_x(0);
		Threshold_x(0);
		Domain_y(1);
		Threshold_y(1);

		intensity_list_push_back(0);
		{
			std::vector<double> v;
			v.push_back(0);
			v.push_back(0);
			v.push_back(0);
			colour_list_push_back(v);
			opacity_list_push_back(0);
		}

		for (int i = 0; i < m*n; i++)
		{
			int colour_index = i / m;
			double opacity = (i % m == 1) ? 0.5 : 0;

			intensity_list_push_back((i + 1) * interval);
			{
				std::vector<double> v;
				v.push_back(spectrum[colour_index][0]);
				v.push_back(spectrum[colour_index][1]);
				v.push_back(spectrum[colour_index][2]);
				colour_list_push_back(v);
				opacity_list_push_back(opacity);
			}
		}

		intensity_list_push_back(1);
		{
			std::vector<double> v;
			v.push_back(1);
			v.push_back(1);
			v.push_back(1);
			colour_list_push_back(v);
			opacity_list_push_back(0);
		}
	}

	void generate_spectrum_ramp_transfer_function(int number_of_colours = 6)
	{
		if (number_of_colours < 1 || number_of_colours > 256)
		{
			std::cout << "number_of_control_points should belongs to [1,256]" << std::endl;
			return;
		}

		int n = number_of_colours; // 1 to 6 groups of control points
		std::vector<std::vector<double>> spectrum;
		for (int i = 0; i < n; i++)
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

		const int m = 1; // 3 control points each group
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

		intensity_list_clear();
		colour_list_clear();
		opacity_list_clear();
		Domain_x(0);
		Threshold_x(0);
		Domain_y(1);
		Threshold_y(1);

		intensity_list_push_back(0);
		{
			std::vector<double> v;
			v.push_back(0);
			v.push_back(0);
			v.push_back(0);
			colour_list_push_back(v);
			opacity_list_push_back(0);
		}

		for (int i = 0; i < m*n; i++)
		{
			int colour_index = i / m;

			intensity_list_push_back((i + 1) * interval);
			{
				std::vector<double> v;
				v.push_back(spectrum[colour_index][0]);
				v.push_back(spectrum[colour_index][1]);
				v.push_back(spectrum[colour_index][2]);
				colour_list_push_back(v);
				opacity_list_push_back(0.5 / 3);
			}
		}

		intensity_list_push_back(1);
		{
			std::vector<double> v;
			v.push_back(1);
			v.push_back(1);
			v.push_back(1);
			colour_list_push_back(v);
			opacity_list_push_back(0);
		}
	}

	void generate_visibility_function(vtkSmartPointer<vtkImageAlgorithm> reader)
	{
		int ignoreZero = 0;
		int numComponents = reader->GetOutput()->GetNumberOfScalarComponents();
		std::cout << "generateVisibilityFunction component number=" << numComponents << endl;
		if (numComponents > 3)
		{
			std::cout << "Error: cannot process an image with "
				<< numComponents << " components!" << std::endl;
			//return EXIT_FAILURE;
		}

		double xmax = 0.;
		double ymax = 0.;

		// Process the image, extracting and plotting a histogram for each component
		for (int i = 0; i < numComponents; ++i)
		{
			vtkSmartPointer<vtkImageExtractComponents> extract =
				vtkSmartPointer<vtkImageExtractComponents>::New();
			extract->SetInputConnection(reader->GetOutputPort());
			extract->SetComponents(i);
			extract->Update();

			double range[2];
			extract->GetOutput()->GetScalarRange(range);
			std::cout << "range " << range[0] << " " << range[1] << endl;

			vtkSmartPointer<vtkImageAccumulate> histogram =
				vtkSmartPointer<vtkImageAccumulate>::New();
			histogram->SetInputConnection(extract->GetOutputPort());
			histogram->SetComponentExtent(
				0,
				static_cast<int>(range[1]) - static_cast<int>(range[0]) - 1, 0, 0, 0, 0);
			histogram->SetComponentOrigin(range[0], 0, 0);
			histogram->SetComponentSpacing(1, 0, 0);
			histogram->SetIgnoreZero(ignoreZero);
			histogram->Update();

			x_min = range[0];
			x_max = range[1];
			if (range[1] > xmax)
			{
				xmax = range[1];
			}
			if (histogram->GetOutput()->GetScalarRange()[1] > ymax)
			{
				ymax = histogram->GetOutput()->GetScalarRange()[1];
			}
			std::cout << "histogram range " << histogram->GetOutput()->GetScalarRange()[0] << " " << histogram->GetOutput()->GetScalarRange()[1] << endl;

			//if (i == 0)
			{
				y_min = histogram->GetOutput()->GetScalarRange()[0];
				y_max = histogram->GetOutput()->GetScalarRange()[1];
				std::cout << "min=" << y_min << " max=" << y_max << endl;

#ifdef OUTPUT_TO_FILE
				char buffer[32];
				itoa(i, buffer, 10);
				char filename[32];
				sprintf(filename, "../%s.csv", buffer);
				std::cout<<"output file "<<filename<<std::endl;
				std::ofstream myfile(filename);
#endif

				// There is a potential problem. The range may not be [0,255]
				frequency_list.clear();
				frequency_list.reserve(Range_y());
				std::cout << "histogram type is " << histogram->GetOutput()->GetScalarTypeAsString() << std::endl;
				auto pixels = static_cast<int *>(histogram->GetOutput()->GetScalarPointer());
				int extent[6];
				histogram->GetOutput()->GetExtent(extent);
				std::cout << "histogram extent " << extent[0] << " " << extent[1] << std::endl;
				count_of_voxels = 0;
				const int max = extent[1];
				for (int j = 0; j <= max + 1; j++)
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
		std::cout << "generate_LH_histogram component number=" << numComponents << endl;
		if (numComponents > 1)
		{
			std::cout << "Error: cannot process an image with "
				<< numComponents << " components!" << std::endl;
			//return EXIT_FAILURE;
		}

		for (int i = 0; i < numComponents; ++i)
		{
			vtkSmartPointer<vtkImageExtractComponents> extract =
				vtkSmartPointer<vtkImageExtractComponents>::New();
			extract->SetInputConnection(reader->GetOutputPort());
			extract->SetComponents(i);
			extract->Update();

			double range[2];
			extract->GetOutput()->GetScalarRange(range);
			std::cout << "range " << range[0] << " " << range[1] << endl;

			double intensity_min = range[0];
			double intensity_max = range[1];

			auto imageData = extract->GetOutput();
			int dimensions[3];
			imageData->GetDimensions(dimensions);
			int count_of_voxels = dimensions[0] * dimensions[1] * dimensions[2];
			std::cout << "dimension " << dimensions[0] << " " << dimensions[1] << " " << dimensions[2] << " count=" << count_of_voxels << std::endl;
			std::cout << "voxel type is " << imageData->GetScalarTypeAsString() << std::endl;

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

	/// depreciated. use get_energy_function_edge() instead.
	/// The energy function is the variance of control point weights.
	double get_energy_function()
	{
		std::vector<double> weights;

		// compute the mean
		double sum = 0;
		for (int i = 0; i < intensity_list_size(); i++)
		{
			double w = get_neighbour_area_entropy(i);
			sum += w;
			weights.push_back(w);
		}
		double mean = sum / weights.size();

		// compute and return the variance
		sum = 0;
		for (unsigned int i = 0; i < weights.size(); i++)
		{
			double diff = weights[i] - mean;
			sum += diff * diff;
		}
		return sum;
	}

	/// a replacement of get_energy_function()
	/// The energy function is the variance of control point weights.
	double get_energy_function_edge()
	{
		std::vector<double> weights;

		// compute the mean
		double sum = 0;
		for (int i = 0; i < intensity_list_size() - 1; i++)
		{
			double w = get_area_entropy(i);
			sum += w;
			weights.push_back(w);
		}
		double mean = sum / weights.size();

		// compute and return the variance
		sum = 0;
		for (unsigned int i = 0; i < weights.size(); i++)
		{
			double diff = weights[i] - mean;
			sum += diff * diff;
		}
		return sum;
	}

	/// depreciated. use get_energy_function_edge_weighted_for_region() instead.
	/// The energy function is the variance of control point weights for region
	double get_energy_function_weighted_for_region()
	{
		std::vector<double> weights;

		// compute the mean
		double sum = 0;
		for (int i = 0; i < intensity_list_size(); i++)
		{
			double w = get_weighted_neighbour_area_entropy(i);
			sum += w;
			weights.push_back(w);
		}
		double mean = sum / weights.size();

		// compute and return the variance
		sum = 0;
		for (unsigned int i = 0; i < weights.size(); i++)
		{
			double diff = weights[i] - mean;
			sum += diff * diff;
		}
		return sum;
	}

	/// a replacement of get_energy_function_weighted_for_region()
	/// The energy function is the variance of control point weights for region
	double get_energy_function_edge_weighted_for_region()
	{
		std::vector<double> weights;

		// compute the mean
		double sum = 0;
		for (int i = 0; i < intensity_list_size() - 1; i++)
		{
			double w = get_weighted_area_entropy(i);
			sum += w;
			weights.push_back(w);
		}
		double mean = sum / weights.size();

		// compute and return the variance
		sum = 0;
		for (unsigned int i = 0; i < weights.size(); i++)
		{
			double diff = weights[i] - mean;
			sum += diff * diff;
		}
		return sum;
	}

	// open a volume file and render it
	void open_volume(QString filename)
	{
		// show filename on window title
		this->setWindowTitle(Window_title() + QString::fromUtf8(" - ") + filename);

		// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
		QByteArray ba = filename.toLocal8Bit();
		const char *filename_str = ba.data();

		if (!file_exist(filename_str))
		{
			std::cout << "failed to open file " << filename_str << endl;
			return;
		}

		vtkSmartPointer<vtkImageReader2> reader;
		std::cout << "volume file: " << filename_str << std::endl;
		auto p1 = strstr(filename_str, ".mhd");
		auto p2 = strstr(filename_str, ".mha");
		if (p1 || p2)
		{
			// read UNC MetaImage (.mhd or .mha) files
			reader = vtkSmartPointer<vtkMetaImageReader>::New();
		}
		else
		{
			// read Nearly Raw Raster Data (*.nrrd *.nhdr) files
			reader = vtkSmartPointer<vtkNrrdReader>::New();
		}
		if (!reader->CanReadFile(filename_str))
		{
			std::cerr << "Reader reports " << filename_str << " cannot be read.";
			exit(EXIT_FAILURE);
		}
		reader->SetFileName(filename_str);

		auto extract = vtkSmartPointer<vtkImageExtractComponents>::New();
		extract->SetInputConnection(reader->GetOutputPort());
		extract->Update();

		double range[2];
		extract->GetOutput()->GetScalarRange(range);
		Range_x(range[0]);
		Range_y(range[1]);
		std::cout << "range of voxels " << Range_x() << " " << Range_y() << endl;

		// scale the volume data to unsigned char (0-255) before passing it to volume mapper
		auto shiftScale = vtkSmartPointer<vtkImageShiftScale>::New();
		shiftScale->SetInputConnection(reader->GetOutputPort());
		if (Range_y() > UCHAR_MAX)
		{
			shiftScale->SetOutputScalarTypeToUnsignedShort();
		}
		else
		{
			shiftScale->SetOutputScalarTypeToUnsignedChar();
		}

		// generate histograms
		generate_visibility_function(shiftScale);;
		generate_LH_histogram(shiftScale);

		// set up volume property
		auto volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
		volumeProperty->SetScalarOpacity(scalar_opacity);
		volumeProperty->SetGradientOpacity(gradient_opacity);
		volumeProperty->SetColor(scalar_color);
		//volumeProperty->ShadeOff();
		volumeProperty->SetInterpolationTypeToLinear();

		// assign volume property to the volume property widget
		volume_property_widget.setVolumeProperty(volumeProperty);

		// The mapper that renders the volume data.
		vtkSmartPointer<vtkVolumeMapper> volumeMapper;

		switch (Volume_mapper_index())
		{
		case 1:
			volumeMapper = vtkSmartPointer<vtkSlicerGPURayCastVolumeMapper>::New();
			break;
		case 2:
			volumeMapper = vtkSmartPointer<vtkSlicerGPURayCastMultiVolumeMapper>::New();
			break;
		default:
			volumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
			break;
		}

		volumeMapper->SetInputConnection(shiftScale->GetOutputPort());

		// The volume holds the mapper and the property and can be used to position/orient the volume.
		auto volume = vtkSmartPointer<vtkVolume>::New();
		Volume(volume);
		volume->SetMapper(volumeMapper);
		volume->SetProperty(volumeProperty);

		//// add the volume into the renderer
		renderer->RemoveAllViewProps();
		renderer->AddVolume(volume);
		renderer->SetBackground(1, 1, 1);

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
			std::cout << "\nremove vtkRenderer " << ++rc << std::endl;
		}
		window->AddRenderer(renderer);
		window->Render();

		// initialize the interactor
		interactor->Initialize();
		interactor->Start();
	}

	// open a volume dataset without rendering it
	void open_volume_no_rendering(QString filename)
	{
		// show filename on window title
		this->setWindowTitle(QString::fromUtf8("Volume Renderer - ") + filename);

		// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
		QByteArray ba = filename.toLocal8Bit();
		const char *filename_str = ba.data();

		if (!file_exist(filename_str))
		{
			std::cout << "failed to open file " << filename_str << endl;
			return;
		}

		// read Meta Image (.mhd or .mha) files
		auto reader = vtkSmartPointer<vtkMetaImageReader>::New();
		reader->SetFileName(filename_str);

		// scale the volume data to unsigned char (0-255) before passing it to volume mapper
		auto shiftScale = vtkSmartPointer<vtkImageShiftScale>::New();
		shiftScale->SetInputConnection(reader->GetOutputPort());
		shiftScale->SetOutputScalarTypeToUnsignedChar();

		// generate histograms
		generate_visibility_function(shiftScale);
		generate_LH_histogram(shiftScale);

		// set up volume property
		auto volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
		volumeProperty->SetScalarOpacity(scalar_opacity);
		volumeProperty->SetGradientOpacity(gradient_opacity);
		volumeProperty->SetColor(scalar_color);
		//volumeProperty->ShadeOff();
		volumeProperty->SetInterpolationTypeToLinear();

		// assign volume property to the volume property widget
		volume_property_widget.setVolumeProperty(volumeProperty);

		// The mapper that renders the volume data.
		vtkSmartPointer<vtkVolumeMapper> volumeMapper;

		switch (Volume_mapper_index())
		{
		case 1:
			volumeMapper = vtkSmartPointer<vtkSlicerGPURayCastVolumeMapper>::New();
			break;
		case 2:
			volumeMapper = vtkSmartPointer<vtkSlicerGPURayCastMultiVolumeMapper>::New();
			break;
		default:
			volumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
			break;
		}

		volumeMapper->SetInputConnection(shiftScale->GetOutputPort());

		// The volume holds the mapper and the property and can be used to position/orient the volume.
		auto volume = vtkSmartPointer<vtkVolume>::New();
		volume->SetMapper(volumeMapper);
		volume->SetProperty(volumeProperty);

		renderer->RemoveAllViewProps();
		renderer->AddVolume(volume);
		renderer->SetBackground(1, 1, 1);
		vtk_widget.GetRenderWindow()->Render();
	}

	/// if distance_metric==1 then compute distance with squared distance
	void read_region_image_and_compute_distance(int squared = 0, int hsv = 0)
	{
		// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
		QByteArray ba = selected_region_filename.toLocal8Bit();
		const char *filename_str = ba.data();

		std::cout << "image file: " << filename_str << endl;

		auto reader = vtkSmartPointer<vtkPNGReader>::New();
		if (!reader->CanReadFile(filename_str))
		{
			std::cout << "Error: cannot read " << filename_str << std::endl;
			return;
		}
		reader->SetFileName(filename_str);
		reader->Update();

		int numComponents = reader->GetOutput()->GetNumberOfScalarComponents();
		std::cout << "component number=" << numComponents << endl;
		if (numComponents > 3)
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
		std::cout << "dimension " << dimensions[0] << " " << dimensions[1] << " " << dimensions[2] << " count=" << count_of_pixels << std::endl;
		std::cout << "pixel type is " << imageData->GetScalarTypeAsString() << std::endl;
		auto pixels = static_cast<unsigned char *>(imageData->GetScalarPointer());

#ifdef OUTPUT_TO_FILE
		// compute and write the histogram to file
		int rgb_histogram[256][3];
		std::memset(rgb_histogram, 0, 256 * 3 * sizeof(int));

		for (int i = 0; i < count_of_pixels; i++)
		{
			for (int j = 0; j < numComponents; j++)
			{
				rgb_histogram[pixels[i*numComponents + j]][j]++;
			}
		}

		char filename2[32] = "../rgb_histogram.csv";
		std::cout << "rgb_histogram file " << filename2 << std::endl;
		std::ofstream out2(filename2);

		for (int i = 0; i < 256; i++)
		{
			for (int j = 0; j < numComponents; j++)
			{
				out2 << rgb_histogram[i][j];
				if (j < numComponents - 1)
				{
					out2 << ",";
				}
			}
			out2 << endl;
		}
		out2.close();
#endif

		// compute region weights based on the selected image
		control_point_weight_list.clear();
		double sum = 0;
		for (int i = 0; i<colour_list_size(); i++)
		{
			double r = get_colour_r(i);
			double g = get_colour_g(i);
			double b = get_colour_b(i);
			double distance = get_distance_between_colour_and_pixels_selector(r, g, b, pixels, count_of_pixels, numComponents, squared, hsv);
			control_point_weight_list.push_back(distance);
			sum += distance;
		}

		if (sum > 0)
		{
			// normalize the distances
			for (unsigned int i = 0; i < control_point_weight_list.size(); i++)
			{
				control_point_weight_list[i] = control_point_weight_list[i] / sum;
			}
		}
	}

	void updateTFWidgetFromOpacityArrays()
	{
		if (intensity_list_size() == 0 || colour_list_size() == 0)
		{
			generate_default_transfer_function();
		}
		if (intensity_list_size() > 0 && intensity_list_size() == colour_list_size())
		{
			scalar_opacity->RemoveAllPoints();
			for (int i = 0; i < intensity_list_size(); i++)
			{
				scalar_opacity->AddPoint(denormalise_intensity(get_intensity(i)), get_opacity(i));
			}
		}
		// update vtk widget
		vtk_widget.repaint();
	}

	void update_colour_palette()
	{
		draw_spectrum_in_graphicsview();
	}

	void updateOpacityArrayFromTFWidget()
	{
		if (scalar_opacity->GetSize() < 1)
		{
			QMessageBox msgBox;
			msgBox.setText("Error: vtkPiecewiseFunction is empty.");
			int ret = msgBox.exec();
			return;
		}

		opacity_list_clear();
		intensity_list_clear();
		for (auto i = 0; i < scalar_opacity->GetSize(); i++)
		{
			double xa[4];
			scalar_opacity->GetNodeValue(i, xa);
			double opacity = xa[1];
			double intensity = normalise_intensity(xa[0]);
			opacity_list_push_back(opacity);
			intensity_list_push_back(intensity);
		}
	}

	void generate_spectrum_transfer_function_and_check_menu_item()
	{
		ui->action_Spectrum_Ramp_Transfer_Function->setChecked(false);
		ui->action_Spectrum_Transfer_Function->setChecked(true);
		enable_spectrum_ramp = 0;
		generate_spectrum_transfer_function(Number_of_colours_in_spectrum());
		updateTFWidgetFromOpacityArrays();
	}

	void generate_spectrum_ramp_transfer_function_and_check_menu_item()
	{
		ui->action_Spectrum_Ramp_Transfer_Function->setChecked(true);
		ui->action_Spectrum_Transfer_Function->setChecked(false);
		enable_spectrum_ramp = 1;
		generate_spectrum_ramp_transfer_function(Number_of_colours_in_spectrum());
		updateTFWidgetFromOpacityArrays();
	}

	void draw_spectrum_in_graphicsview()
	{
		auto tf = scalar_color;
		int n = tf->GetSize();
		const double width = 340;
		const double height = 16;
		double w = width / n;
		QGraphicsScene *scene = get_GraphicsScene_for_spectrum();
		scene->clear();
		QColor colour(Qt::yellow);
		int h, s, v;
		colour.getHsv(&h, &s, &v);
		std::cout << "hsv colour " << h << " " << s << " " << v << std::endl;
		int r, g, b;
		colour.getRgb(&r, &g, &b);
		std::cout << "rgb colour " << r << " " << g << " " << b << std::endl;

		for (int i = 0; i < n; i++)
		{
			double xrgb[6];
			tf->GetNodeValue(i, xrgb);
			QColor colour;
			colour.setRgb(denormalise_rgba(xrgb[1]), denormalise_rgba(xrgb[2]), denormalise_rgba(xrgb[3]));
			QBrush brush(colour);
			QPen pen;
			pen.setStyle(Qt::NoPen);
			auto rect = scene->addRect(w * i, 0, w, height, pen, brush);
			rect->setFlag(QGraphicsItem::ItemIsSelectable);
			rect->setData(0, i);
		}
	}

	void compute_distance_and_weights_by_colours(int r, int g, int b)
	{
		// put the picked colour in the array for distance computation
		unsigned char pixels[] = { r, g, b };

		// compute region weights based on the selected image
		control_point_weight_list.clear();
		double sum = 0;
		for (int i = 0; i < colour_list_size(); i++)
		{
			double r = get_colour_r(i);
			double g = get_colour_g(i);
			double b = get_colour_b(i);

			// compute distance in hue without squaring
			double distance = get_distance_between_colour_and_pixels_selector(r, g, b, pixels, 1, 3, 0, 1);
			control_point_weight_list.push_back(distance);
			sum += distance;
		}

		if (sum > 0)
		{
			// normalize the distances
			for (unsigned int i = 0; i < control_point_weight_list.size(); i++)
			{
				control_point_weight_list[i] = control_point_weight_list[i] / sum;
			}
		}
	}

	void compute_distance_and_weights_by_intensity(double normalized_intensity)
	{
		control_point_weight_list.clear();
		double sum = 0;

		for (int i = 0; i < intensity_list_size(); i++)
		{
			double distance = abs(get_intensity(i) - normalized_intensity);
			control_point_weight_list.push_back(distance);
			sum += distance;
		}

		if (sum > 0)
		{
			// normalize the distances
			for (unsigned int i = 0; i < control_point_weight_list.size(); i++)
			{
				control_point_weight_list[i] = control_point_weight_list[i] / sum;
			}
		}
	}

	void optimise_transfer_function_for_colour(QColor colour)
	{
		colour_for_optimization = colour;

		// compute the distance between control point colour and selected colour
		compute_distance_and_weights_by_colours(colour.red(), colour.green(), colour.blue());

		std::cout << "picked colour (RGB) " << colour.red() << " " << colour.green() << " " << colour.blue() << std::endl;
		std::cout << "picked colour (HSV) " << colour.hue() << " " << colour.saturation() << " " << colour.value() << std::endl;

		// optimise the transfer function for the selected colour
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
	}

	/// 
	void optimise_transfer_function_for_intensity(double normalized_intensity)
	{
		compute_distance_and_weights_by_intensity(normalized_intensity);

		// optimise the transfer function for the selected colour
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
	}

	void reset_transfer_function()
	{
		// generate a spectrum transfer function with n groups of control points
		if (enable_spectrum_ramp == 0)
		{
			generate_spectrum_transfer_function(Number_of_colours_in_spectrum());
		}
		else
		{
			generate_spectrum_ramp_transfer_function(Number_of_colours_in_spectrum());
		}
	}

	void reload_transfer_function_from_file()
	{
		// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
		QByteArray ba = transfer_function_filename.toLocal8Bit();
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
	}

	void populateRenderingTechniqueComboBox()
	{
		RenderingTechniqueComboBox()->clear();
		RenderingTechniqueComboBox()->addItem("Composite With Shading", vtkMRMLVolumeRenderingDisplayNode::Composite);
		RenderingTechniqueComboBox()->addItem("CompositeEdgeColoring", vtkMRMLVolumeRenderingDisplayNode::CompositeEdgeColoring);
		RenderingTechniqueComboBox()->addItem("Maximum Intensity Projection", vtkMRMLVolumeRenderingDisplayNode::MaximumIntensityProjection);
		RenderingTechniqueComboBox()->addItem("Minimum Intensity Projection", vtkMRMLVolumeRenderingDisplayNode::MinimumIntensityProjection);
		RenderingTechniqueComboBox()->addItem("GradiantMagnitudeOpacityModulation", vtkMRMLVolumeRenderingDisplayNode::GradiantMagnitudeOpacityModulation);
		RenderingTechniqueComboBox()->addItem("IllustrativeContextPreservingExploration", vtkMRMLVolumeRenderingDisplayNode::IllustrativeContextPreservingExploration);
	}

	void gaussian(int n = 5, double sigma=1)
	{
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

	private slots:

	void slot_GraphicsScene_selectionChanged()
	{
		auto tf = scalar_color;
		auto scene = get_GraphicsScene_for_spectrum();
		auto list = scene->items();
		for (int i = 0; i < list.size(); i++)
		{
			bool selected = list.at(i)->isSelected();
			if (selected)
			{
				int index = list.at(i)->data(0).toInt();
				std::cout << "QGraphicsItem index=" << index << std::endl;
				if (index >= 0 && index < tf->GetSize())
				{
					double xrgb[6];
					tf->GetNodeValue(index, xrgb);
					double intensity = normalise_intensity(xrgb[0]);
					double min_distance = UCHAR_MAX;
					int min_index = -1;
					for (int i = 0; i < intensity_list_size(); i++)
					{
						double distance = abs(intensity - get_intensity(i));
						if (distance < min_distance)
						{
							min_distance = distance;
							min_index = i;
						}
					}
					if (min_index != -1)
					{
						std::cout << "closest control point index=" << min_index << " intensity=" << get_intensity(min_index) << std::endl;
						if (ui->radioButton_optimise->isChecked())
						{
							// reset transfer function before optimizing it
							//reset_transfer_function();

							optimise_transfer_function_for_intensity(get_intensity(min_index));
						}
						else
						{
							// set opacity of the closest control point to zero
							set_opacity(min_index, 0);
							updateTFWidgetFromOpacityArrays();
							updateOpacityArrayFromTFWidget();
						}
					}
					else
					{
						std::cout << "Errors in slot_GraphicsScene_selectionChanged(). min_index=" << min_index << std::endl;
					}
				}
			}
		}
	}

	void slot_GraphicsScene_sceneRectChanged(const QRectF & rect)
	{
		std::cout << "slot_GraphicsScene_sceneRectChanged " << "width=" << rect.width() << " height=" << rect.height() << std::endl;
	}

	void slot_region_selected(QString filename)
	{
		// get local 8-bit representation of the string in locale encoding (in case the filename contains non-ASCII characters) 
		QByteArray ba = selected_region_filename.toLocal8Bit();
		const char *filename_str = ba.data();
		std::cout << "slot_region_selected " << filename_str << std::endl;

		if (!filename.trimmed().isEmpty())
		{
			selected_region_filename = filename;
		}
		else
		{
			return;
		}

		read_region_image_and_compute_distance(1);
	}

	void on_entropyButton_clicked();
	void on_frequencyButton_clicked();
	void on_opacityButton_clicked();
	void on_visibilityButton_clicked();
	void on_entropyOpacityButton_clicked();
	void on_balanceButton_clicked();
	void on_reduceOpacityButton_clicked();
	void on_lhHistogramButton_clicked();
	void on_balanceEdgeButton_clicked();
	void on_increaseOpacityButton_clicked();
	void on_enhanceRegionButton_clicked();
	void on_weakenRegionButton_clicked();
	void on_balanceRegionButton_clicked();
	void on_action_About_triggered();
	void on_action_Exit_triggered();
	void on_action_Open_Volume_triggered();
	void on_action_Append_Volume_triggered();
	void on_action_Open_Transfer_Function_triggered();
	void on_action_Save_Transfer_Function_triggered();
	void on_action_Open_Selected_Region_triggered();
	void on_action_Compute_Squared_Distance_triggered();
	void on_action_Compute_Distance_triggered();
	void on_action_Compute_Squared_Distance_HSV_triggered();
	void on_action_Compute_Distance_HSV_triggered();
	void on_action_Default_Transfer_Function_triggered();
	void on_action_Spectrum_Transfer_Function_triggered();
	void on_action_Spectrum_Ramp_Transfer_Function_triggered();
	void on_action_Pick_a_colour_and_optimise_transfer_function_triggered();
	void on_action_Genearte_transfer_functions_for_spectrum_triggered();
	void on_action_Open_path_and_generate_transfer_functions_triggered();
	void on_action_Open_path_and_generate_transfer_functions_for_region_triggered();
	void on_action_Open_path_and_generate_transfer_functions_for_colour_triggered();
	void on_resetButton_clicked();
	void on_action_Test_triggered();
	void on_drawWeightButton_clicked();
	void on_action_VtkSmartVolumeMapper_triggered();
	void on_action_VtkSlicerGPURayCastVolumeMapper_triggered();
	void on_action_VtkSlicerGPURayCastMultiVolumeMapper_triggered();
	void on_reloadButton_clicked();
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();
	void on_pushButton_3_clicked();
	void on_pushButton_4_clicked();
	void on_pushButton_5_clicked();
	void on_pushButton_6_clicked();
	void on_action_Screenshot_triggered();
	void on_action_Auto_open_selected_region_triggered();
	void on_checkBox_clicked();
	void on_comboBox_currentIndexChanged(int index);
	void on_newtonButton_clicked();
	void on_gradientDescentButton_clicked();
	void on_fixedStepButton_clicked();
    void on_listView_clicked(const QModelIndex &index);
    void on_listView_2_clicked(const QModelIndex &index);
    void on_listView_2_doubleClicked(const QModelIndex &index);
    void on_listView_doubleClicked(const QModelIndex &index);
    void on_listView_activated(const QModelIndex &index);
    void on_listView_2_activated(const QModelIndex &index);
    void on_action_Reset_Renderer_triggered();
    void on_action_Remove_Renderer_triggered();
    void on_action_Open_dynamically_optimized_vortex_triggered();
    void on_action_Open_vortex_triggered();
    void on_action_Open_statically_optimized_vortex_triggered();
};

#endif // MAINWINDOW_H
