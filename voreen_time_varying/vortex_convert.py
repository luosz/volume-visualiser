# Voreen Python script
import voreen
import voreenqt

voreen.info()
voreenqt.info()

for i in range(0, 98):

    voreen.setPropertyValue("VolumeSelector", "volumeID", i)

    voreen.setPropertyValue("VolumeSave", "outputFilename", "../../../../output/vortex/vortex%02d.mhd" % (i))

    voreenqt.messageBox("../../../../output/vortex/vortex%02d.mhd" % (i))
