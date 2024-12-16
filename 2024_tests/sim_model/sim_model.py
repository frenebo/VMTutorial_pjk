import json
from VMToolkit.VM import Tissue, System, ForceCompute, Integrate, Topology, Dump, Simulation, Vec


class VMToolkitWrapper:
    def __init__(self, verbose=False):
        self._sim_sys = None
        self._forces = None
        self._integrators = None
        self._dumps = None
        self._simulation = None
        
        
        self._initialize_cpp(verbose=verbose)
        
        self.forces_configured = False
        self._tissue_loaded = False

    
    def _initialize_cpp(self, verbose=False):
        ##### Running sim
        tissue  = Tissue()                                               # initialise mesh
        sim_sys = System(tissue)                                         # base object for the system
        if verbose:
            print("constructing force")
            sim_sys.log_debug_stats()
        forces = ForceCompute(sim_sys)                                          # handles all types of forces
        if verbose:
            print("constructing integrators")
            sim_sys.log_debug_stats()
        integrators = Integrate(sim_sys, forces, 0)              # handles all integrators
        if verbose:
            print("constructing topology")
            sim_sys.log_debug_stats()
        topology = Topology(sim_sys, forces)                             # handles all topology changes (T1, division, ingression)
        if verbose:
            print("constructing dumps")
            sim_sys.log_debug_stats()
        dumps = Dump(sim_sys, forces)                                    # handles all data output 
        if verbose:
            print("constructing simulation")
            sim_sys.log_debug_stats()
        simulation = Simulation(sim_sys, integrators, forces, topology)  # simulation object                      # handles all data output 
        
        
        """
        IMPORTANT! All these need to be held as CLASS variables python, otherwise their memory will be freed,
        and they'll try to talk to each other, and just read whatever is in that memory now. Will cause problems
        """
        self._tissue = tissue
        self._sim_sys = sim_sys
        self._forces = forces
        self._integrators = integrators
        self._topology = topology
        self._dumps = dumps
        self._simulation = simulation
        
        if verbose:
            print("Finished intializing cpp...")
            sim_sys.log_debug_stats()
        
    def dump_cpp_json(self):
        return self._dumps.mesh_to_jsonstr()

    def configure_forces(
        self,
        P0_model,
        gamma,
        kappa,
        verbose=False,
        ):
        # if not self._cell_types_configured:
        #     raise Exception("Cell type not configured - forces won't work without cell types preestablished")
        # if not self._cell_types_
        if verbose:
            print("CONFIGURING FORCES==================")
            self._sim_sys.log_debug_stats()
        
        self._forces.add('area')         # add area force form term E = 0.5*kappa*(A-A0)^2
        self._forces.add('perimeter')    # add perimeter force term from E = 0.5*gamma*P^2 + lambda*P (maybe -?)
        if verbose:
            print("Added area and perimeter forces")
            self._sim_sys.log_debug_stats()
        # Set parameters for each cell type

        # lambda_val = P0_model * gamma # @TODO either the sim uses this, or it uses the P0... add a way to force P0 usage in set_params in cpp file?
        self._default_gamma = gamma
        self._default_lambda_val = P0_model * gamma
        self._default_kappa = kappa


        self.forces_configured = True

    def configure_integrators(self, dt, friction_gam, verbose=False):
        # if not self._tissue_loaded:
        #     raise Exception("Cannot configure integrator before tissue has been loaded" +
        #         " - currently c++ code relies on tissue types being there before brownian integrator")
        if verbose:
            print("Adding brownian integrator")
        self._integrators.add('brownian')    


        dt = 0.3
        friction_gam = 1.0
        if verbose:
            print("Setting dt={}, friction_gamma={}".format(dt, friction_gam))
        self._integrators.set_dt(dt) # set time step
        self._integrators.set_params("brownian", {"gamma": friction_gam})
        if verbose:
            print("Done configuring integrators")
    
    def run_steps(
        self,
        n_steps,
        verbose=False,
    ):
        if not self.forces_configured:
            raise Exception("Need to configure force before running")
        
        if verbose:
            print("About to run simulation for {} steps".format(n_steps))
        
        self._simulation.run(n_steps, topological_change=False,)
        


# @TODO the categories and forces should be totally abstracted out of the base vertex model and wrapper
# the vertex model and the cass that controls it should only care about the forces for each individual vertex.
class SimModel:
    def __init__(self, verbose):
        self._vm_wrapper = VMToolkitWrapper(verbose=verbose)
        pass
    
    def configure_integrators(self,verbose=False):
        self._vm_wrapper.configure_integrators(verbose=verbose)
    
    def dump_cpp_json(self):
        return self._vm_wrapper.dump_cpp_json()
        
    def get_json_state(self):
        vertices = {}
        
        cpp_json = json.loads(self._vm_wrapper.dump_cpp_json())
        # print(cpp_json["mesh"])
        # raise NotImplementedError()
        for vtx_index, vtx in enumerate(cpp_json["mesh"]["vertices"]):
            print(vtx)
            vtx_id = str(vtx_index)
            vertices[vtx_id] = {
                "x": vtx['r'][0],
                "y": vtx['r'][1],
            }
        
        
        # edges = {}
        cells = {}
        for cell_index, cpp_cell in enumerate(cpp_json["mesh"]["faces"]):
            cell_id = str(cell_index)
            cells[cell_id] = {
                "vertices": [str(vidx) for vidx in cpp_cell["vertices"]]
            }
        
        return {
            "vertices": vertices,
            "cells": cells,
        }

    def configure_forces(self, P0_model, gamma, kappa, verbose=False):
        self._vm_wrapper.configure_forces(P0_model, gamma, kappa, verbose=verbose)
    
    def load_json_obj(self, json_obj, verbose=False):
        self._vm_wrapper.load_json_obj(json_obj, verbose=verbose)

    
        
    def run_steps(self, n_steps, verbose=False):
        self._vm_wrapper.run_steps(n_steps, verbose=verbose)
        
        

default_config = {
    "integrator": {
        "type": "brownian",
        "temp": 0,
        "friction_gam": 1.0,
        "dt": 0.3,
    },
    
    "cell_types": {
        "default": {
            "forces": {
                "area": {
                    'kappa': 1.0,
                },
                "perimeter": {
                    'gamma': 0.15,
                    'lambda': 0.15 * 0.20 ,
                },
            },
        },
    }
}