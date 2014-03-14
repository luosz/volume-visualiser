'''
Created on 14 Mar 2014

@author: JoeShengzhou
'''

# print "hello"

import xml.etree.ElementTree as ET
tree = ET.parse("standard.tfi")
root = tree.getroot()

TransFuncIntensity = root.find("TransFuncIntensity")
print TransFuncIntensity
domain = TransFuncIntensity.find("domain")
threshold = TransFuncIntensity.find("threshold")
print domain.get("x"), domain.get("y")
print threshold.get("x"), threshold.get("y")

# for key in root.iter('key'):
#     print key.find("intensity").get("value"), key.find("split").get("value")
#     colour = key.find("colorL")
#     print colour.get("r"), colour.get("g"), colour.get("b"), colour.get("a")



# build xml tree
root = ET.Element("root")
doc = ET.SubElement(root, "doc")
field1 = ET.SubElement(doc, "field1")
field1.set("name", "blah")
field1.text = "some value1"
field2 = ET.SubElement(doc, "field2")
field2.set("name", "asdfasd")
field2.text = "some vlaue2"
tree = ET.ElementTree(root)
# tree.write("filename.xml")

# from xml.etree.ElementTree import Element, tostring
#  
# document = Element('outer')
# node = ET.SubElement(document, 'inner')
# node.NewValue = 1
# print tostring(document)

# from xml.etree.ElementTree import ElementTree

# et = ET.ElementTree(root)
# 
# f = open("test.xml", "w")
# et.write(f, encoding='utf-8', xml_declaration=True) 
# f.close()

xml_string = ET.tostring(root)
# print xml_string

import xml.dom.minidom as MD
xml =  MD.parseString(xml_string)
pretty_xml_as_string = xml.toprettyxml()
# print pretty_xml_as_string

root2 = ET.fromstring(pretty_xml_as_string)
# print root2
et2 = ET.ElementTree(root2)
f2 = open("test2.xml", "w")
et2.write(f2, encoding='utf-8', xml_declaration=True) 
f2.close()
