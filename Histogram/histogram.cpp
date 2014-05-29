#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <cstring>
#include <cstdlib>
using namespace std;

#include <vtkSmartPointer.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVolumeProperty.h>
#include <vtkMatrix4x4.h>
#include <vtkAxesActor.h>
#include <vtkMetaImageReader.h>
#include <vtkImageShiftScale.h>
#include <vtkImageData.h>
#include <vtkHAVSVolumeMapper.h>
#include <vtkVolume16Reader.h>
#include <vtkNrrdReader.h>
#include <vtkNew.h>
#include <vtkVersion.h>

#include <vtkImageReader2.h>
#include <vtkImageAccumulate.h>
#include <vtkIntArray.h>
#include <vtkBarChartActor.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkProperty2D.h>
#include <vtkLegendBoxActor.h>
#include <vtkPNGReader.h>
#include <vtkXYPlotActor.h>
#include <vtkImageExtractComponents.h>

#define OUTPUT_TO_FILE

//void generateHistogram(vtkSmartPointer<vtkImageReader2> reader)
//{
//	int bins = 256;
//	vtkSmartPointer<vtkImageAccumulate> histogram = vtkSmartPointer<vtkImageAccumulate>::New();
//	histogram->SetInputConnection(reader->GetOutputPort());
//	histogram->SetComponentExtent(0, bins-1, 0, 0, 0, 0);
//	histogram->SetComponentOrigin(0, 0, 0);
//	histogram->SetComponentSpacing(256/bins, 0, 0);
//	histogram->Update();
//	vtkSmartPointer<vtkIntArray> frequencies = vtkSmartPointer<vtkIntArray>::New();
//	frequencies->SetNumberOfComponents(1);
//	frequencies->SetNumberOfTuples(bins);
//	int * output = static_cast<int *>(histogram->GetOutput()->GetScalarPointer());
//
//	for(int j = 0; j < bins; ++j)
//	{
//		frequencies->SetTuple1(j, *output++);
//	}
//
//	//histogram->GetOutput()->GetPointData()->GetScalars()
//
//	vtkSmartPointer<vtkDataObject> dataObject = vtkSmartPointer<vtkDataObject>::New();
//	dataObject->GetFieldData()->AddArray( frequencies );
//	vtkSmartPointer<vtkBarChartActor> barChart = vtkSmartPointer<vtkBarChartActor>::New();
//
//	barChart->SetInput(dataObject);
//	barChart->SetTitle("Histogram");
//	barChart->GetPositionCoordinate()->SetValue(0.05,0.05,0.0);
//	barChart->GetPosition2Coordinate()->SetValue(0.95,0.95,0.0);
//	barChart->GetProperty()->SetColor(1,1,1);
//	barChart->GetLegendActor()->SetNumberOfEntries(dataObject->GetFieldData()->GetArray(0)->GetNumberOfTuples());
//	barChart->LegendVisibilityOff();
//	barChart->LabelVisibilityOff();
//
//	double red[3] = {1, 0, 0 };
//	for(int i = 0; i < bins; ++i)
//	{
//		barChart->SetBarColor(i, red );
//	}
//
//	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
//	renderer->AddActor(barChart);
//	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
//	//auto renderWindow = histogramWidget.GetRenderWindow();
//	renderWindow->AddRenderer(renderer);
//	renderWindow->SetSize(640, 480);
//	vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
//	interactor->SetRenderWindow(renderWindow);
//	//interactor->SetInteractorStyle(vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New());
//	renderWindow->Render();
//	interactor->Initialize();
//	interactor->Start();
//}

/// Re-maps a number from one range to another.
double map_to_range(double n, double lower, double upper, double target_lower, double target_upper)
{
	n = n < lower ? lower : n;
	n = n > upper ? upper : n;
	double normalised = (n - lower) / (upper - lower);
	return normalised * (target_upper - target_lower) + target_lower;
}

double normalise_rgba(int n)
{
	return map_to_range(n, 0, 255, 0, 1);
}

double range_x = 0, range_y = 255;

double normalise_intensity(double n)
{
	return map_to_range(n, 0, 255, 0, 1);
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

int main(int argc, char *argv[])
{
	//// This is the data that will be volume rendered.
	//std::string volumeFilename("../../data/nucleon.mhd");
	//if(argc >= 2)
	//{
	//	volumeFilename = argv[1];
	//}
	//std::cout<<"volume file: "<<volumeFilename<<endl;

	//// to read a .mhd file or a folder containing a set of raw files
	//vtkSmartPointer<vtkMetaImageReader> reader = vtkMetaImageReader::New();
	//reader->SetFileName(volumeFilename.c_str());

	//generateHistogram(reader);

	if( argc < 2 )
	{
		std::cout << "Usage: " << argv[0] << " Filename.png, [optional ignore zero:] <y/n>" << std::endl;
		return EXIT_FAILURE;
	}

	int ignoreZero = 0;

	// Read a jpeg image
	//auto reader = vtkSmartPointer<vtkMetaImageReader>::New();
	vtkSmartPointer<vtkImageReader2> reader;
	//auto reader = vtkSmartPointer<vtkPNGReader>::New();
	char *filename_str = argv[1];
	auto p = strstr(filename_str, ".png");
	if (p)
	{
		reader = vtkSmartPointer<vtkPNGReader>::New();
	} 
	else
	{
		p = strstr(filename_str, ".mhd");
		if (p)
		{
			reader = vtkSmartPointer<vtkMetaImageReader>::New();
		} 
		else
		{
			std::cout << filename_str << " has unknown filename extension. Only .png and .mhd are supported." << std::endl;
			return EXIT_FAILURE;
		}
	}
	if (!reader->CanReadFile(filename_str))
	{
		std::cout << "Error: cannot read " << filename_str << std::endl;
		return EXIT_FAILURE;
	}
	reader->SetFileName(filename_str);
	reader->Update();

	int numComponents = reader->GetOutput()->GetNumberOfScalarComponents();
	std::cout<<"component number="<<numComponents<<endl;
	if( numComponents > 3 )
	{
		std::cout << "Error: cannot process an image with " 
			<< numComponents << " components!" << std::endl;
		return EXIT_FAILURE;
	}

	//////////////////////////////////////////////////////////////////////////
	auto imageData = reader->GetOutput();
	int dimensions[3];
	imageData->GetDimensions(dimensions);
	int count_of_pixels = dimensions[0] * dimensions[1] * dimensions[2];
	std::cout<<"dimension "<<dimensions[0]<<" "<<dimensions[1]<<" "<<dimensions[2]<<" count="<<count_of_pixels<<std::endl;
	std::cout<<"pixel type is "<<imageData->GetScalarTypeAsString()<<std::endl;
	auto pixels = static_cast<unsigned char *>(imageData->GetScalarPointer());

	{
		double range[2];
		imageData->GetScalarRange(range);
		range_x = range[0];
		range_y = range[1];
		std::cout << "GetScalarRange "<<range_x<<" "<<range_y << std::endl;
	}

	int rgb_histogram[256][3];
	std::memset(rgb_histogram, 0, 256*3*sizeof(int));

	for (int i=0; i<count_of_pixels; i++)
	{
		for (int j=0; j<numComponents; j++)
		{
			rgb_histogram[pixels[i*numComponents+j]][j]++;
		}
	}

	double r, g, b;
	r = g = b = 0;
	double distance = get_distance_between_colour_and_pixels(r, g, b, pixels, count_of_pixels, numComponents);
	double d1 = distance;
	std::cout<<"distance to ("<<r<<","<<g<<","<<b<<")="<<distance<<std::endl;

	r = 0.909803;
	g = 0.517647;
	b = 0.517647;
	distance = get_distance_between_colour_and_pixels(r, g, b, pixels, count_of_pixels, numComponents);
	double d2 = distance;
	std::cout<<"distance to ("<<r<<","<<g<<","<<b<<")="<<distance<<std::endl;

	r = g = b = 1;
	distance = get_distance_between_colour_and_pixels(r, g, b, pixels, count_of_pixels, numComponents);
	double d3 = distance;
	std::cout<<"distance to ("<<r<<","<<g<<","<<b<<")="<<distance<<std::endl;
	std::cout<<d1/(d1+d2+d3)<<" "<<d2/(d1+d2+d3)<<" "<<d3/(d1+d2+d3)<<std::endl;

#ifdef OUTPUT_TO_FILE
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
	//////////////////////////////////////////////////////////////////////////

	// Create a vtkXYPlotActor
	vtkSmartPointer<vtkXYPlotActor> plot = 
		vtkSmartPointer<vtkXYPlotActor>::New();
	plot->ExchangeAxesOff();
	plot->SetLabelFormat( "%g" );
	plot->SetXTitle( "Level" );
	plot->SetYTitle( "Frequency" );
	plot->SetXValuesToValue();

	double xmax = 0.;
	double ymax = 0.;

	double colors[3][3] = {
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ 0, 0, 1 } };

	const char* labels[3] = {
		"Red", "Green", "Blue" };

	vtkSmartPointer<vtkIntArray> redFrequencies = 
		vtkSmartPointer<vtkIntArray>::New();
	vtkSmartPointer<vtkIntArray> greenFrequencies = 
		vtkSmartPointer<vtkIntArray>::New();
	vtkSmartPointer<vtkIntArray> blueFrequencies = 
		vtkSmartPointer<vtkIntArray>::New();

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
			static_cast<int>(range[1])-static_cast<int>(range[0]),0,0,0,0 );
		histogram->SetComponentOrigin( range[0],0,0 );
		histogram->SetComponentSpacing( 1,0,0 );
		histogram->SetIgnoreZero( ignoreZero );
		histogram->Update();
		std::cout << "histogram->SetComponentExtent=" << static_cast<int>(range[1]) - static_cast<int>(range[0]) - 1 << std::endl;

		vtkIntArray* currentArray = 0;
		if( i == 0 )
		{
			currentArray = redFrequencies;
		}
		else if( i == 1 )
		{
			currentArray = greenFrequencies;
		}
		else
		{
			currentArray = blueFrequencies;
		}

		currentArray->SetNumberOfComponents(1);
		currentArray->SetNumberOfTuples(256);
		int* output = static_cast<int*>(histogram->GetOutput()->GetScalarPointer());

		for( int j = 0; j < 256; ++j )
		{
			currentArray->SetTuple1( j, *output++ );
		}

		if( range[1] > xmax ) 
		{ 
			xmax = range[1];
		}
		if( histogram->GetOutput()->GetScalarRange()[1] > ymax ) 
		{
			ymax = histogram->GetOutput()->GetScalarRange()[1];
		}
		std::cout<<"histogram range "<<histogram->GetOutput()->GetScalarRange()[0]<<" "<<histogram->GetOutput()->GetScalarRange()[1]<<endl;

#if VTK_MAJOR_VERSION <= 5
		plot->AddInput( histogram->GetOutput() );
#else
		plot->AddDataSetInputConnection( histogram->GetOutputPort() );
#endif
		if( numComponents > 1 )
		{
			plot->SetPlotColor(i,colors[i]);
			plot->SetPlotLabel(i,labels[i]);
			plot->LegendOn();
		}

		//if (i == 0)
		{
			double min = histogram->GetOutput()->GetScalarRange()[0];
			double max = histogram->GetOutput()->GetScalarRange()[1];
			std::cout<<"min="<<min<<" max="<<max<<endl;
			char buffer[32];
			itoa(i, buffer, 10);
			char filename[32];
			sprintf(filename, "../%s.csv", buffer);
			std::cout<<"output file "<<filename<<std::endl;
			ofstream myfile;
			myfile.open(filename);
			int* pixels = static_cast<int*>(histogram->GetOutput()->GetScalarPointer());
			for (int j=0; j<256; j++)
			{
				int value = pixels[j];
				if (value < min || value > max)
				{
					value = 0;
				}
				myfile<<j<<", "<<value<<std::endl;
			}
			myfile.close();
		}
	}

	plot->SetXRange( 0, xmax );
	plot->SetYRange( 0, ymax );

	// Visualize the histogram(s)
	vtkSmartPointer<vtkRenderer> renderer = 
		vtkSmartPointer<vtkRenderer>::New();
	renderer->AddActor(plot);

	vtkSmartPointer<vtkRenderWindow> renderWindow = 
		vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer( renderer );
	renderWindow->SetSize(640, 480);

	vtkSmartPointer<vtkRenderWindowInteractor> interactor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	interactor->SetRenderWindow( renderWindow );

	// Initialize the event loop and then start it
	interactor->Initialize();
	interactor->Start(); 

	return EXIT_SUCCESS;
}
