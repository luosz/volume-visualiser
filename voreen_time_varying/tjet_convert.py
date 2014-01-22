# Voreen Python script
import voreen
import voreenqt

voreen.info()
voreenqt.info()

for i in range(0, 150):

    voreen.setPropertyValue("VolumeSelector", "volumeID", i)

    voreen.setPropertyValue("VolumeSave", "outputFilename", "../../../../output/tjet/tjet%03d.mhd" % (i))

    voreenqt.messageBox("../../../../output/tjet/tjet%03d.mhd" % (i))
