# Voreen Python script
import voreen
import voreenqt

voreen.info()
voreenqt.info()

for i in range(0, 98):
    voreen.setPropertyValue("VolumeSelector", "volumeID", i)
    filename = "../../../../_uchar/vortex/%02d.mhd" % i
    voreen.setPropertyValue("VolumeSave", "outputFilename", filename)
    voreen.repaint()
    print filename

voreen.setPropertyValue("VolumeSelector", "volumeID", 0)
