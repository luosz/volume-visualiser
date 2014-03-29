# Voreen Python script
import voreen
import voreenqt

voreen.info()
voreenqt.info()

for i in range(0, 48):
    voreen.setPropertyValue("VolumeSelector", "volumeID", i)
    tf_filename = "../../../../output/isabel_QVAPOR/%02d.tfi" % i
    voreen.loadTransferFunction("SingleVolumeRaycaster", "transferFunction", tf_filename)
    print "loaded transfer function %s" % tf_filename
    voreen.repaint()
    image_filename = "../../../../output/isabel_QVAPOR_output/%02d.png" % i
    voreen.snapshotCanvas(0, image_filename)
    print "saved to image %s" % image_filename

voreen.setPropertyValue("VolumeSelector", "volumeID", 0)
