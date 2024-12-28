
import argparse
import os
import json


from simcode.sim_model.sim_model import SimModel
from simcode.squeeze_testing.squeezer import build_squeezer_model
# from ..sim_model.vm_state import (
#     VMState, SimulationSettings, IntegratorSettings, 
#     CellAreaForce, CellPerimeterForce, ConstantVertexForce,  CellGroup, VertexGroup,
#     ElectricForceOnCellBoundary, EFieldSpecConstantPolygonRegion, PolygonSpec,
    
# )
# from ..sim_model.box_selector import BoxSelector

# def analyze_squeeze_results()

def do_stuff():
    parser = argparse.ArgumentParser()
    
    args = parser.parse_args()
    
    
    vm_initial_state = build_squeezer_model(
        tissue_cell_properties={
            "A0": 1.0,
            "P0": 0.3,
            "gamma": 0.16,
            "kappa": 1.0,
        },
        env_setup_properties={
            "box_lx": 15.0,
            "box_ly": 5.0,
            "forcing_field_strength": 0.005,
            "forcing_field_relative_width": 1/4,
        },
        simulation_settings={
            "friction_gamma": 0.1,
            "step_dt": 0.05,
        },
    )
    
    # # #################################################################
    # #
    # # Create the initial configuration and read it
    # #
    # # #################################################################
    print("Instantiating SimModel")
    sim_model = SimModel(verbose=False)
    print("Running sim_model.load_from_json_state")
    
    with open("scratch/initial_vm_state.json", "w") as f:
        f.write(json.dumps(vm_initial_state.to_json()))
    
    sim_model.load_from_json_state(vm_initial_state.to_json())
    print("Finished sim_mode.load_from_json_state")
    
    checkpoint_fps = []
    
    ckpt_dir = "scratch"
    if not os.path.exists(ckpt_dir):
        raise Exception("could not find {}".format(ckpt_dir))
    
    for fn in os.listdir(ckpt_dir):
        if (fn.startswith("res") or fn.startswith("vmst_")) and fn.endswith(".json"):
            os.remove(os.path.join(ckpt_dir, fn))
    
    step_size = 40     # Step counter in terms of time units
    N_checkpoints = 50
    
    for i in range(N_checkpoints):
        ckpt_fp = "scratch/res{}.json".format(str(i).zfill(3))
        json_str = sim_model.dump_cpp_json()
        with open(ckpt_fp, "w") as f:
            f.write(json_str)
        
        sim_model.run_steps(step_size)


if __name__ == "__main__":
    do_stuff()
