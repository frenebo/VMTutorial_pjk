/*!
 * \file force_compute.cpp
 * \author Rastko Sknepnek, sknepnek@gmail.com
 * \date 30-Nov-2023
 * \brief ForceCompute class
 */

#include "force_compute.hpp"

#include <stdexcept>
// #include <

#include "forces/force_area.hpp"
#include "forces/force_perimeter.hpp"
#include "forces/force_const_vertex_propulsion.hpp"
#include "forces/force_efield_on_cell_boundary_uniform.hpp"
#include "forces/force_efield_on_cell_boundary_pixelated.hpp"



using std::runtime_error;
using std::cout;
using std::endl;
using std::map;
using std::string;

namespace VMSim
{
	
	void ForceCompute::start_force_compute_timers(bool verbose)
	{
		if (verbose)
		{
			cout << "ForceCompute::start_force_compute_timers - clearing and setting timers to zero" << endl;
		}
		_force_timers.clear();
		
		for ( const auto &it : factory_map ) {
			_force_timers[it.first] = timer_duration::zero();
		}
		if (verbose)
		{
			cout << "  finished initializing timers" << endl;
		}
	}
	
	map<string, double> ForceCompute::get_force_compute_timers_millis(bool verbose)
	{
		if (verbose) {
			cout << "ForceCompute::get_force_compute_timers_millis - reading values in timer map" << endl;
		}
		map<string, double> millis_for_forces;
		
		for (const auto &it : _force_timers) {
			string force_id = it.first;
			timer_duration tot_elapsed = it.second;
			
			std::chrono::duration<double, std::milli> ms_double = tot_elapsed;
			
			millis_for_forces[force_id] = ms_double.count();
		}
		
		return millis_for_forces;
	}
	
	map<string, vector<Vec>> ForceCompute::for_py_get_all_vertex_instantaneous_forces(bool verbose)
	{
		vector<Vec> tot_vertex_forces;
		compute_all_vertex_forces(tot_vertex_forces, verbose);
		
		map<string, vector<Vec>> vtx_forces_by_force_id;
		
		for (auto& fit : this->factory_map)
		{
			string force_id = fit.first;
			auto& force = fit.second;
			
			vector<Vec> component_forces;
			force->compute_all_vertex_forces(component_forces, verbose);
			vtx_forces_by_force_id[force_id] = component_forces;
		}
		
		return vtx_forces_by_force_id;
	}
	
	
	
	void ForceCompute::compute_all_vertex_forces(vector<Vec>& tot_vertex_forces, bool verbose)
	{
		if (verbose) {
			cout << "ForceCompute::compute_and_apply_vertex_force - computing all forces" << endl;
		}
		tot_vertex_forces.clear();
		tot_vertex_forces.resize(_sys.cmesh().cvertices().size(), Vec(0.0,0.0));
		
		bool do_time_computations = (_force_timers.size() > 1);
		
		vector<Vec> tmp_component_forces;
		
		// Vector forces start at zero, we add force as w iterate through forces, vectors, halfedges
		for (auto& fit : this->factory_map)
		{
			string force_id = fit.first;
			if (verbose)
			{
				cout << "  computing force '" << force_id << "'" << endl;
			}
			
			auto& force = fit.second;
			
			time_pt t1;
			time_pt t2;
			bool do_time_this_force = (do_time_computations && _force_timers.count(force_id));
			
			if (do_time_this_force) {
				t1 = std::chrono::high_resolution_clock::now();
			}
			
			// cout << "   ca"
			// vector<Vec>& 
			force->compute_all_vertex_forces(tmp_component_forces, verbose);
			
			size_t vid = 0;
			for (const auto& v : tmp_component_forces)
			{
				tot_vertex_forces.at(vid) += v;
				vid++;
			}
			
			if (do_time_this_force) {
				t2 = std::chrono::high_resolution_clock::now();
				auto elapsed = t2 - t1;
				
				_force_timers.at(force_id) += elapsed;
			}
		}
	}
	
	

	void ForceCompute::set_global_params(
		const string& force_id,
		const map<string, double>& num_params,
		const map<string,string>& str_params,
        const map<string, int>& int_params,
        const map<string, vector<double>> flt_array_params,
		bool verbose
	) {
		if (this->factory_map.find(force_id) != this->factory_map.end()) {
			this->factory_map[force_id]->set_global_params(
				num_params,
				str_params,
				int_params,
				flt_array_params,
				verbose
			);
		} else {
			throw runtime_error("ForceCompute::set_global_params: Force id " + force_id + " has not been added yet, could not set its params");
		}
	}

	
	void ForceCompute::set_face_params_facewise(const string& force_id, const vector<int>& fids, const vector<map<string,double>>& params, bool verbose)
	{
		if (!(fids.size() == params.size())) {
			throw runtime_error(
			"ForceCompute::set_face_params_facewise: For setting face parameters elementwise, number of fids and fparams should be identical."
			);
		}
		
		if (this->factory_map.find(force_id) != this->factory_map.end()) {
			this->factory_map[force_id]->set_face_params_facewise(fids, params, verbose);
		}
		else {
			throw runtime_error("ForceCompute::set_params_facewise: Force id " + force_id + " has not been added.");
		}
	}
	
	void ForceCompute::set_vertex_params_vertexwise(const string& force_id, const vector<int>& vids, const vector< map<string,double> >& params, bool verbose)
	{
		if (!(vids.size() == params.size())) {
			throw runtime_error(
			"ForceCompute::set_vertex_params_vertexwisee: For setting vertex parameters elementwise, number of vids and fparams should by identical."
			);
		}
		
		if (this->factory_map.find(force_id) != this->factory_map.end()) {
			this->factory_map[force_id]->set_vertex_params_vertexwise(vids, params, verbose);
		} else {
			throw runtime_error("ForceCompute::set_vertex_params_vertexwises: Force type " + force_id + " has not been setup in this simulation.");
		}
	}

	void ForceCompute::add_force(const string& force_id, const string& force_type, bool verbose)
	{
		if (verbose) {
			cout << "ForceCompute::add_force - Adding force with id '" << force_id << "' and force type '" << force_type << "'" << endl;
		}
		// Check if this force has already been added.
		if (this->factory_map.find(force_id) != this->factory_map.end()) {
			throw runtime_error("ForceCompute::add_force - Cannot add force with id '" + force_id + "' - already has been added to ForceCompute class.");
		}
		
		if (force_type == "area") {
			this->add<ForceArea,const System&>(force_id, _sys);
		} else if (force_type == "perimeter") {
			this->add<ForcePerimeter,const System&>(force_id, _sys);
		} else if (force_type == "const_vertex_propulsion") {
			this->add<ForceConstVertexPropulsion,const System&>(force_id, _sys);
		} else if (force_type == "force_efield_on_cell_boundary_uniform") {
			this->add<ForceEFieldOnCellBoundary, const System&>(force_id, _sys);
		} else if (force_type == "force_efield_on_cell_boundary_pixelated") {
			this->add<PixelatedElectricStuff::ForceEFieldOnCellBoundPixelated, const System&>(force_id, _sys);
		} else  {
			throw runtime_error("Unknown force name : " + force_id + ".");
		}
	}
      void ForceCompute::delete_force(const std::string& force_id, bool verbose)
	  {
		if (verbose) {
			cout << "ForceCompute::delete_force - deleting force with id " << force_id << endl;
		}
		if (this->factory_map.find(force_id) == this->factory_map.end()) {
			throw runtime_error("ForceCompute::delete_force - cannot delete force with id '" + force_id + "' - does not exist");
		}
		
		this->factory_map.erase(force_id);
	  }
	
	void export_ForceCompute(py::module &m)
	{
		py::class_<ForceCompute>(m, "ForceCompute")
			.def(py::init<System &>())
			.def("set_global_params", &ForceCompute::set_global_params,
				py::arg("force_id"),
				py::arg("num_params"),
				py::arg("str_params"),
				py::arg("int_params"),
				py::arg("flt_array_params"),
				py::arg("verbose")=false
			)
			.def(
				"get_instantaneous_forces", &ForceCompute::for_py_get_all_vertex_instantaneous_forces,
				py::arg("verbose")=false
			)
			.def("set_face_params_facewise", &ForceCompute::set_face_params_facewise,
				py::arg("force_id"), py::arg("face_ids"), py::arg("params"), py::arg("verbose")=false
			)
			.def("set_vertex_params_vertexwise", &ForceCompute::set_vertex_params_vertexwise,
				py::arg("force_id"), py::arg("vertex_ids"), py::arg("params"), py::arg("verbose")=false
			)
			.def("add_force", &ForceCompute::add_force,
				py::arg("force_id"), py::arg("force_type"), py::arg("verbose")=false
			)
			.def("delete_force", &ForceCompute::delete_force,
				py::arg("force_id"), py::arg("verbose")=false
			)
			.def("start_force_compute_timers", &ForceCompute::start_force_compute_timers,
				py::arg("verbose")=false
			)
			.def("get_force_compute_timers_millis", &ForceCompute::get_force_compute_timers_millis,
				py::arg("verbose")=false
			)
			;
	}
}