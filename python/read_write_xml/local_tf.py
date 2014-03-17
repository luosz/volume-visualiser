'''
Created on 17 Mar 2014

@author: Shengzhou
'''

import sys
import xml.etree.ElementTree as ET

# number of transfer functions
n = 98
filename_in = "D:\\_uchar\\vortex\\%02d.tfi"

if len(sys.argv) == 3:
    print 'Number of arguments:', len(sys.argv), 'arguments.'
    print 'Argument List:', str(sys.argv)
#     print sys.argv[1]
#     print sys.argv[2]
    n = int(sys.argv[1])
    filename_in = sys.argv[2]

filename_out = "average.tfi"
index = filename_in.rfind("/")
if index == -1:
    index = filename_in.rfind("\\")
if index != -1:
    filename_out = filename_in[:index + 1] + filename_out
    print filename_out

# initialize variables
domain_x = 0
domain_y = 0
threshold_x = 0
threshold_y = 0
list_intensity = []
list_split = []
list_r = []
list_g = []
list_b = []
list_a = []

# print "list lenght=", len(list_intensity)

for j in range(0, n):
    filename = filename_in % j
#     print filename
    tree = ET.parse(filename)
    root = tree.getroot()
    
    TransFuncIntensity = root.find("TransFuncIntensity")
    domain = TransFuncIntensity.find("domain")
    domain_x += float(domain.get("x"))
    domain_y += float(domain.get("y"))
    threshold = TransFuncIntensity.find("threshold")
    threshold_x += float(threshold.get("x"))
    threshold_y += float(threshold.get("y"))
    
    
    if j == 0:
#         print "list lenght=", len(list_intensity), " append items to list"
        for key in root.iter('key'):
            colour = key.find("colorL")
            list_intensity.append(float(key.find("intensity").get("value")))
            list_split.append(key.find("split").get("value"))
            list_r.append(float(colour.get("r")))
            list_g.append(float(colour.get("g")))
            list_b.append(float(colour.get("b")))
            list_a.append(float(colour.get("a")))        
    else:
#         print "summarize"
        k = 0
        for key in root.iter('key'):
            colour = key.find("colorL")
            list_intensity[k] += float(key.find("intensity").get("value"))
            
            # simply take the last split value
            list_split[k] = key.find("split").get("value")
            
            list_r[k] += float(colour.get("r"))
            list_g[k] += float(colour.get("g"))
            list_b[k] += float(colour.get("b"))
            list_a[k] += float(colour.get("a"))
            k += 1

# calculate average       
domain_x /= n
domain_y /= n
threshold_x /= n
threshold_y /= n
for i in range(len(list_intensity)):
    list_intensity[i] /= n
    list_r[i] /= n
    list_g[i] /= n
    list_b[i] /= n
    list_a[i] /= n

# build xml tree
root1 = ET.Element("VoreenData")
root1.set("version", "1")
TransFuncIntensity1 = ET.SubElement(root1, "TransFuncIntensity")
TransFuncIntensity1.set("type", "TransFuncIntensity")
domain1 = ET.SubElement(TransFuncIntensity1, "domain")
domain1.set("x", str(domain_x))
domain1.set("y", str(domain_y))
threshold1 = ET.SubElement(TransFuncIntensity1, "threshold")
threshold1.set("x", str(threshold_x))
threshold1.set("y", str(threshold_y))
Keys1 = ET.SubElement(TransFuncIntensity1, "Keys")

for i in range(len(list_intensity)):
#     print i
#     print list_intensity[i], list_split[i], list_r[i], list_g[i], list_b[i], list_a[i]
     
    key1 = ET.SubElement(Keys1, "key")
    key1.set("type", "TransFuncMappingKey")
    intensity1 = ET.SubElement(key1, "intensity")
    intensity1.set("value", str(list_intensity[i]))
    split1 = ET.SubElement(key1, "split")
    split1.set("value", list_split[i])
    colorL1 = ET.SubElement(key1, "colorL")
    colorL1.set("r", str(list_r[i]))
    colorL1.set("g", str(list_g[i]))
    colorL1.set("b", str(list_b[i]))
    colorL1.set("a", str(list_a[i]))

xml_string = ET.tostring(root1)
import xml.dom.minidom as MD
xml =  MD.parseString(xml_string)
pretty_xml_as_string = xml.toprettyxml()
root2 = ET.fromstring(pretty_xml_as_string)
et2 = ET.ElementTree(root2)
et2.write(filename_out, encoding='utf-8', xml_declaration=True)
