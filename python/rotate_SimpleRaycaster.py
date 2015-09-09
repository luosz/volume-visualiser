# Voreen Python script
import voreen
import voreenqt
import time
import math

voreen.info()
voreenqt.info()

# --- configuration ---
frames    = 100
canvasDim = (512, 512)

processor  = "SingleVolumeRaycaster"
cameraProp = "camera"
canvas     = "Canvas"
rotAxis    = (1.0, 0.0, 0.0)
# --- configuration ---

# Camera
# const vec3 &     position = vec3(0.f, 0.f,  0.f),
# const vec3 &     focus = vec3(0.f, 0.f, -1.f),
# const vec3 &     up = vec3(0.f, 1.f,  0.f),

def front():
    initCam    = ((0.0, 2.75, 0.0), (0.0, 0.0, 0.0), (0.0, 1.0, 0.0))
    # store current canvas size and camera settings
    prevDim = voreen.getPropertyValue(canvas, "canvasSize")
    prevCam = voreen.getPropertyValue(processor, cameraProp)
    print(prevCam)
    # resize canvas and initialize camera
    voreen.setPropertyValue(canvas, "canvasSize", canvasDim)
    voreen.setPropertyValue(processor, cameraProp, initCam)
    voreen.repaint()
    # make sure all Qt events have been processed before starting
    voreenqt.processEvents()  
    # start loop for animation
    counter = 0
    angleIncr = 2*math.pi / frames  # do a full rotation
    start = time.time()
    while (counter < frames):
        counter = counter + 1
        # update camera for current frame
        voreen.rotateCamera(processor, cameraProp, angleIncr, rotAxis)
        # render network state
        voreen.repaint()
    end = time.time()

def back():
    initCam    = ((0.0, -2.75, 0.0), (0.0, 0.0, 0.0), (0.0, 1.0, 0.0))
    # store current canvas size and camera settings
    prevDim = voreen.getPropertyValue(canvas, "canvasSize")
    prevCam = voreen.getPropertyValue(processor, cameraProp)
    print(prevCam)
    # resize canvas and initialize camera
    voreen.setPropertyValue(canvas, "canvasSize", canvasDim)
    voreen.setPropertyValue(processor, cameraProp, initCam)
    voreen.repaint()
    # make sure all Qt events have been processed before starting
    voreenqt.processEvents()  
    # start loop for animation
    counter = 0
    angleIncr = 2*math.pi / frames  # do a full rotation
    start = time.time()
    while (counter < frames):
        counter = counter + 1
        # update camera for current frame
        voreen.rotateCamera(processor, cameraProp, angleIncr, rotAxis)
        # render network state
        voreen.repaint()
    end = time.time()

def top():
    initCam    = ((0.0, 0.0, -2.75), (0.0, 0.0, 0.0), (0.0, 1.0, 0.0))
    # store current canvas size and camera settings
    prevDim = voreen.getPropertyValue(canvas, "canvasSize")
    prevCam = voreen.getPropertyValue(processor, cameraProp)
    print(prevCam)
    # resize canvas and initialize camera
    voreen.setPropertyValue(canvas, "canvasSize", canvasDim)
    voreen.setPropertyValue(processor, cameraProp, initCam)
    voreen.repaint()
    # make sure all Qt events have been processed before starting
    voreenqt.processEvents()  
    # start loop for animation
    counter = 0
    angleIncr = 2*math.pi / frames  # do a full rotation
    start = time.time()
    while (counter < frames):
        counter = counter + 1
        # update camera for current frame
        voreen.rotateCamera(processor, cameraProp, angleIncr, rotAxis)
        # render network state
        voreen.repaint()
    end = time.time()

def bottom():
    initCam    = ((0.0, 0.0, 2.75), (0.0, 0.0, 0.0), (0.0, 1.0, 0.0))
    # store current canvas size and camera settings
    prevDim = voreen.getPropertyValue(canvas, "canvasSize")
    prevCam = voreen.getPropertyValue(processor, cameraProp)
    print(prevCam)
    # resize canvas and initialize camera
    voreen.setPropertyValue(canvas, "canvasSize", canvasDim)
    voreen.setPropertyValue(processor, cameraProp, initCam)
    voreen.repaint()
    # make sure all Qt events have been processed before starting
    voreenqt.processEvents()  
    # start loop for animation
    counter = 0
    angleIncr = 2*math.pi / frames  # do a full rotation
    start = time.time()
    while (counter < frames):
        counter = counter + 1
        # update camera for current frame
        voreen.rotateCamera(processor, cameraProp, angleIncr, rotAxis)
        # render network state
        voreen.repaint()
    end = time.time()

def left():
    initCam    = ((-2.75, 0.0, 0.0), (0.0, 0.0, 0.0), (0.0, 0.0, -1.0))
    # store current canvas size and camera settings
    prevDim = voreen.getPropertyValue(canvas, "canvasSize")
    prevCam = voreen.getPropertyValue(processor, cameraProp)
    print(prevCam)
    # resize canvas and initialize camera
    voreen.setPropertyValue(canvas, "canvasSize", canvasDim)
    voreen.setPropertyValue(processor, cameraProp, initCam)
    voreen.repaint()
    # make sure all Qt events have been processed before starting
    voreenqt.processEvents()  
    # start loop for animation
    counter = 0
    angleIncr = 2*math.pi / frames  # do a full rotation
    start = time.time()
    while (counter < frames):
        counter = counter + 1
        # update camera for current frame
        voreen.rotateCamera(processor, cameraProp, angleIncr, rotAxis)
        # render network state
        voreen.repaint()
    end = time.time()

def right():
    initCam    = ((2.75, 0.0, 0.0), (0.0, 0.0, 0.0), (0.0, 0.0, -1.0))
    # store current canvas size and camera settings
    prevDim = voreen.getPropertyValue(canvas, "canvasSize")
    prevCam = voreen.getPropertyValue(processor, cameraProp)
    print(prevCam)
    # resize canvas and initialize camera
    voreen.setPropertyValue(canvas, "canvasSize", canvasDim)
    voreen.setPropertyValue(processor, cameraProp, initCam)
    voreen.repaint()
    # make sure all Qt events have been processed before starting
    voreenqt.processEvents()  
    # start loop for animation
    counter = 0
    angleIncr = 2*math.pi / frames  # do a full rotation
    start = time.time()
    while (counter < frames):
        counter = counter + 1
        # update camera for current frame
        voreen.rotateCamera(processor, cameraProp, angleIncr, rotAxis)
        # render network state
        voreen.repaint()
    end = time.time()

front()
#back()
#top()
#bottom()
#left()
#right()
print(voreen.getPropertyValue(processor, cameraProp))