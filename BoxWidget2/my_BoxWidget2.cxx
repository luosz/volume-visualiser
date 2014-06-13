/// http://public.kitware.com/pipermail/vtkusers/2013-August/081387.html

#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkBoxWidget2.h>
#include <vtkBoxRepresentation.h>
#include <vtkCommand.h>
#include <vtkAbstractVolumeMapper.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkMetaImageReader.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageMandelbrotSource.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkImageData.h>

// This does the actual work.
// Callback for the interaction
class vtkBoxCallback : public vtkCommand
{
  public:
    static vtkBoxCallback *New()
    {
      return new vtkBoxCallback;
    }

	virtual void SetVolume(vtkVolume * vol)
	{
		volumen = vol;
	}

	void SetInteractor(vtkRenderWindowInteractor *interactor)
	{
		this->Interactor = interactor;
	}
    
    virtual void Execute(vtkObject *caller, unsigned long, void*)
    {
      
      vtkBoxWidget2 *boxWidget = 
        reinterpret_cast<vtkBoxWidget2*>(caller);
      
      // Get the actual box coordinates/planes
      vtkSmartPointer<vtkPolyData> polydata = 
        vtkSmartPointer<vtkPolyData>::New();
      static_cast<vtkBoxRepresentation*>(boxWidget->GetRepresentation())->GetPolyData (polydata);
      
      // Display the center of the box
      double p[3];
      polydata->GetPoint(14,p); // As per the vtkBoxRepresentation documentation, the 15th point (index 14) is the center of the box
      std::cout << "Box center: " << p[0] << " " << p[1] << " " << p[2] << std::endl;



	  //vtkBoxWidget2 *boxWidget = reinterpret_cast<vtkBoxWidget2*>(caller);
	  //vtkBoxRepresentation * rep =
		 // vtkBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation());
	  //rep->GetPlanes(clippingPlanes);
	  //volumen->GetMapper()->SetClippingPlanes(clippingPlanes);
	  ////        Interactor->Render();

	  ////        volumen->Update();
    }
    vtkBoxCallback(){}
    
private:
	vtkVolume * volumen;
	vtkPlanes * clippingPlanes;
	vtkRenderWindowInteractor *Interactor;
};
 
int main(int, char *[])
{
	// Clipping box widget
	//auto boxWidget = vtkBoxWidget2::New();
	//boxWidget->SetInteractor(this->ui->qvtkWidget->GetInteractor());
	//auto boxCallback = vtkBoxCallback::New();
	//boxCallback->SetVolume(this->volumen[0]);
	////    boxCallback->SetInteractor(this->ui->qvtkWidget->GetInteractor());
	//boxWidget->AddObserver(vtkCommand::InteractionEvent, boxCallback);
	//auto boxRepresentation = vtkSmartPointer<vtkBoxRepresentation>::New();
	//boxWidget->SetRepresentation(boxRepresentation);

	//boxWidget->On();

  vtkSmartPointer<vtkSphereSource> sphereSource = 
    vtkSmartPointer<vtkSphereSource>::New();
  sphereSource->Update();

  // Create a mapper and actor
  vtkSmartPointer<vtkPolyDataMapper> mapper = 
    vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(sphereSource->GetOutputPort());
  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);
  
  // A renderer and render window
  vtkSmartPointer<vtkRenderer> renderer = 
    vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renderWindow = 
    vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  //renderer->AddActor(actor);
  
  // An interactor
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = 
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);

  // allow the user to interactively manipulate (rotate, pan, etc.) the camera, the viewpoint of the scene.
  auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
  renderWindowInteractor->SetInteractorStyle(style);
  renderWindowInteractor->SetStillUpdateRate(1);

  // Create transfer mapping scalar value to opacity.
  auto scalar_opacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
  scalar_opacity->AddPoint(0.0, 0.0);
  scalar_opacity->AddPoint(36.0, 0.125);
  scalar_opacity->AddPoint(72.0, 0.25);
  scalar_opacity->AddPoint(108.0, 0.375);
  scalar_opacity->AddPoint(144.0, 0.5);
  scalar_opacity->AddPoint(180.0, 0.625);
  scalar_opacity->AddPoint(216.0, 0.75);
  scalar_opacity->AddPoint(255.0, 0.0);

  // Create transfer mapping scalar value to color.
  auto scalar_color = vtkSmartPointer<vtkColorTransferFunction>::New();
  scalar_color->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
  scalar_color->AddRGBPoint(36.0, 1.0, 0.0, 0.0);
  scalar_color->AddRGBPoint(72.0, 1.0, 1.0, 0.0);
  scalar_color->AddRGBPoint(108.0, 0.0, 1.0, 0.0);
  scalar_color->AddRGBPoint(144.0, 0.0, 1.0, 1.0);
  scalar_color->AddRGBPoint(180.0, 0.0, 0.0, 1.0);
  scalar_color->AddRGBPoint(216.0, 1.0, 0.0, 1.0);
  scalar_color->AddRGBPoint(255.0, 1.0, 1.0, 1.0);

  char *filename_str = "../../data/CT-Knee.mhd";
  auto reader = vtkSmartPointer<vtkMetaImageReader>::New();
  if (!reader->CanReadFile(filename_str))
  {
	  std::cerr << "Reader reports " << filename_str << " cannot be read.";
	  exit(EXIT_FAILURE);
  }
  reader->SetFileName(filename_str);

  // set up volume property
  auto volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
  volumeProperty->SetScalarOpacity(scalar_opacity);
  //volumeProperty->SetGradientOpacity(gradient_opacity);
  volumeProperty->SetColor(scalar_color);
  //volumeProperty->ShadeOff();
  //volumeProperty->SetInterpolationTypeToLinear();

  auto volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
  volumeMapper->SetRequestedRenderMode(vtkSmartVolumeMapper::GPURenderMode);

  //vtkSmartPointer<vtkImageMandelbrotSource> source =
	 // vtkSmartPointer<vtkImageMandelbrotSource>::New();
  //source->Update();

  int extent[6];
  reader->GetOutput()->GetExtent(extent);
  std::cout << "extent " << extent[0] << " " << extent[1] << " " << extent[2] << " " << extent[3] << " " << extent[4] << " " << extent[5] << std::endl;
  int dim[3];
  reader->GetOutput()->GetDimensions(dim);
  std::cout << "dimension " << dim[0] << " " << dim[1] << " " << dim[2] << std::endl;

  volumeMapper->SetInputConnection(reader->GetOutputPort());

  auto volume = vtkSmartPointer<vtkVolume>::New();
  volume->SetMapper(volumeMapper);
  volume->SetProperty(volumeProperty);
  
  vtkSmartPointer<vtkBoxCallback> boxCallback = 
    vtkSmartPointer<vtkBoxCallback>::New();
  //boxCallback->SphereSource = sphereSource;

  boxCallback->SetVolume(volume);
  renderer->AddVolume(volume);

  vtkSmartPointer<vtkBoxWidget2> boxWidget =
	  vtkSmartPointer<vtkBoxWidget2>::New();
  boxWidget->SetInteractor(renderWindowInteractor);
  //boxWidget->CreateDefaultRepresentation();

  vtkSmartPointer<vtkBoxRepresentation> boxRepresentation =
	  vtkSmartPointer<vtkBoxRepresentation>::New();
  boxWidget->SetRepresentation(boxRepresentation);

  double bounds[] = { 0, 379, 0, 229, 0, 305 };
  boxRepresentation->PlaceWidget(bounds);
 
  boxWidget->AddObserver(vtkCommand::InteractionEvent, boxCallback);
  
  // Render
  renderWindow->Render();
  
  renderWindowInteractor->Initialize();
  renderWindow->Render();
  boxWidget->On();
  
  // Begin mouse interaction
  renderWindowInteractor->Start();
  
  return EXIT_SUCCESS;
}
