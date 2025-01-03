
/*!
 * \file mesh.hpp
 * \author Rastko Sknepnek, sknepnek@gmail.com
 * \date 09-Jun-2017
 * \brief Mesh class
 */

#ifndef __MESH_HPP__
#define __MESH_HPP__

#include <vector>
#include <iostream>


#include "types.hpp"
#include "type_circulators.hpp"

using std::runtime_error;
using std::vector;

namespace VMTutorial
{

	class Mesh
	{
	public:
		Mesh() {}

		//! Mesh setup functions
		void add_vertex(const Vertex &v)
		{
			_vertices.push_back(v);
		}
		void add_edge(const Edge &e) { _edges.push_back(e); }
		void add_halfedge(const HalfEdge &he) { _halfedges.push_back(he); }
		void add_face(int face_id, const vector<int> &vert_ids, bool verbose=false);


		void wipe()
		{
			_halfedges.clear();
			_vertices.clear();
			_edges.clear();
			_faces.clear();
		}

		// accessor functions
		HEHandle get_mesh_he(int);
		VertexHandle get_mesh_vertex(int);
		EdgeHandle get_mesh_edge(int);
		FaceHandle get_mesh_face(int);

		Vertex &get_vertex(int);
		HalfEdge &get_halfedge(int);
		Edge &get_edge(int);
		Face &get_face(int);

		vector<HalfEdge> &halfedges() { return _halfedges; }
		vector<Vertex> &vertices() { return _vertices; }
		const vector<Vertex> &cvertices() const { return _vertices; }
		vector<Edge> &edges() { return _edges; }
		const vector<Edge> &cedges() const { return _edges; }
		vector<Face> &faces() { return _faces; }
		const vector<Face> &cfaces() const { return _faces; }

		int num_vert() const { return _vertices.size(); }
		int num_faces() const { return _faces.size(); }

		// mesh manipulation functions
		bool T1(Edge &, double);

		// mesh info functions
		vector<vector<double>> get_vertex_positions() const
		{
			vector<vector<double>> vpositions(num_vert(), vector<double>{0.0,0.0});
			
			size_t vidx = 0;
			for (const auto& v : _vertices)
			{
				vpositions[vidx][0] = v.data().r.x;
				vpositions[vidx][1] = v.data().r.y;
				
				vidx++;
			}
			
			return vpositions;
		}

		double area(const Face &) const;
		double perim(const Face &) const;
		double len(const Edge &);
		int coordination(const Vertex &);
		int face_sides(const Face &);
		bool is_boundary_face(const Face &);

		void tidyup();	  // get the mesh in order (set boundary edges, outer faces, etc. )
		Vec get_centre(); // compute geometric centre of the mesh
		Vec get_face_centre(const Face &);
		Vec get_face_centroid(const Face &);

	private:
		std::vector<HalfEdge> _halfedges;
		std::vector<Vertex> _vertices;
		std::vector<Edge> _edges;
		std::vector<Face> _faces;
	};

};

#endif
