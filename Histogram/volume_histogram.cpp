/// http://www.itk.org/Wiki/VTK/Examples/Cxx/Images/ImageAccumulateGreyscale

#include <vtkActor.h>
#include <vtkBarChartActor.h>
#include <vtkFieldData.h>
#include <vtkImageAccumulate.h>
#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>
#include <vtkIntArray.h>
#include <vtkJPEGReader.h>
#include <vtkLegendBoxActor.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkImageMagnitude.h>
#include <vtkPNGReader.h>
#include <vtkMetaImageReader.h>

int main( int argc, char *argv[] )
{
  // Handle the arguments
  if( argc < 2 )
    {
    std::cout << "Required arguments: filename.png" << std::endl;
    return EXIT_FAILURE;
    }
 
  //// Read a jpeg image
  //vtkSmartPointer<vtkPNGReader> reader = 
  //  vtkSmartPointer<vtkPNGReader>::New();

  // Read a jpeg image
  vtkSmartPointer<vtkImageReader2> reader;
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

  reader->SetFileName(filename_str);
  reader->Update();
 
  vtkSmartPointer<vtkImageMagnitude> magnitude =
    vtkSmartPointer<vtkImageMagnitude>::New();
  magnitude->SetInputConnection(reader->GetOutputPort());
  magnitude->Update();
  
  double red[3] = {1, 0, 0 };
 
  vtkSmartPointer<vtkIntArray> frequencies = 
    vtkSmartPointer<vtkIntArray>::New();
 
  vtkSmartPointer<vtkImageAccumulate> histogram = 
    vtkSmartPointer<vtkImageAccumulate>::New();
  histogram->SetInputConnection(magnitude->GetOutputPort());
  histogram->SetComponentExtent(0,255,0,0,0,0);
  histogram->SetComponentOrigin(0,0,0);
  histogram->SetComponentSpacing(1,0,0);
  histogram->IgnoreZeroOn();
  histogram->Update();

  frequencies->SetNumberOfComponents(1);
  frequencies->SetNumberOfTuples(256);
  int* output = static_cast<int*>(histogram->GetOutput()->GetScalarPointer());

  for(int j = 0; j < 256; ++j)
    {
    frequencies->SetTuple1(j, *output++);
    }
 
  vtkSmartPointer<vtkDataObject> dataObject = 
    vtkSmartPointer<vtkDataObject>::New();
 
  dataObject->GetFieldData()->AddArray( frequencies );
 
  // Create a vtkBarChartActor
  vtkSmartPointer<vtkBarChartActor> barChart = 
    vtkSmartPointer<vtkBarChartActor>::New();
 
  barChart->SetInput(dataObject);
  barChart->SetTitle("Histogram");
  barChart->GetPositionCoordinate()->SetValue(0.05,0.05,0.0);
  barChart->GetPosition2Coordinate()->SetValue(0.95,0.85,0.0);
  barChart->GetProperty()->SetColor(1,1,1);
 
  barChart->GetLegendActor()->SetNumberOfEntries(dataObject->GetFieldData()->GetArray(0)->GetNumberOfTuples());
  barChart->LegendVisibilityOff();
  barChart->LabelVisibilityOff();
 
  int count = 0;
  for(int i = 0; i < 256; ++i)
    {
    barChart->SetBarColor( count++, red );
    }
 
  // Visualize the histogram
  vtkSmartPointer<vtkRenderer> renderer = 
    vtkSmartPointer<vtkRenderer>::New();
  renderer->AddActor(barChart);
 
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
 
  return  EXIT_SUCCESS;
}
