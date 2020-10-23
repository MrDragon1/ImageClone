#ifndef VECTOR3D_H
#define VECTOR3D_H

#include "opencv2//core//core.hpp"

class vector3d
{
public:
    double x,y,z;

    vector3d():x(0),y(0),z(0){}

    vector3d(double xx,double yy,double zz):x(xx),y(yy),z(zz){}

    vector3d(cv::Point &v):x(v.x),y(v.y),z(0){}

    void Set(double xx,double yy,double zz){
        x=xx;y=yy;z=zz;
    }

    vector3d operator + (const vector3d& v) const
    {
        return vector3d(x + v.x, y + v.y, z + v.z);
    }

    vector3d operator - (const vector3d& v) const
    {
        return vector3d(x - v.x, y - v.y, z - v.z);
    }
    //点乘
    double dot(const vector3d& v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }
    //叉乘
    vector3d Cross(const vector3d& v) const
    {
        return vector3d(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }

    void CalPlanePointZ(vector3d A,vector3d B,vector3d C);
};

#endif // VECTOR3D_H
