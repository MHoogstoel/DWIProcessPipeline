
#-----------------------------------------------------------------------------
## empty until ITK is brought into here as an ExternalProject
macro(PACKAGE_NEEDS_ITK gen)
  set(packageToCheck ITK)
  OPTION(OPT_USE_SYSTEM_${packageToCheck} "Use the system's ${packageToCheck} library." OFF)
  #  MARK_AS_ADVANCED(OPT_USE_SYSTEM_${packageToCheck})
  if(OPT_USE_SYSTEM_ITK)
    find_package(ITK REQUIRED)
    include(${ITK_USE_FILE})
    set(ITK_DEPEND "") ## Set the external depandancy for ITK
  else()
    set(proj Insight)
    ExternalProject_Add(${proj}
      CVS_REPOSITORY ":pserver:anonymous:insight@public.kitware.com:/cvsroot/Insight"
      CVS_MODULE "Insight"
      CVS_TAG -r ITK-3-20
      UPDATE_COMMAND ""
      SOURCE_DIR ${proj}
      BINARY_DIR ${proj}-build
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DBUILD_TESTING:BOOL=OFF
        -DITK_USE_REVIEW:BOOL=ON
        -DITK_USE_REVIEW_STATISTICS:BOOL=ON
        -DITK_USE_TRANSFORM_IO_FACTORIES:BOOL=ON
        -DITK_USE_ORIENTED_IMAGE_DIRECTION:BOOL=ON
        -DITK_IMAGE_BEHAVES_AS_ORIENTED_IMAGE:BOOL=ON
        -DITK_USE_OPTIMIZED_REGISTRATION_METHODS:BOOL=ON
        -DITK_USE_PORTABLE_ROUND:BOOL=ON
        -DITK_USE_CENTERED_PIXEL_COORDINATES_CONSISTENTLY:BOOL=ON
        -DITK_USE_TRANSFORM_IO_FACTORIES:BOOL=ON
        -DITK_LEGACY_REMOVE:BOOL=ON
        -DBUILD_EXAMPLES:BOOL=OFF
        INSTALL_COMMAND ""
        #    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}
    )
    set(ITK_DIR ${CMAKE_CURRENT_BINARY_DIR}/Insight-build)
    set (ITK_DEPEND ${proj}) ## Set the internal dependancy for ITK
  endif()
endmacro()

#-----------------------------------------------------------------------------
# Get and build VTK
#

macro(PACKAGE_NEEDS_VTK_NOGUI gen)
  set(packageToCheck VTK)
  OPTION(OPT_USE_SYSTEM_${packageToCheck} "Use the system's ${packageToCheck} library." OFF)
  #  MARK_AS_ADVANCED(OPT_USE_SYSTEM_${packageToCheck})
  if(OPT_USE_SYSTEM_VTK)
    find_package(VTK 5.6 REQUIRED)
    include(${VTK_USE_FILE})
    set(VTK_DEPEND "") ## Set the external depandancy for ITK
  else()
    set(proj vtk-5-6)
    set(vtk_tag -r VTK-5-6)
    set(vtk_module VTK)

    set(vtk_WRAP_TCL OFF)
    set(vtk_WRAP_PYTHON OFF)

    set(vtk_GUI_ARGS
        -DVTK_USE_GUISUPPORT:BOOL=OFF
        -DVTK_USE_QVTK:BOOL=OFF
        -DVTK_USE_QT:BOOL=OFF
        -DVTK_USE_X:BOOL=OFF
        -DVTK_USE_CARBON:BOOL=OFF
        -DVTK_USE_COCOA:BOOL=OFF
        -DVTK_USE_RENDERING:BOOL=OFF
    )
    if(APPLE)
      # Qt 4.6 binary libs are built with empty OBJCXX_FLAGS for mac Cocoa
      set(vtk_GUI_ARGS
        ${vtk_GUI_ARGS}
        -DVTK_REQUIRED_OBJCXX_FLAGS:STRING=
        )
    endif(APPLE)

    ExternalProject_Add(${proj}
      CVS_REPOSITORY ":pserver:anonymous:vtk@public.kitware.com:/cvsroot/VTK"
      CVS_MODULE "${vtk_module}"
      CVS_TAG ${vtk_tag}
      UPDATE_COMMAND ""
      SOURCE_DIR ${proj}
      BINARY_DIR ${proj}-build
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DVTK_USE_PARALLEL:BOOL=ON
        -DVTK_DEBUG_LEAKS:BOOL=OFF
        -DVTK_WRAP_TCL:BOOL=${vtk_WRAP_TCL}
        -DVTK_WRAP_PYTHON:BOOL=${vtk_WRAP_PYTHON}
        ${vtk_GUI_ARGS}
      INSTALL_COMMAND ""
      #  INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    )

    set(VTK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
    set(VTK_DEPEND ${proj})
    MESSAGE(STATUS "Setting VTK_DIR to -DVTK_DIR:PATH=${VTK_DIR}")
    set(VTK_CMAKE
       -DVTK_DIR:PATH=${VTK_DIR}
    )
  endif()
endmacro()

#-----------------------------------------------------------------------------
# Get and build SlicerExecutionModel
##  Build the SlicerExecutionModel Once, and let all derived project use the same version
macro(PACKAGE_NEEDS_SlicerExecutionModel gen)
  set(packageToCheck SlicerExecutionModel)
  OPTION(OPT_USE_SYSTEM_${packageToCheck} "Use the system's ${packageToCheck} library." OFF)
  #  MARK_AS_ADVANCED(OPT_USE_SYSTEM_${packageToCheck})
  if(OPT_USE_SYSTEM_SlicerExecutionModel)
    find_package(GenerateCLP NO_MODULE REQUIRED)
    include(${GenerateCLP_USE_FILE})
    set(GenerateCLP_DEPEND "")
    set(SlicerExecutionModel_DEPEND "")
  else()
    #### ALWAYS BUILD WITH STATIC LIBS
    set(proj SlicerExecutionModel)
    ExternalProject_Add(${proj}
      SVN_REPOSITORY "http://svn.slicer.org/Slicer3/trunk/Libs/SlicerExecutionModel"
      SOURCE_DIR ${proj}
      BINARY_DIR ${proj}-build
      DEPENDS ${ITK_DEPEND}
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        -DBUILD_SHARED_LIBS:BOOL=OFF
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DITK_DIR:PATH=${ITK_DIR}
        INSTALL_COMMAND ""
        # INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}
    )
    set(GenerateCLP_DIR ${CMAKE_CURRENT_BINARY_DIR}/SlicerExecutionModel-build/GenerateCLP)
    set(GenerateCLP_DEPEND "${proj}")
    set(SlicerExecutionModel_DEPEND "${proj}")
  endif()
endmacro()

#-----------------------------------------------------------------------------
# Get and build BatchMake
##  Build the BatchMake Once, and let all derived project use the same version

macro(PACKAGE_NEEDS_BatchMake gen)
  set(packageToCheck BatchMake)
    OPTION(OPT_USE_SYSTEM_${packageToCheck} "Use the system's ${packageToCheck} library." OFF)
  if(OPT_USE_SYSTEM_BatchMake)
    find_package(BatchMake REQUIRED)
    include(${BatchMake_USE_FILE})
    set(BatchMake_DEPEND "")
  else(OPT_USE_SYSTEM_BatchMake)
    include(External_BatchMake)
    set(BatchMake_DIR ${CMAKE_CURRENT_BINARY_DIR}/BatchMake-build)
    set(BatchMake_DEPEND BatchMake )
  endif(OPT_USE_SYSTEM_BatchMake)
endmacro()
