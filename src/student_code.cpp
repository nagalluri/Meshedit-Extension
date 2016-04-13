/*
 * Student solution for UC Berkeley Project 2
 *
 * Implemented by Tim Greeno on 2/2016.
 *
 */

#include "student_code.h"
#include "mutablePriorityQueue.h"

namespace CGL {

    void BezierPatch::preprocess() {
        // TODO Part 1.
        // TODO If you use the matrix form for Bezier patch evaluation, you will need to
        // TODO compute your matrices based on the 16 control points here. 
        // TODO You will also need to define your matrices
        // TODO as member variables in the "BezierPatch" class.
        // TODO If you use De Casteljau's recursive algorithm, you will not need to do anything here.

        double data[16] = {1.0,0.0,0.0,0.0,-3.0,3.0,0.0,0.0,3.0,-6.0,3.0,0.0,-1.0,3.0,-3.0,1.0};
        double xdata[16];
        double ydata[16];
        double zdata[16];

        Matrix4x4 left = Matrix4x4(data);
        Matrix4x4 right = left.T();

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                xdata[i*4 + j] = controlPoints[i][j][0];
                ydata[i*4 + j] = controlPoints[i][j][1];
                zdata[i*4 + j] = controlPoints[i][j][2];
            }
        }
        X = Matrix4x4(xdata);
        Y = Matrix4x4(ydata);
        Z = Matrix4x4(zdata);

        X = left * X;
        X = X * right;
        Y = left * Y;
        Y = Y * right;
        Z = left * Z;
        Z = Z * right;
    }

    Vector3D BezierPatch::evaluate(double u, double v) const {
        // TODO Part 1.
        // TODO Returns the 3D point whose parametric coordinates are (u, v) on the Bezier patch.
        // TODO Note that both u and v are within [0, 1]. 
        
        Vector4D U = Vector4D(1.0, u, pow(u, 2.0), pow(u, 3.0));
        Vector4D V = Vector4D(1.0, v, pow(v, 2.0), pow(v, 3.0));

        double xval = dot(U,X*V);
        double yval = dot(U,Y*V);
        double zval = dot(U,Z*V);
        return Vector3D(xval,yval,zval);
    }

    void BezierPatch::add2mesh(Polymesh* mesh) const {
        // TODO Part 1.
        // TODO Tessellate the given Bezier patch into triangles uniformly on a 8x8 grid(8x8x2=128 triangles) in parameter space.
        // TODO You will call your own evaluate function here to compute vertex positions of the tessellated triangles.
        // TODO The "addTriangle" function inherited from the "BezierPatchLoader" class may help you add triangles to the output mesh. 
        for (double u = 0.0; u < 1.00; u += .125) {
            for (double v = 0.0; v < 1.00; v += .125) {
                Vector3D v0 = evaluate(u, v);
                Vector3D v1 = evaluate(u+.125,v+.125);
                Vector3D v2 = evaluate(u,v+.125);
                addTriangle(mesh,v0,v1,v2);
                v2 = evaluate(u+.125,v);
                addTriangle(mesh,v0,v2,v1);
            }
        }
    }

    Vector3D Vertex::normal(void) const
    // TODO Part 2.
    // TODO Returns an approximate unit normal at this vertex, computed by
    // TODO taking the area-weighted average of the normals of neighboring
    // TODO triangles, then normalizing.
    {
        // TODO Compute and return the area-weighted unit normal.
        Vector3D n(0,0,0); 
        HalfedgeCIter h = halfedge(); 
        h = h->twin(); 

        HalfedgeCIter horig = h;
        do {
            Vector3D v0 = h->vertex()->position;
            h = h->next();
            Vector3D v1 = h->vertex()->position;
            h = h->twin();
            Vector3D v2 = h->vertex()->position;

            n = n + cross(v1-v0,v2-v1);

        } while (h != horig);
        return n.unit();
    }

    EdgeIter HalfedgeMesh::flipEdge(EdgeIter e0) {
        // TODO Part 3.
        // TODO This method should flip the given edge and return an iterator to the flipped edge.
        HalfedgeIter h1 = e0->halfedge();
        HalfedgeIter t1 = h1->twin();

        if (e0->isBoundary()) {
            return e0;
        }

        HalfedgeIter h2 = h1->next();
        HalfedgeIter t2 = t1->next();        
        HalfedgeIter h3 = h2->next();
        HalfedgeIter t3 = t2->next();        

        VertexIter v0 = h1->vertex();
        VertexIter v2 = t1->vertex();
        VertexIter v1 = h3->vertex();
        VertexIter v3 = t3->vertex();

        FaceIter f1 = h1->face();
        FaceIter f2 = t1->face();

        h1->setNeighbors(t3,t1,v1,e0,f1);
        h2->setNeighbors(h1,h2->twin(),v2,h2->edge(),f1);
        h3->setNeighbors(t2,h3->twin(),v1,h3->edge(),f2);        
        t1->setNeighbors(h3,h1,v3,e0,f2);
        t2->setNeighbors(t1,t2->twin(),v0,t2->edge(),f2);
        t3->setNeighbors(h2,t3->twin(),v3,t3->edge(),f1);

        f1->halfedge() = h1;
        f2->halfedge() = t1;
        v0->halfedge() = t2;
        v1->halfedge() = h3;
        v2->halfedge() = h2;
        v3->halfedge() = t1;
        return e0;
    }

    VertexIter HalfedgeMesh::splitEdge(EdgeIter e0) {
        // TODO Part 4.
        // TODO This method should split the given edge and return an iterator to the newly inserted vertex.
        // TODO The halfedge of this vertex should point along the edge that was split, rather than the new edges.
        if (e0->isBoundary()) {
            return VertexIter();
        }
        HalfedgeIter h1 = e0->halfedge();
        HalfedgeIter t1 = h1->twin();
        HalfedgeIter h2 = h1->next();
        HalfedgeIter t2 = t1->next();        
        HalfedgeIter h3 = h2->next();
        HalfedgeIter t3 = t2->next();        
        VertexIter v0 = h1->vertex();
        VertexIter v2 = t1->vertex();
        VertexIter v1 = h3->vertex();
        VertexIter v3 = t3->vertex();
        FaceIter f1 = h1->face();
        FaceIter f2 = t1->face();

        VertexIter v4 = newVertex();
        Vector3D pos = ((v0->position) + (v2->position)) * .50;
        v4->position = pos;

        HalfedgeIter h4 = newHalfedge();
        HalfedgeIter h5 = newHalfedge();
        HalfedgeIter h6 = newHalfedge();
        HalfedgeIter t4 = newHalfedge();
        HalfedgeIter t5 = newHalfedge();
        HalfedgeIter t6 = newHalfedge();
        EdgeIter e1 = newEdge();
        EdgeIter e2 = newEdge();
        EdgeIter e3 = newEdge();
        FaceIter f3 = newFace();
        FaceIter f4 = newFace();

        h1->setNeighbors(h4,t6,v0,e0,f1);
        h2->setNeighbors(h5,h2->twin(),v2,h2->edge(),f3);
        h3->setNeighbors(h1,h3->twin(),v1,h3->edge(),f1);        
        h4->setNeighbors(h3,h5,v4,e1,f1);        
        h5->setNeighbors(h6,h4,v1,e1,f3);        
        h6->setNeighbors(h2,t1,v4,e2,f3);

        t1->setNeighbors(t4,h6,v2,e2,f2);
        t2->setNeighbors(t5,t2->twin(),v0,t2->edge(),f4);
        t3->setNeighbors(t1,t3->twin(),v3,t3->edge(),f2);        
        t4->setNeighbors(t3,t5,v4,e3,f2);        
        t5->setNeighbors(t6,t4,v3,e3,f4);        
        t6->setNeighbors(t2,h1,v4,e0,f4);  

        f1->halfedge() = h1;
        f2->halfedge() = t1;
        f3->halfedge() = h5;
        f4->halfedge() = t5;

        v0->halfedge() = t2;
        v1->halfedge() = h3;
        v2->halfedge() = h2;
        v3->halfedge() = t3; 
        v4->halfedge() = h6; 

        e0->halfedge() = h1;       
        e1->halfedge() = h5;       
        e2->halfedge() = t1;       
        e3->halfedge() = t5;       

        return v4;
    }

    void MeshResampler::upsample(HalfedgeMesh& mesh)
    // TODO Part 5.
    // This routine should increase the number of triangles in the mesh using Loop subdivision.
    {
        // Each vertex and edge of the original surface can be associated with a vertex in the new (subdivided) surface.
        // Therefore, our strategy for computing the subdivided vertex locations is to *first* compute the new positions
        // using the connectity of the original (coarse) mesh; navigating this mesh will be much easier than navigating
        // the new subdivided (fine) mesh, which has more elements to traverse.  We will then assign vertex positions in
        // the new mesh based on the values we computed for the original mesh.


        // TODO Compute new positions for all the vertices in the input mesh, using the Loop subdivision rule,
        // TODO and store them in Vertex::newPosition. At this point, we also want to mark each vertex as being
        // TODO a vertex of the original mesh.
        for (VertexIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++) {
            v->isNew = false;
            float n = (float) v->degree();
            float u = (n == 3.0) ? 3.0 / 16.0 : 3.0 / (8.0 * (float) n);
            
            Vector3D sum = Vector3D(0,0,0);
            HalfedgeIter h1 = v->halfedge();
            HalfedgeIter h = v->halfedge();
            VertexIter curr;
            do {
                HalfedgeIter h_twin = h->twin(); 
                VertexIter curr = h_twin->vertex();
                sum = sum + curr->position;  
                h = h_twin->next();         
            } while(h != h1); 

            v->newPosition = (1.0 - n * u) * v->position + (u * sum);
        }

        // TODO Next, compute the updated vertex positions associated with edges, and store it in Edge::newPosition.
        for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++) {
            HalfedgeIter h1 = e->halfedge();
            HalfedgeIter t1 = h1->twin();

            HalfedgeIter h3 = h1->next()->next();
            HalfedgeIter t3 = t1->next()->next();
        
            VertexIter v0 = h1->vertex();
            VertexIter v2 = t1->vertex();
            VertexIter v1 = h3->vertex();
            VertexIter v3 = t3->vertex();

            e->newPosition = (3.0/8.0) * (v0->position + v2->position) + (1.0 / 8.0) * (v1->position + v3->position);

            e->isNew = false;
        }

        // TODO Next, we're going to split every edge in the mesh, in any order.  For future
        // TODO reference, we're also going to store some information about which subdivided
        // TODO edges come from splitting an edge in the original mesh, and which edges are new,
        // TODO by setting the flat Edge::isNew.  Note that in this loop, we only want to iterate
        // TODO over edges of the original mesh---otherwise, we'll end up splitting edges that we
        // TODO just split (and the loop will never end!)


        EdgeIter last_edge;
        for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++) {
            last_edge = e;
        }
        for (EdgeIter e = mesh.edgesBegin(); e != last_edge; e++) {
                VertexIter newv = mesh.splitEdge(e);
                newv->isNew = true;
                newv->newPosition = e->newPosition;

                HalfedgeIter h6 = newv->halfedge();

                h6->edge()->isNew = false;

                h6->twin()->next()->twin()->next()->edge()->isNew = false;

                h6->twin()->next()->edge()->isNew = true;

                h6->next()->next()->edge()->isNew = true;

        }
         if(last_edge->isNew == false) {
                VertexIter newv = mesh.splitEdge(last_edge);
                newv->isNew = true;
                newv->newPosition = last_edge->newPosition;

                HalfedgeIter h1 = newv->halfedge();

                h1->edge()->isNew = false;

                h1->twin()->next()->twin()->next()->edge()->isNew = false;

                h1->twin()->next()->edge()->isNew = true;

                h1->next()->next()->edge()->isNew = true;

             }



        // TODO Now flip any new edge that connects an old and new vertex.
        for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++) {
            if (e->isNew == true) {
                HalfedgeIter h1 = e->halfedge();
                HalfedgeIter t1 = h1->twin();
            
                VertexIter v0 = h1->vertex();
                VertexIter v2 = t1->vertex();
                if((v0->isNew == false && v2->isNew == true) || (v0->isNew == true && v2->isNew == false)) {
                    mesh.flipEdge(e);
                }
            }
        }

        // TODO Finally, copy the new vertex positions into final Vertex::position.
        for (VertexIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++) {
            v->position = v->newPosition;
        }
    }

    // TODO Part 6.
    // TODO There's also some code you'll need to complete in "Shader/frag" file.

}