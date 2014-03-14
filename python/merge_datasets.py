# -*- coding: utf-8 -*-
"""
Created on Thu Mar 13 16:39:55 2014

@author: JoeShengzhou
"""

size = 4

for i in range(1, 49):
    str1 = "D:/_time_varying_data/sciviscontest_2004_isabel/U/Uf%02d.bin" % i
    str2 = "D:/_time_varying_data/sciviscontest_2004_isabel/V/Vf%02d.bin" % i
    str3 = "D:/_time_varying_data/sciviscontest_2004_isabel/W/Wf%02d.bin" % i
    str4 = "D:/_time_varying_data/sciviscontest_2004_isabel/UVW/UVWf%02d.bin" % i
    print str1, str2, str3, str4

    f1 = open(str1, "rb")
    f2 = open(str2, "rb")
    f3 = open(str3, "rb")
    
    f4 = open(str4, "wb")
    
    data1 = f1.read(size)
    data2 = f2.read(size)
    data3 = f3.read(size)
    
    while data1 and data2 and data3:
        f4.write(data1)
        f4.write(data2)
        f4.write(data3)
        data1 = f1.read(size)
        data2 = f2.read(size)
        data3 = f3.read(size)
    
    f4.close()
    
    f1.close()
    f2.close()
    f3.close()
