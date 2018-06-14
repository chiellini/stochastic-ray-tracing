import numpy as np
from math import sqrt
from rope.base.oi.type_hinting.providers.inheritance import ReturnProvider

#全局常量
PI=3.14159265359
EPSILON=1e-6
RAY_EPSILON=1e-3

ZERO_VECTOR3=np.array([0.0,0.0,0.0])
ONE_VECTOR3=np.array([1.0,1.0,1.0])
UP_VECTOR3=np.array([0.0,1.0,0.0])
DOWN_VECTOR3=np.array([0.0,-1,0.0])
LEFT_VECTOR3=np.array([-1.0,0.0,0.0])
RIGHT_VECTOR3=np.array([1.0,0.0,0.0])
FORWARD_VECTOR3=np.array([0.0,0.0,-1.0])
BACK_VECTOR3=np.array([0.0,0.0,1.0])

#Material Types (漫反射表面, 镜面反射表面, 折射表面)
DIFF=0
SPEC=1
REFR=2

class Material:
    refl=int(0)
    emission=ZERO_VECTOR3
    color=ZERO_VECTOR3
    ior=0.0

    def __init__(self,refl=0,emission=ZERO_VECTOR3,color=ZERO_VECTOR3,ior=0.0):
        self.refl=refl
        self.emission=emission
        self.color=color
        self.ior=ior

class Sphere:
    radius=0.0
    pos=ZERO_VECTOR3
    mat=Material()

    def __init__(self,radius=0.0,pos=ZERO_VECTOR3,mat=Material()):
        self.radius=radius
        self.pos=pos
        self.mat=mat

class Plane:
    pos=ZERO_VECTOR3
    normal=ZERO_VECTOR3
    mat=Material()

    def __init__(self,pos=ZERO_VECTOR3,normal=ZERO_VECTOR3,mat=Material()):
        self.pos=pos
        self.normal=normal
        self.mat=mat


class Ray:
    origin=ZERO_VECTOR3
    dir=ZERO_VECTOR3

    def __init__(self,origin=ZERO_VECTOR3,dir=ZERO_VECTOR3):
        self.origin=origin
        self.dir=dir

    def IntersectWithSphere(self,sphere):
        op=sphere.pos-self.origin
        b=np.dot(op,self.dir)

        delta=b*b-np.dot(op,op)+sphere.radius*sphere._radius
        if(delta<0):
            return int(0)
        else:
            delta=sqrt(delta)
        
        distance=b-delta
        if (distance)>EPSILON:
            return distance
        elif (b+delta)>EPSILON:
            return (b+delta)
        else:
            return distance
    
    def IntersectWithPlane(self,plane):
        t=np.dot((plane.pos-self.origin),plane.normal)/np.dot(self.dir,plane.normal)

        if t>EPSILON:
            return t
        else:
            return 0        
