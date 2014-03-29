# Voreen Python script
import voreen
import voreenqt

voreen.info()
voreenqt.info()

for i in range(0, 48):
    voreen.setPropertyValue("VolumeSelector", "volumeID", i)
    filename = "../../../../_uchar/isabel_TC/%02d.mhd" % i
    voreen.setPropertyValue("VolumeSave", "outputFilename", filename)
    voreen.repaint()
    print filename

voreen.setPropertyValue("VolumeSelector", "volumeID", 0)
