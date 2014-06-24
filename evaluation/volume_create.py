# Voreen Python script
import voreen
import voreenqt
import time

voreen.info()
voreenqt.info()

from time import gmtime, strftime
t = strftime("%Y%m%d_%H%M%S", gmtime())
voreen.setPropertyValue("VolumeCreate", "regenerate", 0)
filename = "../../../../random/volume_" + t + ".mhd"
print filename
voreen.setPropertyValue("VolumeSave", "outputFilename", filename)
voreen.repaint()
