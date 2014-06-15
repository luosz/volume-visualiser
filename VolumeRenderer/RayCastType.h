#ifndef RayCastType_h
#define RayCastType_h

/// definition from vtkMRMLVolumeRenderingDisplayNode.h
#ifndef __vtkMRMLVolumeRenderingDisplayNode_h

namespace vtkMRMLVolumeRenderingDisplayNode
{
	/// Quality used for PerformanceControl
	enum Quality
	{
		Adaptative = 0,
		MaximumQuality
	};

	/// Performance Control method
	/// 0: Adaptive
	/// 1: Maximum Quality
	/// 2: Fixed Framerate // unsupported yet
	//int PerformanceControl;

	enum RayCastType
	{
		Composite = 0, // composite with directional lighting (default)
		CompositeEdgeColoring, // composite with fake lighting (edge coloring, faster)
		MaximumIntensityProjection,
		MinimumIntensityProjection,
		GradiantMagnitudeOpacityModulation,
		IllustrativeContextPreservingExploration
	};
}

#endif // __vtkMRMLVolumeRenderingDisplayNode_h

#endif // RayCastType_h
