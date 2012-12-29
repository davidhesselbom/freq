#pragma once

#ifdef _MSC_VER
#include <stdlib.h> //  error C2381: 'exit' : redefinition; __declspec(noreturn) differs
#endif

// OpenGL
#include "gl.h" // from gpumisc
#ifndef __APPLE__
#   include <GL/glut.h>
#else
#   include <GLUT/glut.h>
#endif

// Sonic AWE
#include "sawe/project.h"

#ifndef NOGUI
#include "heightmap/collection.h"
#include "heightmap/renderer.h"
#include "tfr/cwtfilter.h"
#include "tfr/stftfilter.h"
#include "tools/rendercontroller.h"
#include "ui/mainwindow.h"
#endif

// gpumisc
#include "TaskTimer.h"
#ifdef USE_CUDA
#include "cuda_vector_types_op.h"
#endif

// Qt
#ifndef NOGUI
#include <QDockWidget>
#include <QWheelEvent>
#include <QHBoxLayout>
#endif

// boost
#include <boost/archive/binary_iarchive.hpp> 
#include <boost/archive/binary_oarchive.hpp> 
#include <boost/serialization/base_object.hpp> 

