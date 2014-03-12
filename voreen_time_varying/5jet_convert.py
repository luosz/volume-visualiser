# Voreen Python script
import voreen
import voreenqt

voreen.info()
voreenqt.info()

voreen.setPropertyValue("VolumeSelector", "volumeID", 0)
voreen.setPropertyValue("VolumeSelector 2", "volumeID", 0)
voreen.setPropertyValue("VolumeSelector 3", "volumeID", 0)

for i in range(0, 100):
    voreen.setPropertyValue("VolumeSelector", "volumeID", i)
    voreen.setPropertyValue("VolumeSelector 2", "volumeID", i)
    voreen.setPropertyValue("VolumeSelector 3", "volumeID", i)

    filename = "../../../../_uchar/5jet_Density/%02d.mhd" % i
    filename2 = "../../../../_uchar/5jet_Energy/%02d.mhd" % i
    filename3 = "../../../../_uchar/5jet_Velocity/%02d.mhd" % i

    voreen.setPropertyValue("VolumeSave", "outputFilename", filename)
    voreen.setPropertyValue("VolumeSave 2", "outputFilename", filename2)
    voreen.setPropertyValue("VolumeSave 3", "outputFilename", filename3)

    voreen.repaint()

    print filename
