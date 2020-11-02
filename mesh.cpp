#include "mesh.h"

/*处理ROI中的顶点，生成三角形数组，保存在成员变量中*/
Mesh::Mesh(std::vector<CDTPoint> ROI)
{
    using namespace std;

    DelaunayTriangulation* adaptiveMesh = new DelaunayTriangulation();

    adaptiveMesh->clear();
    adaptiveMesh->insert(ROI.begin(), ROI.end());
    for (unsigned int i=0 ; i < ROI.size() ; ++i) {
        CDTPoint curr = ROI[i];
        CDTPoint next = ROI[(i+1) % ROI.size()];
        adaptiveMesh->insert_constraint(curr,next);
    }

    cout << "Number of vertices: " << adaptiveMesh->number_of_vertices() <<endl;
    cout << "Meshing the triangulation..." << endl;

    Mesher mesher(*adaptiveMesh);
    Criteria params(0.05,0);
    mesher.set_criteria(params);
    mesher.refine_mesh();
    std::cout << "Number of vertices: " << adaptiveMesh->number_of_vertices() << std::endl;
    std::cout << "Number of finite faces: " << adaptiveMesh->number_of_faces() << std::endl;

    for(CDT::Vertex_iterator vit = adaptiveMesh->vertices_begin(); vit!= adaptiveMesh->vertices_end(); ++vit)
    {
        //DTPoint tmp((float)vit->point().x(), (float)vit->point().y());
        vertex_map.insert(make_pair(vit->point(),vertex_list.size()));
        vertex_list.push_back(vit->point());
    }
    tagFaces (adaptiveMesh);
    int mesh_faces_counter = 0;
    for(CDT::Finite_faces_iterator fit = adaptiveMesh->finite_faces_begin(); fit != adaptiveMesh->finite_faces_end(); ++fit)
    {
        if(fit->is_in_domain()) {
            vector<size_t> index(3);
            for(size_t i = 0; i<3; i++)
            {
                auto iter = vertex_map.find(DTPoint(fit->vertex(i)->point().x(), fit->vertex(i)->point().y()));
                if(iter == vertex_map.end())
                {
                    continue;
                }
                index[i] = iter->second;
            }
            face_vertex_index.push_back(index);
            ++mesh_faces_counter;
        }
    }
    std::cout << "Number of faces in the mesh domain: " << mesh_faces_counter << std::endl;

}

void Mesh::tagFaces(DelaunayTriangulation* adaptiveMesh) const {

    // implement BFS to go over the mesh faces
    std::stack<Face> stack;
    std::set<Face*> visitedFaces; // a set of faces we've already visited, in order to avoid revisiting them

    // push all the infinite faces (which are made of the infinite vertex + convex hull edge) to the stack with tag FALSE
    FacesCirculator fc = adaptiveMesh->incident_faces(adaptiveMesh->infinite_vertex());
    FacesCirculator done(fc);
    do {
        fc->set_in_domain(false);
        stack.push(*fc);
        visitedFaces.insert(&(*fc));
        fc++;
    } while (done != fc);

    while (!stack.empty()) {
        Face currFace = stack.top();
        stack.pop();
        visitedFaces.insert(&currFace);
        // now, go over all the incident faces to this face
        for (int i=0 ; i<3 ; ++i) {
            FaceHandle fh = currFace.neighbor(i);
            // if we haven't treated this face already
            if (visitedFaces.find(&(*fh)) == visitedFaces.end()) {
                // no matter what - we should push fh to the stack
                stack.push(*fh);
                visitedFaces.insert(&(*fh));
                // an edge is simply a pair of face+number according to the neighbor
                // if the corresponding edge is constrained - we should flip the tag for fh, else - retain the same tag

                // we need to find the correct edge which is actually Edge(currFace,i)
                int j;
                for (j=0 ; j<3 ; ++j) {
                    // for which j is fh.neighbor(j) == currFace
                    if (equals(*(fh->neighbor(j)),currFace)) break;
                }
                assert(j<3);
                if (adaptiveMesh->is_constrained(Edge(fh,j))) {
                    // switch tag for fh
                    fh->set_in_domain(!currFace.is_in_domain());
                } else {
                    fh->set_in_domain(currFace.is_in_domain());
                }
            }
        }
    }
}

bool Mesh::equals(Face& f1, Face& f2) const {
    // two faces are equal if they have the same vertices in the same order
    for (int i=0 ; i<3 ; ++i) {
        VertexHandle v1 = f1.vertex(i);
        VertexHandle v2 = f2.vertex(i);
        if (v1->point().x() != v2->point().x()) return false;
        if (v1->point().y() != v2->point().y()) return false;
    }
    return true;
}

