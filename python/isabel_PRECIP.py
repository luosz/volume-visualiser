# -*- coding: utf-8 -*-
"""
Created on Wed Mar 12 17:32:02 2014

@author: JoeShengzhou
"""

#==============================================================================
# generate strings for voreen workspace files
#==============================================================================

print "                            <VolumeURLs>"

for i in range(1, 49):
    string1 = "                                <url value=\"raw://../../../../_time_varying_data/sciviscontest_2004_isabel/PRECIP/PRECIPf%02d.bin?bigEndian=1&amp;dim_x=500&amp;dim_y=500&amp;dim_z=100&amp;format=FLOAT&amp;headerskip=0&amp;objectModel=I&amp;spacing_x=1&amp;spacing_y=1&amp;spacing_z=1&amp;timeframe=0\" />" % i
    print string1

print "                            </VolumeURLs>"
