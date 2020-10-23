#include "vector3d.h"


void vector3d::CalPlanePointZ(vector3d v1,vector3d v2,vector3d v3)
{
    vector3d vn;
    double na = (v2.y - v1.y)*(v3.z - v1.z) - (v2.z - v1.z)*(v3.y - v1.y);
    double nb = (v2.z - v1.z)*(v3.x - v1.x) - (v2.x - v1.x)*(v3.z - v1.z);
    double nc = (v2.x - v1.x)*(v3.y - v1.y) - (v2.y - v1.y)*(v3.x - v1.x);

    //平面法向量
    vn.Set(na, nb, nc);

    if (vn.z != 0)				//如果平面平行Z轴
    {
        this->z = v1.z - (vn.x * (this->x - v1.x) + vn.y * (this->y - v1.y)) / vn.z;			//点法式求解
    }
}
