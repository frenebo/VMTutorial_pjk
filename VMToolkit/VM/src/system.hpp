/*!
 * \file system.hpp
 * \author Rastko Sknepnek, sknepnek@gmail.com
 * \date 30-Nov-2023
 * \brief System class 
 */ 

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <exception>
#include <memory>
#include <cmath>
// #include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/info_parser.hpp>

#include "json.hpp"

#include "rng.hpp"
#include "mesh.hpp"
#include "property.hpp"


using std::string;
using std::map;
// using std::to_string;

namespace pt = boost::property_tree;

namespace VMTutorial
{

  typedef Mesh<Property> MyMesh;
  

  typedef map<string, double> params_type;         // Used when we set a numerical value to a parameter, e.g., kappa = 1.0
  typedef map<string, Vec> vec_params_type;        // Used when we set a vectorial value to a parameter, e.g., n = Vec(1,0)
  typedef map<string, vector<double>> multi_params_type;   // Used when we need to at least two values to set a parameter, e.g., a parameters is drawn from a random distribution 
  typedef map<string, string> string_params_type;         // Used when we set a string value to a parameter, e.g., force_type = "nematic_vm"
  
  bool operator<(const VertexHandle<Property>&, const VertexHandle<Property>&);

  class System
  {
    public:

      System(MyMesh& mesh) : _mesh{mesh}, 
                             _time_step{0},
                             _simulation_time{0.0},
                            //  _num_cell_types{0},
                            //  _num_vert_types{0},
                            //  _num_junction_types{0},
                             _mesh_set{false},
                             _topology_changed{true}
                             {
                             }

      // System setup
      
      void read_input_from_jsonstring(const string& json_contents, bool verbose=false);
      void input_from_jsonobj(nlohmann::json& j, bool verbose);
      
      void set_simulation_time_step(int time_step) { _time_step = time_step; }
      
      void log_debug_stats()
      {
        std::cout << "   System::log_debug_stats:" << std::endl;
        std::cout << "     CURRENT size of _halfedges: " << _mesh.halfedges().size() << std::endl; 
      }
      void set_topology_change(bool flag) { _topology_changed = flag; }

      // System info access 
      MyMesh& mesh()  { return _mesh; }
      const MyMesh& cmesh() const { return _mesh; }
      
      int& time_step() { return _time_step; }
      double& simulation_time() { return _simulation_time; }
      bool topology_change() { return _topology_changed;  }
      
    private:
  
      MyMesh &_mesh;
      int _time_step;
      double _simulation_time;
      bool _mesh_set;
      bool _topology_changed;  // If true, mesh topology has changed

  };

  void export_VertexProperty(py::module&);
  void export_EdgeProperty(py::module&);
  void export_HEProperty(py::module&);
  void export_FaceProperty(py::module&);
  // void export_Spoke(py::module &);
  void export_Vertex(py::module &);
  void export_VertexCirculator(py::module &);
  void export_Edge(py::module&);
  void export_HalfEdge(py::module&);
  void export_Face(py::module&);
  void export_FaceCirculator(py::module &);
  void export_Mesh(py::module&);
  void export_System(py::module&);

}

#endif
