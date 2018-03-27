# State of the report #
This documentation is a Master's thesis provided "as is". It was delivered on February, 9. 2018 and not updated ever since.

The report does not take into account the latest add-ons and improvements brought to the StereoDepth application.
Indeed, there has been several modifications on the following topics that are NOT covered in the thesis due to lack of time at delivery.

V4L2
----
Implementation of the video capture, directly with V4L2 (Video4Linux2) with the class `CaptureVideo`.

Features:
- Uses `V4L2_MEMORY_USERPTR` buffer type
- Multithreaded with 2 thread-safe queues (`boost::lock_free::spsc_queue`)

The reference that was used is https://linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/capture.c.html.

Color-space conversion
----------------------
One area that could see massive performance improvements is color conversion that is done in software (OpenCV). The ZED Camera outputs the color format "YUYV 4:2:2" and OpenGL ES needs RGBA pixel format. The color conversion was hand over directly to the GPU, which improved performance.

Color space conversion can be done by the GPU in fragment/vertex shader with the following input formats :

- YUYV 4:2:2
- GRAYSCALE (8-bit)
- GRAYSCALE (16-bit)

The source code is located in the class `Graphics2D`.

Generate Point Cloud
--------------------
The class `Graphics3D` has seen several improvements:

- The number of vertex attributes is reduced from 5 to 3, by removing texture coordinates (s, t).
  - Texture coordinates are infered from position coordinates in the vertex shader.
  - Scaling of vertices coordinates is done in the Model-View-Position `transform` matrix.

- The vertex coordinates X and Y are generated only once (at init).
 - Only the Z coordinate is updated in the function `Graphics3D::generatePointCloud()`.

- The double for-loops use OpenMP pragmas for parallel execution in CPU:
 - `#pragma omp parallel for collapse(2)`

These improvements significantly reduce the overhead of the `Graphics3D::drawPoints()` function.

Capture with the ZED Camera at 1344x376 and 30FPS and display of the Points Cloud now runs flawlessly.
