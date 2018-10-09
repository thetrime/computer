# computer
Computer, on screen!


# Compiling tensorflow
We need libtensorflow (ie the C api). This is unfortunately not available pre-compiled for the raspberry pi. Fortunately, it is not too difficult to compile, even if it is incredibly time consuming. On my mac, I installed Docker (make sure you allocate enough resources or the compile will just fail. I used 4 CPUs, 8GB of memor and 1.5GB of swap (originally I had about 2GB of memory and 2 CPUs and it did not successfully compile).

Then just do something like this: (I built 1.10 because 1.11 also failed to build for me)
$$
git clone https://github.com/tensorflow/tensorflow.git
cd tensorflow
git checkout r1.10
CI_DOCKER_EXTRA_PARAMS="-e CI_BUILD_PYTHON=python3 -e CROSSTOOL_PYTHON_INCLUDE_PATH=/usr/include/python3.4" tensorflow/tools/ci_build/ci_build.sh PI-PYTHON3 tensorflow/tools/ci_build/pi/build_raspberry_pi.sh
$$

Then you should have the three files you need:
   * c_api/lib/libtensorflow.so
   * c_api/lib/libtensorflow_framework.so
   * tensorflow/c/c_api.h

You need to put these into sensible locations such as /opt/tensorflow/lib and /opt/tensorflow/include. Note that the build took me almost 5 hours, not '30 minutes' like the Tensorflow documentation suggested. It also almost drive my battery to 0% even while charging. YMMV.