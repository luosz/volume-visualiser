# -*- coding: utf-8 -*-
"""
Created on Tue Mar 11 22:44:52 2014

@author: JoeShengzhou
"""

#==============================================================================
# generate strings for voreen workspace files
#==============================================================================

for i in range(1, 2001):
#    string1 = "                                <url value=\"raw://../../../../_time_varying_data/5Jets/Density/ns_%04d_r.dat?dim_x=128&amp;dim_y=128&amp;dim_z=128&amp;format=FLOAT&amp;headerskip=0&amp;objectModel=I&amp;spacing_x=1&amp;spacing_y=1&amp;spacing_z=1&amp;timeframe=0\" />" % i
#    print string1
    
#    string2 = "                                <url value=\"raw://../../../../_time_varying_data/5Jets/Energy/ns_%04d_e.dat?dim_x=128&amp;dim_y=128&amp;dim_z=128&amp;format=FLOAT&amp;headerskip=0&amp;objectModel=I&amp;spacing_x=1&amp;spacing_y=1&amp;spacing_z=1&amp;timeframe=0\" />" % i
#    print string2
    
    string3 = "                                <url value=\"raw://../../../../_time_varying_data/5Jets/Velocity/ns_%04d_v.dat?dim_x=128&amp;dim_y=128&amp;dim_z=128&amp;format=FLOAT&amp;headerskip=0&amp;objectModel=RGB&amp;spacing_x=1&amp;spacing_y=1&amp;spacing_z=1&amp;timeframe=0\" />" % i
    print string3
    