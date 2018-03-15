# zc702_pfm.tcl --
# 
# This file uses the SDSoC Tcl Platform API to create the
# UltraZed hardware platform file
#
# Author : Thibault Chappuis
# Date : 19.10.2017

# call base script for pfm generation
#source -notrace C:/Xilinx/SDx/2017.2/scripts/vivado/sdsoc_pfm.tcl

# set a var called "pfm" for location of platform
set pfm [sdsoc::create_pfm ultrazed_dp_sds.hpfm]

# Description of the platform
sdsoc::pfm_name 		$pfm "Hes-so" "xd" "UltraZed-IOCC DP" "1.0"
sdsoc::pfm_description	$pfm "UltraZed SOM + I/O Carrier Card with GPIOs and DisplayPort (DP)"

# Assign Properties
sdsoc::pfm_clock		$pfm pl_clk0 ps_e 0 true proc_sys_reset_0; # default
sdsoc::pfm_clock 		$pfm pl_clk1 ps_e 1 false proc_sys_reset_1
sdsoc::pfm_clock		$pfm pl_clk2 ps_e 2 false proc_sys_reset_2
sdsoc::pfm_clock		$pfm pl_clk3 ps_e 3 false proc_sys_reset_3

# Add all AXI ports (General Purpose, Accelerator Coherence Port, High Performance)
sdsoc::pfm_axi_port 	$pfm M_AXI_HPM0_FPD ps_e M_AXI_GP
sdsoc::pfm_axi_port 	$pfm M_AXI_HPM1_FPD ps_e M_AXI_GP
sdsoc::pfm_axi_port 	$pfm M_AXI_HPM0_LPD ps_e M_AXI_GP
sdsoc::pfm_axi_port 	$pfm S_AXI_HPC0_FPD ps_e S_AXI_HPC
sdsoc::pfm_axi_port 	$pfm S_AXI_HPC1_FPD ps_e S_AXI_HPC
sdsoc::pfm_axi_port 	$pfm S_AXI_HP0_FPD  ps_e S_AXI_HP
sdsoc::pfm_axi_port 	$pfm S_AXI_HP1_FPD  ps_e S_AXI_HP
sdsoc::pfm_axi_port 	$pfm S_AXI_HP2_FPD  ps_e S_AXI_HP
sdsoc::pfm_axi_port 	$pfm S_AXI_HP3_FPD  ps_e S_AXI_HP

#sdsoc::pfm_axi_port		$pfm S00_AXI ps8_0_axi_periph M_AXI_GP

# Export all unused Master ports of the AXI Interconnect IP [3:15]
for {set i 3} {$i < 16} {incr i} {
  sdsoc::pfm_axi_port  $pfm M[format %02d $i]_AXI ps8_0_axi_periph M_AXI_GP
}

# Assign axi_gpio_* to be driven by software, with Linux UserIO subsystem
sdsoc::pfm_iodev 		$pfm S_AXI axi_gpio_0 uio
sdsoc::pfm_iodev 		$pfm S_AXI axi_gpio_1 uio
sdsoc::pfm_iodev 		$pfm S_AXI axi_gpio_2 uio

# Assign all the interrupt ports to the 2 concat blocks
for {set i 0} {$i < 8} {incr i} {
  sdsoc::pfm_irq $pfm In$i xlconcat_0
}

for {set i 0} {$i < 8} {incr i} {
  sdsoc::pfm_irq $pfm In$i xlconcat_1
}


# Eventually add AXI-Stream Ports
#sdsoc::pfm_axis_port	$pfm S_AXIS axis2io S_AXIS
#sdsoc::pfm_axis_port	$pfm M_AXIS io2axis M_AXIS

# Finally, generate the *.pfm file...
sdsoc::generate_hw_pfm $pfm