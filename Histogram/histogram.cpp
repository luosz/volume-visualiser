#include <iostream>
#include <string>
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

void generateHistogram(vtkSmartPointer<vtkImageReader2> reader)
{
	int bins = 256;
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

	//histogram->GetOutput()->GetPointData()->GetScalars()

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

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		std::cerr << "Required arguments: volumeVTKFile" << std::endl;
		return EXIT_FAILURE;
	}

	// This is the data that will be volume rendered.
	std::string volumeFilename = argv[1]; // "/Data/ironProt.vtk";

	// to read a .mhd file or a folder containing a set of raw files
	vtkSmartPointer<vtkMetaImageReader> reader = vtkMetaImageReader::New();
	reader->SetFileName(volumeFilename.c_str());

	generateHistogram(reader);

	return EXIT_SUCCESS;
}
