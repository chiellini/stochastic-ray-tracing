import somefunc.BMP as bp
import somefunc.Model as md
from somefunc.basefunc import clamp
import somefunc.Vector as vector
import numpy as np
from random import random
import math as mt

#分辨率
iResolution=np.array([640,480])
#采样点个数
SUB_SAMPLES=4
#最大深度
MAX_DEPTH=64
#最大样本循环
MAX_SAMPLE_LOOP=4
RESAMPLE_VALVE=1e-6
MAX_THREAD=4
#gama值
GAMMA=2.2


def reflect(source,normal):
    temp1=normal.MultipleDecimal(2)
    temp2=vector.vector3_instance.Dot(source,normal)
    temp=vector.vector3_instance.Dot(temp1,temp2)
    temp=source-temp
    return temp

def cosWeightedSampleHemisphere(n):
    u1=random()
    u2=random()
    r=mt.sqrt(u1)
    theta=2*mt.pi*u2

    x=r*mt.cos(theta)
    y=r*mt.sin(theta)
    z=mt.sqrt(1-u1)
    
    a=n
    b=vector.ZERO_VECTOR3

    if (abs(a.x)<=abs(a.y)) and (abs(a.x)<=abs(a.z)):
        a.x=1
    elif abs(a.y)<=abs(a.z):
        a.y=1
    else:
        a.z=1
    
    a.Normalize()
    
    a=vector.vector3_instance.Cross(n,a)
    b=vector.vector3_instance.Cross(n,a)

    temp1=a.MultipleDecimal(x)
    temp2=b.MultipleDecimal(y)
    temp3=n.MultipleDecimal(z)
    temp=(temp1+temp2+temp3).normalized()
    return temp

"""
以下开始真正初始化和设置场景
"""
NUM_SPHERES=6	#球体数量
NUM_PLANES=6	#平面数量

spheres=[]
planes=[]

def initScene():
    spheres.append(md.Sphere(16.5,vector.Vector3(27,16.5,47),md.Material(md.SPEC,vector.ZERO_VECTOR3,vector.ONE_VECTOR3,0.)))
    spheres.append(md.Sphere(16.5,vector.Vector3(73,16.5,78),md.Material(md.REFR,vector.ZERO_VECTOR3, vector.Vector3(.75,1.,.75), 1.5)))
    spheres.append(md.Sphere(5, vector.Vector3(50,70,50), md.Material(md.DIFF, vector.Vector3(5.7,5.7,5.7),vector.ZERO_VECTOR3,0.)))
    spheres.append(md.Sphere(14, vector.Vector3(50, 14, 60), md.Material(md.REFR,vector.ZERO_VECTOR3, vector.Vector3(0.5, 0.5, 0.), 2)))
    spheres.append(md.Sphere(12, vector.Vector3(92, 35, 65), md.Material(md.REFR, vector.ZERO_VECTOR3, vector.Vector3(0.5, 0.5, 1.), 4)))
    spheres.append(md.Sphere(18, vector.Vector3(8, 25, 80), md.Material(md.SPEC, vector.ZERO_VECTOR3, vector.Vector3(1., 0.5, 0.5), 2)))

    planes.append(md.Plane(vector.Vector3(0, 0, 0),vector.Vector3(0, 1, 0), md.Material(md.DIFF,vector.ZERO_VECTOR3, vector.ONE_VECTOR3, 0.)))
    planes.append(md.Plane(vector.Vector3(-7, 0, 0),vector.Vector3(1, 0, 0), md.Material(md.DIFF,vector.ZERO_VECTOR3,vector.Vector3(.75, .25, .25), 0.)))
    planes.append(md.Plane(vector.Vector3(0,0,0), vector.Vector3(0, 0, -1), md.Material(md.SPEC, vector.ZERO_VECTOR3, vector.Vector3(.75, .75, .75), 0.)))
    planes.append(md.Plane(vector.Vector3(107, 0, 0), vector.Vector3(-1, 0, 0), md.Material(md.SPEC, vector.ZERO_VECTOR3, vector.Vector3(.75, .75, 1), 0.)))
    planes.append(md.Plane(vector.Vector3(0, 0, 185), vector.Vector3(0, 0, 1), md.Material(md.SPEC, vector.ZERO_VECTOR3, vector.Vector3(1, 1, .75), 0.)))
    planes.append(md.Plane(vector.Vector3(0, 90, 0), vector.Vector3(0, -1, 0), md.Material(md.DIFF, vector.ZERO_VECTOR3, vector.Vector3(.75,.75,.75), 0.)))