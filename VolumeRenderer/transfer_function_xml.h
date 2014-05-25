#ifndef transfer_function_xml_h
#define transfer_function_xml_h

#include <string>

/************************************************************************
3D Slicer transfer function XML file format:
<?xml version="1.0" encoding="utf-8"?>
<MRML>
<VolumeProperty selected="false" hideFromEditors="false" name="CT-AAA" gradientOpacity="4 0 1 255 1" userTags="" specularPower="10" scalarOpacity="12 -3024 0 143.556 0 166.222 0.686275 214.389 0.696078 419.736 0.833333 3071 0.803922" id="vtkMRMLVolumePropertyNode1" specular="0.2" shade="1" ambient="0.1" colorTransfer="24 -3024 0 0 0 143.556 0.615686 0.356863 0.184314 166.222 0.882353 0.603922 0.290196 214.389 1 1 1 419.736 1 0.937033 0.954531 3071 0.827451 0.658824 1" selectable="true" diffuse="0.9" interpolation="1"/>
</MRML>
************************************************************************/

struct VolumePropertyXML
{
public:
	std::string selected;
	std::string hideFromEditors;
	std::string name;
	std::string gradientOpacity;
	std::string userTags;
	std::string specularPower;
	std::string scalarOpacity;
	std::string id;
	std::string specular;
	std::string shade;
	std::string ambient;
	std::string colorTransfer;
	std::string selectable;
	std::string diffuse;
	std::string interpolation;

	VolumePropertyXML()
	{
	}

	~VolumePropertyXML()
	{
	}
};

#endif // transfer_function_xml_h
