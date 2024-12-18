/*!
 * \file force_compute.cpp
 * \author Rastko Sknepnek, sknepnek@gmail.com
 * \date 30-Nov-2023
 * \brief ForceCompute class
 */

#include "force_compute.hpp"

namespace VMTutorial
{
	void export_ForceCompute(py::module &m)
	{
		py::class_<ForceCompute>(m, "ForceCompute")
			.def(py::init<System &>())
			// .def("set_params", &ForceCompute::set_params)
			// .def("set_vec_params", &ForceCompute::set_vec_params)
			.def("set_face_params_facewise", &ForceCompute::set_face_params_facewise)
			.def("set_vertex_params_vertexwise", &ForceCompute::set_vertex_params_vertexwise)
			// .def("set_flag", &ForceCompute::set_flag)
			.def("add", &ForceCompute::add_force);
			// .def("compute", &ForceCompute::compute_forces)
			// .def("energy", &ForceCompute::total_energy);
	}
}