'''
Created on 14 Mar 2014

@author: Shengzhou
'''

import xml.etree.ElementTree as ET

j = 0
filename = "D:/_uchar/vortex/%02d.tfi" % j
print filename
tree = ET.parse(filename)
root = tree.getroot()

TransFuncIntensity = root.find("TransFuncIntensity")
domain = TransFuncIntensity.find("domain")
domain_x = domain.get("x")
domain_y = domain.get("y")
threshold = TransFuncIntensity.find("threshold")
threshold_x = threshold.get("x")
threshold_y = threshold.get("y")

list_intensity = []
list_split = []
list_r = []
list_g = []
list_b = []
list_a = []

for key in root.iter('key'):
    colour = key.find("colorL")
    list_intensity.append(key.find("intensity").get("value"))
    list_split.append(key.find("split").get("value"))
    list_r.append(colour.get("r"))
    list_g.append(colour.get("g"))
    list_b.append(colour.get("b"))
    list_a.append(colour.get("a"))

# build xml tree
root1 = ET.Element("VoreenData")
root1.set("version", "1")
TransFuncIntensity1 = ET.SubElement(root1, "TransFuncIntensity")
TransFuncIntensity1.set("type", "TransFuncIntensity")
domain1 = ET.SubElement(TransFuncIntensity1, "domain")
domain1.set("x", domain_x)
domain1.set("y", domain_y)
threshold1 = ET.SubElement(TransFuncIntensity1, "threshold")
threshold1.set("x", threshold_x)
threshold1.set("y", threshold_y)
Keys1 = ET.SubElement(TransFuncIntensity1, "Keys")

for i in range(len(list_intensity)):
    print i
    print list_intensity[i], list_split[i], list_r[i], list_g[i], list_b[i], list_a[i]
     
    key1 = ET.SubElement(Keys1, "key")
    key1.set("type", "TransFuncMappingKey")
    intensity1 = ET.SubElement(key1, "intensity")
    intensity1.set("value", list_intensity[i])
    split1 = ET.SubElement(key1, "split")
    split1.set("value", list_split[i])
    colorL1 = ET.SubElement(key1, "colorL")
    colorL1.set("r", list_r[i])
    colorL1.set("g", list_g[i])
    colorL1.set("b", list_b[i])
    colorL1.set("a", list_a[i])

xml_string = ET.tostring(root1)
import xml.dom.minidom as MD
xml =  MD.parseString(xml_string)
pretty_xml_as_string = xml.toprettyxml()
root2 = ET.fromstring(pretty_xml_as_string)
et2 = ET.ElementTree(root2)
et2.write("output.xml", encoding='utf-8', xml_declaration=True)
