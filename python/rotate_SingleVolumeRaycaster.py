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
    print(voreen.getPropertyValue(processor, cameraProp))
    initCam    = ((70.0, 270, 80), (70.0, 60.0, 80), (0.0, 0.0, -1.0))
    voreen.setPropertyValue(processor, cameraProp, initCam)
    voreen.repaint()
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
    print(voreen.getPropertyValue(processor, cameraProp))
    initCam    = ((70.0, 60, -130), (70.0, 60.0, 80), (0.0, -1.0, 0))
    voreen.setPropertyValue(processor, cameraProp, initCam)
    voreen.repaint()
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
    print(voreen.getPropertyValue(processor, cameraProp))
    initCam    = ((70.0, 60, 290), (70.0, 60.0, 80), (0.0, 1.0, 0))
    voreen.setPropertyValue(processor, cameraProp, initCam)
    voreen.repaint()
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
    print(voreen.getPropertyValue(processor, cameraProp))
    initCam    = ((70, -150, 80), (70.0, 60.0, 80), (0.0, 0.0, -1.0))
    voreen.setPropertyValue(processor, cameraProp, initCam)
    voreen.repaint()
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
    print(voreen.getPropertyValue(processor, cameraProp))
    initCam    = ((-140, 60, 80), (70.0, 60.0, 80), (0.0, 0.0, -1.0))
    voreen.setPropertyValue(processor, cameraProp, initCam)
    voreen.repaint()
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
    print(voreen.getPropertyValue(processor, cameraProp))
    initCam    = ((280, 60, 80), (70.0, 60.0, 80), (0.0, 0.0, -1.0))
    voreen.setPropertyValue(processor, cameraProp, initCam)
    voreen.repaint()
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

#front()
#back()
#top()
#bottom()
left()
#right()
print(voreen.getPropertyValue(processor, cameraProp))