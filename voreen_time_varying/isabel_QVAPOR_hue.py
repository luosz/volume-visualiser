# Voreen Python script
import voreen
import voreenqt

voreen.info()
voreenqt.info()

#voreen.setPropertyValue("VolumeSelector", "volumeID", 0)
for i in range(0, 16):
#    voreen.setPropertyValue("VolumeSelector", "volumeID", i)
    tf_filename = "../../../../output/_tf/%02d.tfi" % i
    voreen.loadTransferFunction("SingleVolumeRaycaster", "transferFunction", tf_filename)
    voreen.loadTransferFunction("SingleVolumeRaycaster 2", "transferFunction", tf_filename)
    print "loaded transfer function %s" % tf_filename
    voreen.repaint()
    image_filename = "../../../../output/_tf/%02d.png" %i
    image_filename2 = "../../../../output/_tf/%02d_clipped.png" %i
    voreen.snapshotCanvas(0, image_filename)
    voreen.snapshotCanvas(1, image_filename2)
    print "saved to image %s" % image_filename
