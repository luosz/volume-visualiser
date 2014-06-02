#include <vtkVersion.h>
#include <vtkSmartPointer.h>

#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#include <vtkActor.h>
#include <vtkImageAccumulate.h>
#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>
#include <vtkJPEGReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkXYPlotActor.h>
#include <vtkPNGReader.h>
#include <vtkMetaImageReader.h>
#include <vtkNrrdReader.h>

int main(int argc, char *argv[])
{
	// Handle the arguments
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " Filename.jpg, [optional ignore zero:] <y/n>" << std::endl;
		return EXIT_FAILURE;
	}

	int ignoreZero = 0;
	if (argc == 3)
	{
		std::string ignore = argv[2];
		std::cout << ignore << std::endl;
		if (ignore == "y" || ignore == "Y")
		{
			ignoreZero = 1;
		}
	}

	//// Read a jpeg image
	//vtkSmartPointer<vtkJPEGReader> reader =
	//	vtkSmartPointer<vtkJPEGReader>::New();

	char *filename_str = argv[1];
	vtkSmartPointer<vtkImageReader2> reader;
	if (strstr(filename_str, ".jpg"))
	{
		reader = vtkSmartPointer<vtkJPEGReader>::New();
	}
	if (strstr(filename_str, ".png"))
	{
		reader = vtkSmartPointer<vtkPNGReader>::New();
	}
	if (strstr(filename_str, ".mhd"))
	{
		reader = vtkSmartPointer<vtkMetaImageReader>::New();
	}
	if (strstr(filename_str, ".nrrd"))
	{
		reader = vtkSmartPointer<vtkNrrdReader>::New();
	}
	if (reader == NULL)
	{
		std::cout << filename_str << " has unknown filename extension. Only .jpg .png .mhd and .nrrd are supported." << std::endl;
		return EXIT_FAILURE;
	}

	if (!reader->CanReadFile(argv[1]))
	{
		std::cout << "Error: cannot read " << argv[1] << std::endl;
		return EXIT_FAILURE;
	}
	reader->SetFileName(argv[1]);
	reader->Update();

	int numComponents = reader->GetOutput()->GetNumberOfScalarComponents();
	if (numComponents > 3)
	{
		std::cout << "Error: cannot process an image with "
			<< numComponents << " components!" << std::endl;
		return EXIT_FAILURE;
	}

	// Create a vtkXYPlotActor
	vtkSmartPointer<vtkXYPlotActor> plot =
		vtkSmartPointer<vtkXYPlotActor>::New();
	plot->ExchangeAxesOff();
	plot->SetLabelFormat("%g");
	plot->SetXTitle("Level");
	plot->SetYTitle("Frequency");
	plot->SetXValuesToValue();

	double xmax = 0.;
	double ymax = 0.;

	double colors[3][3] = {
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ 0, 0, 1 } };

	const char* labels[3] = {
		"Red", "Green", "Blue" };

	// Process the image, extracting and plotting a histogram for each
	// component
	for (int i = 0; i < numComponents; ++i)
	{
		vtkSmartPointer<vtkImageExtractComponents> extract =
			vtkSmartPointer<vtkImageExtractComponents>::New();
		extract->SetInputConnection(reader->GetOutputPort());
		extract->SetComponents(i);
		extract->Update();

		double range[2];
		extract->GetOutput()->GetScalarRange(range);

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

		if (range[1] > xmax)
		{
			xmax = range[1];
		}
		if (histogram->GetOutput()->GetScalarRange()[1] > ymax)
		{
			ymax = histogram->GetOutput()->GetScalarRange()[1];
		}

		std::cout << "range " << range[0] << " " << range[1] << std::endl;
		int extents[6];
		histogram->GetComponentExtent(extents);
		std::cout << "extent " << extents[0] << " " << extents[1] << std::endl;
		auto histogram_data = static_cast<int *>(histogram->GetOutput()->GetScalarPointer());
		char filename[MAX_PATH];
		sprintf(filename, "../~%i.txt", i);
		ofstream f(filename);
		for (int j = extents[0]; j < extents[1]; j++)
		{
			f << j<<"\t"<<histogram_data[j] << std::endl;
			//std::cout << histogram_data[j] << "\t";
		}
		std::cout << endl;
		
#if VTK_MAJOR_VERSION <= 5
		plot->AddInput(histogram->GetOutput());
#else
		plot->AddDataSetInputConnection(histogram->GetOutputPort());
#endif
		//if (numComponents > 1)
		{
			plot->SetPlotColor(i, colors[i]);
			plot->SetPlotLabel(i, labels[i]);
			plot->LegendOn();
		}
	}

	plot->SetXRange(0, xmax);
	plot->SetYRange(0, ymax);

	// Visualize the histogram(s)
	vtkSmartPointer<vtkRenderer> renderer =
		vtkSmartPointer<vtkRenderer>::New();
	renderer->AddActor(plot);

	vtkSmartPointer<vtkRenderWindow> renderWindow =
		vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	renderWindow->SetSize(640, 480);

	vtkSmartPointer<vtkRenderWindowInteractor> interactor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	interactor->SetRenderWindow(renderWindow);

	// Initialize the event loop and then start it
	interactor->Initialize();
	interactor->Start();

	return EXIT_SUCCESS;
}
