# Introduction #
Source code of my Master Thesis project "MPSoC for High Performance Heterogenous Computing".

This repository contains the source code of the two applications of my master thesis:

- SobelFilter
- StereoDepth

The report is also included in pdf format in the "doc" directory.

The discussed topics are:
- The Zynq Ultrascale+ MPSoC Architecture.
- The SDSoC Development Environnment (concepts, techniques and custom platform creation).
- Software Stack and Libraries (OpenCV, xfOpenCV, OpenGL ES).
- Development of Video Processing Applications :
  - SobelFilter implementation in hw vs sw vs gpu.
  - StereoDepth computing the block matching algorithm and displaying a 3D points cloud.

Requirements
============
The development environnment Xilinx SDSoC 2017.4, which provides trial licenses : https://www.xilinx.com/products/design-tools/software-zone/sdsoc.html

The library xfOpenCV available at : https://github.com/Xilinx/xfopencv.

The library "esUtils" available at: https://github.com/danginsburg/opengles-book-samples/tree/master/LinuxX11/Common.

The boost library : http://www.boost.org/

The GLM library : https://glm.g-truc.net/0.9.9/index.html

Platform
=========
Avnet UltraZed-EG
-----------------
The applications are intended to run on an UltraZed-EG System-on-Module (http://microzed.org/product/ultrazed-EG).

It should be fairly easy to port them on any Zynq Ultrascale+ MPSoC (https://www.xilinx.com/products/silicon-devices/soc/zynq-ultrascale-mpsoc.html).

SDSoC Platform ultrazed_dp_sds
------------------------------
A custom SDSoC Platform has been created for this project. The files are given in the "platform" directory.
To use this platform, unzip the file ultrazed_dp_sds.zip. It has been tested with SDSoC 2017.2 and 2017.4.

