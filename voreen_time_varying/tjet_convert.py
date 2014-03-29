# Voreen Python script
import voreen
import voreenqt

voreen.info()
voreenqt.info()

for i in range(0, 150):
    voreen.setPropertyValue("VolumeSelector", "volumeID", i)
    filename = "../../../../_uchar/tjet/%03d.mhd" % i
    voreen.setPropertyValue("VolumeSave", "outputFilename", filename)
    voreen.repaint()
    print filename

voreen.setPropertyValue("VolumeSelector", "volumeID", 0)
