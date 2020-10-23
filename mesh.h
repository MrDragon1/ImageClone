#ifndef MESH_H
#define MESH_H

#include <vector>
#include <QPoint>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_mesher_2.h>
#include <CGAL/Delaunay_mesh_face_base_2.h>
#include <CGAL/Delaunay_mesh_size_criteria_2.h>


struct Point{
    float x, y;
    Point(float xx,float yy):x(xx),y(yy){}

    Point():x(0),y(0){}

    bool operator < (const Point& v) const
    {
        if(x < v.x && y < v.y)
        {
            return true;
        }
        return false;
    }

    Point operator - (const Point& v) const
    {
        return Point(x - v.x, y - v.y);
    }

    Point operator + (const Point& v) const
    {
        return Point(x + v.x, y + v.y);
    }

    double Mod() const
    {
        return sqrt(x * x + y * y);
    }
    double Dot(const Point& v) const
    {
        return x * v.x + y * v.y;
    }
    bool Equel(const Point& v) const
        {
            if(abs(x-v.x) < 0.000001 && abs(y-v.y)< 0.000001)
            {
                return true;
            }
            return false;
        }

};

typedef struct {
    float x, y ;
} TexCoords;

typedef struct {
    int p1, p2, p3;
} Triangles;


typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Triangulation_vertex_base_2<K> Vb;
typedef CGAL::Delaunay_mesh_face_base_2<K> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb> Tds;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, Tds> CDT;
typedef CGAL::Delaunay_mesh_size_criteria_2<CDT> Criteria;
typedef CGAL::Delaunay_mesher_2<CDT, Criteria> Mesher;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, Tds> DelaunayTriangulation;
typedef CDT::Vertex_handle Vertex_handle;
typedef CDT::Point CDTPoint;
typedef DelaunayTriangulation::Face_circulator FacesCirculator;
typedef DelaunayTriangulation::Point_iterator PointIterator;
typedef DelaunayTriangulation::Vertex Vertex;
typedef DelaunayTriangulation::Vertex_handle VertexHandle;
typedef DelaunayTriangulation::Edge Edge;
typedef DelaunayTriangulation::Face Face;
typedef DelaunayTriangulation::Face_handle FaceHandle;
typedef DelaunayTriangulation::Point DTPoint;

class Mesh
{
public:
    Mesh(std::vector<CDTPoint> ROI);

    void tagFaces(DelaunayTriangulation* adaptiveMesh) const;

    bool equals(Face& f1, Face& f2) const;
    std::vector<DTPoint> vertex_list;//保存三角形的顶点
    std::map<DTPoint, size_t> vertex_map;//保存顶点与在vertex_list中下标的映射关系
    std::vector<std::vector<size_t>> face_vertex_index;//保存各个三角形的顶点在vertex_list的下标

};

#endif // MESH_H
