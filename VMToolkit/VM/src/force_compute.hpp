/*!
 * \file force_compute.hpp
 * \author Rastko Sknepnek, sknepnek@gmail.com
 * \date 30-Nov-2023
 * \brief ForceCompute class 
 */ 

#ifndef __FORCE_COMPUTE_HPP__
#define __FORCE_COMPUTE_HPP__

#include <exception>
#include <algorithm>
#include <map>
#include <string>

#include "system.hpp"
#include "class_factory.hpp"
#include "force.hpp"
#include "force_area.hpp"
#include "force_perimeter.hpp"
#include "force_const_vertex_propulsion.hpp"
// #include "force_self_propulsion.hpp"

 
using std::runtime_error;
using std::transform;
using std::map;
using std::string;


namespace VMTutorial
{

  class ForceCompute : public ClassFactory<Force>
  {
    public:

      ForceCompute(System& sys) : _sys{sys}
      { 

      }

      ~ForceCompute() = default; 

      ForceCompute(const ForceCompute&) = delete;

      // void compute_forces()
      // {
      //   for (auto v : _sys.mesh().vertices())
      //     if (!v.erased)
      //       this->compute(v);
      // }
      
      // @TODO remove this from stuff - does compute change the force value or not?
      // i might accidentally use the wrong one and change it twice at some point...
      void compute(Vertex<Property> &v, bool verbose=false)
      {
        if (verbose)
        {
          cout << "ForceCompute::compute(vertex) - Computing force on vector... " << v.id << endl;
        }
        v.data().force = Vec(0.0,0.0);
        for (auto he : v.circulator()) {
          v.data().force += this->compute(v, he, verbose);
        }
        if (verbose)
        {
          cout << "  TOTAL force on vector " << v.id << ": " << v.data().force.x << ", " << v.data().force.y << endl;
        }
      }

      Vec compute(Vertex<Property> &v, const HalfEdge<Property> &he, bool verbose=false)
      {
        
        if (verbose)
        {
          cout << "ForceCompute::compute(vertex, he) - computing force for vertex " << v.id << " half edge " << he.idx() << endl;
        }
        Vec ftot(0,0);
        for (auto& f : this->factory_map) {
          ftot += f.second->compute(v, he, verbose);
        }
        return ftot;
      }

      double tension(HalfEdge<Property>& he)
      {
        double T = 0.0;
        for (auto& f : this->factory_map)
          T += f.second->tension(he);
        return T;
      }

      // double energy(const Face<Property>& face)
      // {
      //   double E = 0.0;
      //   for (auto& f : this->factory_map)
      //     E += f.second->energy(face);
      //   return E;
      // }

      // double total_energy()
      // {
      //   double E = 0.0;
      //   for (auto f : _sys.mesh().faces())  
      //     E += this->energy(f);
      //   return E;
      // }

      // void set_params(const string& fname, const string& type, const params_type& params)
      // {
      //   if (this->factory_map.find(fname) != this->factory_map.end())
      //     this->factory_map[fname]->set_params(type, params);
      //   else
      //     throw runtime_error("set_params: Force type " + fname + " is not used in this simulation.");
      // }
      
      void set_face_params_facewise(const string& fname, const vector<int>& fids, const vector<params_type>& params, bool verbose)
      {
        if (!(fids.size() == params.size())) {
          throw runtime_error(
            "ForceCompute::set_face_params_facewise: For setting face parameters elementwise, number of fids and fparams should be identical."
          );
        }
        
        if (this->factory_map.find(fname) != this->factory_map.end()) {
            this->factory_map[fname]->set_face_params_facewise(fids, params, verbose);
        }
        else {
          throw runtime_error("ForceCompute::set_params_facewise: Force type " + fname + " is not used in this simulation.");
        }
      }
      
      void set_vertex_params_vertexwise(const string& fname, const vector<int>& vids, const vector<params_type>& params, bool verbose)
      {
        if (!(vids.size() == params.size())) {
          throw runtime_error(
            "ForceCompute::set_vertex_params_vertexwisee: For setting vertex parameters elementwise, number of vids and fparams should by identical."
          );
        }
        
        if (this->factory_map.find(fname) != this->factory_map.end()) {
          this->factory_map[fname]->set_vertex_params_vertexwise(vids, params, verbose);
        } else {
          throw runtime_error("ForceCompute::set_vertex_params_vertexwises: Force type " + fname + " has not been setup in this simulation.");
        }
      }

      void add_force(const string& fname)
      {
        // Check if this force has already been added.
        if (this->factory_map.find(fname) != this->factory_map.end()) {
          throw runtime_error("ForceCompute::add_force - Cannot add force '" + fname + "' - already has been added to ForceCompute class.");
        }
        
        if (fname == "area") {
          this->add<ForceArea,System&>(fname, _sys);
        } else if (fname == "perimeter") {
          this->add<ForcePerimeter,System&>(fname, _sys);
        } else if (fname == "const_vertex_propulsion") {
          this->add<ForceConstVertexPropulsion,System&>(fname, _sys);
        } else  {
          throw runtime_error("Unknown force name : " + fname + ".");
        }
      }

    private: 

      System& _sys;
 
  };

  void export_ForceCompute(py::module&);

}
#endif
