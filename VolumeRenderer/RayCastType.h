#ifndef RayCastType_h
#define RayCastType_h

/// definition from vtkMRMLVolumeRenderingDisplayNode.h
#ifndef __vtkMRMLVolumeRenderingDisplayNode_h

namespace vtkMRMLVolumeRenderingDisplayNode
{
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

/// Performance Control method
/// 0: Adaptive
/// 1: Maximum Quality
/// 2: Fixed Framerate // unsupported yet

#endif // __vtkMRMLVolumeRenderingDisplayNode_h

#endif // RayCastType_h
