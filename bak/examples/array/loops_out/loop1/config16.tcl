loop_pipeline "loop3"
set_parameter LOCAL_RAMS 1
set_parameter MODULO_SCHEDULER "NI"
set_resource_constraint mem_dual_port 2
set_resource_constraint signed_add_32 2
set_resource_constraint signed_comp_eq_32 1
set_resource_constraint signed_divide_32 1
set_resource_constraint signed_multiply_32 1
