from math import sqrt
import Vector as vector

#全局常量
EPSILON=1e-6
RAY_EPSILON=1e-3


#Material Types (漫反射表面, 镜面反射表面, 折射表面)
DIFF=0
SPEC=1
REFR=2

class Material:
    refl=int(0)
    emission=vector.ZERO_VECTOR3
    color=vector.ZERO_VECTOR3
    ior=0.0

    def __init__(self,refl=0,emission=vector.ZERO_VECTOR3,color=vector.ZERO_VECTOR3,ior=0.0):
        self.refl=refl
        self.emission=emission
        self.color=color
        self.ior=ior

class Sphere:
    radius=0.0
    pos=vector.ZERO_VECTOR3
    mat=Material()

    def __init__(self,radius=0.0,pos=vector.ZERO_VECTOR3,mat=Material()):
        self.radius=radius
        self.pos=pos
        self.mat=mat

class Plane:
    pos=vector.ZERO_VECTOR3
    normal=vector.ZERO_VECTOR3
    mat=Material()

    def __init__(self,pos=vector.ZERO_VECTOR3,normal=vector.ZERO_VECTOR3,mat=Material()):
        self.pos=pos
        self.normal=normal
        self.mat=mat


class Ray:
    origin=vector.ZERO_VECTOR3
    dir=vector.ZERO_VECTOR3

    def __init__(self,origin=vector.ZERO_VECTOR3,dir=vector.ZERO_VECTOR3):
        self.origin=origin
        self.dir=dir

    def IntersectWithSphere(self,sphere):
        op=sphere.pos.MinusVector3(self.origin)
        b=vector.vector3_instance.Dot(op,self.dir)

        delta=b*b-vector.vector3_instance.Dot(op,op)+sphere.radius*sphere.radius
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
        t=vector.vector3_instance.Dot((plane.pos.MinusVector3(self.origin)),plane.normal)/vector.vector3_instance.Dot(self.dir,plane.normal)
        
        if t>EPSILON:
            return t
        else:
            return 0        


print(vector.FORWARD_VECTOR3.x)
print(vector.FORWARD_VECTOR3.y)
print(vector.FORWARD_VECTOR3.z)
c=vector.FORWARD_VECTOR3.MinusVector3(vector.BACK_VECTOR3)
print(c.x)
print(c.y)
print(c.z)