# Voreen Python script
import voreen
import voreenqt

voreen.info()
voreenqt.info()

voreen.setPropertyValue("VolumeSelector", "volumeID", 0)

for i in range(0, 48):

    voreen.setPropertyValue("VolumeSelector", "volumeID", i)

    filename = "../../../../output/isabel/%02d.mhd" % i

    voreen.setPropertyValue("VolumeSave", "outputFilename", filename)

    voreen.repaint()

    print filename
