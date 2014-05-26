#ifndef transfer_function_xml_h
#define transfer_function_xml_h

#include <string>
#include <string.h>
#include <ctype.h>
#include <iostream>

#include <vtkSmartPointer.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>

#include "tinyxml2/tinyxml2.h"

#ifndef MAX_PATH
#define MAX_PATH          260
#endif

/************************************************************************
3D Slicer transfer function XML file format:
<?xml version="1.0" encoding="utf-8"?>
<MRML>
<VolumeProperty selected="false" hideFromEditors="false" name="CT-AAA" gradientOpacity="4 0 1 255 1" userTags="" specularPower="10" scalarOpacity="12 -3024 0 143.556 0 166.222 0.686275 214.389 0.696078 419.736 0.833333 3071 0.803922" id="vtkMRMLVolumePropertyNode1" specular="0.2" shade="1" ambient="0.1" colorTransfer="24 -3024 0 0 0 143.556 0.615686 0.356863 0.184314 166.222 0.882353 0.603922 0.290196 214.389 1 1 1 419.736 1 0.937033 0.954531 3071 0.827451 0.658824 1" selectable="true" diffuse="0.9" interpolation="1"/>
</MRML>
************************************************************************/

struct TransferFunctionXML
{
public:
	bool selected;
	bool hideFromEditors;
	std::string name;
	std::string userTags;
	std::string id;
	bool selectable;
	vtkSmartPointer<vtkVolumeProperty> volume;

	vtkSmartPointer<vtkVolumeProperty> Volume() const { return volume; }
	void Volume(vtkSmartPointer<vtkVolumeProperty> val) { volume = val; }

	TransferFunctionXML()
	{
		selected = hideFromEditors = false;
		selectable = true;
		volume = vtkSmartPointer<vtkVolumeProperty>::New();
	}

	~TransferFunctionXML(){}

	void parse(const char *filename)
	{
		char * msg = "An error occurred in TransferFunctionXML.parse()";

		tinyxml2::XMLDocument doc;
		auto r = doc.LoadFile(filename);
		if (r != tinyxml2::XML_NO_ERROR)
		{
			std::cout << "failed to open file" << endl;
			return;
		}

		auto mrml = doc.FirstChildElement("MRML");
		if (mrml)
		{
			parse_Slicer_XML(doc);
		}
		else
		{
			auto transferFunction = doc.FirstChildElement("TransferFunction");
			if (!transferFunction)
			{
				std::cerr << msg << " Neither <MRML> nor <TransferFunction> exists." << std::endl;
				return;
			}
			parse_MITK_XML(doc);
		}
	}

	void parse_Slicer_XML(const tinyxml2::XMLDocument &doc)
	{
		std::cout << "parse_Slicer_XML()" << std::endl;

		char * msg = "An error occurred in TransferFunctionXML.parse_Slicer_XML()";

		auto property = doc.FirstChildElement("MRML")->FirstChildElement("VolumeProperty");
		if (!property)
		{
			std::cerr << msg << " <VolumeProperty> doesn't exist in <MRML>." << std::endl;
			return;
		}

		char s[MAX_PATH];
		strcpy(s, property->Attribute("selected"));
		to_lower_case(s);
		std::cout << "selected=" << s << std::endl;
		selected = (0 == strcmp(s, "true"));

		strcpy(s, property->Attribute("hideFromEditors"));
		to_lower_case(s);
		std::cout << "hideFromEditors=" << s << std::endl;
		hideFromEditors = (0 == strcmp(s, "true"));

		name = property->Attribute("name");
		userTags = property->Attribute("userTags");
		id = property->Attribute("id");

		strcpy(s, property->Attribute("selectable"));
		to_lower_case(s);
		std::cout << "selectable=" << s << std::endl;
		selectable = (0 == strcmp(s, "true"));

		volume->SetSpecularPower(atof(property->Attribute("specularPower")));
		volume->SetSpecular(atof(property->Attribute("specular")));
		volume->SetShade(atoi(property->Attribute("shade")));
		volume->SetAmbient(atof(property->Attribute("ambient")));
		volume->SetDiffuse(atof(property->Attribute("diffuse")));

		/// #define 	VTK_NEAREST_INTERPOLATION   0
		/// #define 	VTK_LINEAR_INTERPOLATION   1
		volume->SetInterpolationType(atoi(property->Attribute("interpolation")));

		volume->SetGradientOpacity(parse_piecewise(property->Attribute("gradientOpacity")));
		volume->SetScalarOpacity(parse_piecewise(property->Attribute("scalarOpacity")));
		volume->SetColor(parse_color(property->Attribute("colorTransfer")));
	}

	void parse_MITK_XML(const tinyxml2::XMLDocument &doc)
	{
		std::cout << "parse_MITK_XML()" << std::endl;

		{
			std::cout << "<ScalarOpacity>" << std::endl;
			auto scalar = vtkSmartPointer<vtkPiecewiseFunction>::New();
			auto point = doc.FirstChildElement("TransferFunction")->FirstChildElement("ScalarOpacity")->FirstChildElement("point");
			do
			{
				double x = atof(point->Attribute("x"));
				double y = atof(point->Attribute("y"));
				std::cout << "x=" << x << " y=" << y << std::endl;
				scalar->AddPoint(x, y);

				point = point->NextSiblingElement();
			} while (point);

			volume->SetScalarOpacity(scalar);
		}

		{
			std::cout << "<GradientOpacity>" << std::endl;
			auto gradient = vtkSmartPointer<vtkPiecewiseFunction>::New();
			auto point = doc.FirstChildElement("TransferFunction")->FirstChildElement("GradientOpacity")->FirstChildElement("point");
			do
			{
				double x = atof(point->Attribute("x"));
				double y = atof(point->Attribute("y"));
				std::cout << "x=" << x << " y=" << y << std::endl;
				gradient->AddPoint(x, y);

				point = point->NextSiblingElement();
			} while (point);

			volume->SetGradientOpacity(gradient);
		}

		{
			std::cout << "<Color>" << std::endl;
			auto color = vtkSmartPointer<vtkColorTransferFunction>::New();
			auto point = doc.FirstChildElement("TransferFunction")->FirstChildElement("Color")->FirstChildElement("point");
			do
			{
				double x = atof(point->Attribute("x"));
				double r = atof(point->Attribute("r"));
				double g = atof(point->Attribute("g"));
				double b = atof(point->Attribute("b"));
				double midpoint = atof(point->Attribute("midpoint"));
				double sharpness = atof(point->Attribute("sharpness"));
				std::cout << "x=" << x << " r=" << r << " g=" << g << " b=" << b << " midpoint=" << midpoint << " sharpness=" << sharpness << std::endl;
				color->AddRGBPoint(x, r, g, b);

				point = point->NextSiblingElement();
			} while (point);

			volume->SetColor(color);
		}
	}

	void to_lower_case(char *str)
	{
		for (int i = 0; str[i]; i++)
		{
			str[i] = tolower(str[i]);
		}
	}

	vtkSmartPointer<vtkPiecewiseFunction> parse_piecewise(const char *s)
	{
		char *msg = "An error occurred in TransferFunctionXML.parse_piecewise()";
		auto piecewise = vtkSmartPointer<vtkPiecewiseFunction>::New();
		char str[MAX_PATH];
		strcpy(str, s);
		char *pch;
		pch = strtok(str, " ");

		if (!pch)
		{
			std::cerr << msg << std::endl;
			return piecewise;
		}

		int n = atoi(pch);
		for (int i = 0; i < n; i++)
		{
			pch = strtok(NULL, " ");
			if (!pch)
			{
				std::cerr << msg << std::endl;
				break;
			}
			double x = atof(pch);
			pch = strtok(NULL, " ");
			if (!pch)
			{
				std::cerr << msg << std::endl;
				break;
			}
			double y = atof(pch);

			piecewise->AddPoint(x, y);
		}

		std::cout << "parse_piecewise" << std::endl;
		for (int i = 0; i < piecewise->GetSize(); i++)
		{
			double xa[4];
			piecewise->GetNodeValue(i, xa);
			std::cout << xa[0] << " " << xa[1] << std::endl;
		}

		return piecewise;
	}

	vtkSmartPointer<vtkColorTransferFunction> parse_color(const char *s)
	{
		auto color = vtkSmartPointer<vtkColorTransferFunction>::New();
		char *msg = "An error occurred in TransferFunctionXML.parse_color()";
		char str[MAX_PATH];
		strcpy(str, s);
		char *pch;
		pch = strtok(str, " ");

		if (!pch)
		{
			std::cerr << msg << std::endl;
			return color;
		}

		int n = atoi(pch);
		for (int i = 0; i < n; i++)
		{
			pch = strtok(NULL, " ");
			if (!pch)
			{
				std::cerr << msg << std::endl;
				break;
			}
			double x = atof(pch);
			pch = strtok(NULL, " ");
			if (!pch)
			{
				std::cerr << msg << std::endl;
				break;
			}
			double r = atof(pch);
			pch = strtok(NULL, " ");
			if (!pch)
			{
				std::cerr << msg << std::endl;
				break;
			}
			double g = atof(pch);
			pch = strtok(NULL, " ");
			if (!pch)
			{
				std::cerr << msg << std::endl;
				break;
			}
			double b = atof(pch);

			color->AddRGBPoint(x, r, g, b);
		}

		std::cout << "parse_color" << std::endl;
		for (int i = 0; i < color->GetSize(); i++)
		{
			double xrgb[6];
			color->GetNodeValue(i, xrgb);
			std::cout << xrgb[0] << " " << xrgb[1] << " " << xrgb[2] << " " << xrgb[3] << std::endl;
		}

		return color;
	}
};

#endif // transfer_function_xml_h
