# Voreen Python script
import voreen
import voreenqt

voreen.info()
voreenqt.info()

for i in range(0, 98):

    voreen.setPropertyValue("VolumeSelector 2", "volumeID", i)

    voreen.setPropertyValue("VolumeSave 2", "outputFilename", "../../../../output/vortex%02d.mhd" % (i))

    voreenqt.messageBox("../../../../output/vortex%02d.mhd" % (i))
