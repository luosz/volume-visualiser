# -*- coding: utf-8 -*-
"""
Created on Mon Sep 22 13:42:29 2014

@author: JoeShengzhou
"""

## https://gitorious.org/kitware/vtk/source/a30d5065fc63bc746fd418ca7a5b106a0d3723bc:Examples/VolumeRendering/Python/SimpleRayCast.py

#!/usr/bin/env python
 
# This is a simple volume rendering example that uses a
# vtkVolumeRayCast mapper

import vtk
from vtk.util.misc import vtkGetDataRoot
import Tkinter, tkFileDialog
import xml.etree.ElementTree as ET

def get_volume_filename():
    root = Tkinter.Tk()
    root.withdraw()
    filename = tkFileDialog.askopenfilename(parent=root, filetypes=[('Metaimage', '*.mhd'), ('all files', '*')], title='Select a volume data set')
    if len(filename ) == 0:
        filename = "D:/_data/CT-Knee.mhd"
    return filename

def load_transfer_function():
    root = Tkinter.Tk()
    root.withdraw()
    filename = tkFileDialog.askopenfilename(parent=root, filetypes=[('Voreen transfer functions', '*.tfi'), ('all files', '*')], title='Select a transfer function')
    if len(filename ) == 0:
        filename = "tf_optimized.tfi"
        
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

    # Create transfer mapping scalar value to opacity
    opacityTransferFunction = vtk.vtkPiecewiseFunction()
     
    # Create transfer mapping scalar value to color
    colorTransferFunction = vtk.vtkColorTransferFunction()
    
    max_intensity = 255
    for i in range(len(list_intensity)):
        intensity = float(list_intensity[i]) * max_intensity
        r = float(list_r[i]) / max_intensity
        g = float(list_g[i]) / max_intensity
        b = float(list_b[i]) / max_intensity
        a = float(list_a[i]) / max_intensity
        opacityTransferFunction.AddPoint(intensity, a)
        colorTransferFunction.AddRGBPoint(intensity, r, g, b)

    return opacityTransferFunction, colorTransferFunction

def show_volume():
    VTK_DATA_ROOT = vtkGetDataRoot()
     
    # Create the standard renderer, render window and interactor
    ren = vtk.vtkRenderer()
    renWin = vtk.vtkRenderWindow()
    renWin.AddRenderer(ren)
    iren = vtk.vtkRenderWindowInteractor()
    iren.SetRenderWindow(renWin)
    
    volume_filename = get_volume_filename()
    opacityTransferFunction, colorTransferFunction = load_transfer_function()
     
    # Create the reader for the data
    #reader = vtk.vtkStructuredPointsReader()
    #reader.SetFileName(VTK_DATA_ROOT + "/Data/ironProt.vtk")
    reader = vtk.vtkMetaImageReader()
    reader.SetFileName(volume_filename)
     
    # The property describes how the data will look
    volumeProperty = vtk.vtkVolumeProperty()
    volumeProperty.SetColor(colorTransferFunction)
    volumeProperty.SetScalarOpacity(opacityTransferFunction)
    volumeProperty.ShadeOn()
    volumeProperty.SetInterpolationTypeToLinear()

    ### for vtkVolumeRayCastMapper
    ## The mapper / ray cast function know how to render the data
    #compositeFunction = vtk.vtkVolumeRayCastCompositeFunction()
    #volumeMapper = vtk.vtkVolumeRayCastMapper()
    #volumeMapper.SetVolumeRayCastFunction(compositeFunction)
    
    ## for vtkSmartVolumeMapper
    #volumeMapper = vtk.vtkSmartVolumeMapper()
    #volumeMapper.SetRequestedRenderMode(vtk.vtkSmartVolumeMapper.GPURenderMode)
    
    ## for vtkGPUVolumeRayCastMapper
    volumeMapper = vtk.vtkGPUVolumeRayCastMapper()
    volumeMapper.SetInputConnection(reader.GetOutputPort())
     
    # The volume holds the mapper and the property and
    # can be used to position/orient the volume
    volume = vtk.vtkVolume()
    volume.SetMapper(volumeMapper)
    volume.SetProperty(volumeProperty)
     
    ren.AddVolume(volume)
    ren.SetBackground(1, 1, 1)
    renWin.SetSize(600, 600)
    renWin.Render()
     
    def CheckAbort(obj, event):
        if obj.GetEventPending() != 0:
            obj.SetAbortRender(1)
     
    renWin.AddObserver("AbortCheckEvent", CheckAbort)
     
    iren.Initialize()
    renWin.Render()
    iren.Start()

if __name__ == "__main__":
    show_volume()
