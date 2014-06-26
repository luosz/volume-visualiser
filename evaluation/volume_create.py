# Voreen Python script
import voreen
import voreenqt
import time
from time import gmtime, strftime

voreen.info()
voreenqt.info()

t = strftime("%Y%m%d_%H%M%S", gmtime())
voreen.setPropertyValue("VolumeCreate", "regenerate", 0)
filename = "../../../../evaluation/volume_" + t + ".mhd"
print filename
voreen.setPropertyValue("VolumeSave", "outputFilename", filename)
voreen.repaint()
