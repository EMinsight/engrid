TEMPLATE = app
LANGUAGE = C++

#CONFIG  += qt release thread
CONFIG += qt debug thread
QT += xml network opengl

LIBS += -lvtkCommon 
LIBS += -lvtkGraphics 
LIBS += -lvtkImaging 
LIBS += -lvtkHybrid 
LIBS += -lQVTK


LIBS += -lng

#DEFINES += QT_NO_DEBUG

!win32 {
    LIBS += -L./netgen_svn
    LIBS += -L$(VTKDIR)/lib/$(VTKVERSION)
    LIBS += -L$(VTKDIR)/lib/vtk-5.2
    LIBS += -Wl,-rpath
    QMAKE_CXXFLAGS += -Wno-deprecated
    INCLUDEPATH += $(VTKDIR)/include/$(VTKVERSION)
    INCLUDEPATH += ./netgen_svn/netgen-mesher/netgen/nglib
    INCLUDEPATH += ./netgen_svn/netgen-mesher/netgen/libsrc/general
}

win32 {
    VTK_DIR = C:\VTK
    VTK_SRCDIR = C:\VTK\5.0.4
    LIBS += -L$$VTK_DIR\bin\release
    LIBS += -lvtkRendering
    LIBS += -lvtkFiltering
    LIBS += -lvtkIO
    LIBS += -lvtkfreetype
    LIBS += -lvtkftgl
    LIBS += -lvtkexpat
    LIBS += -lvtkzlib
    INCLUDEPATH += $$VTK_SRCDIR\COMMON
    INCLUDEPATH += $$VTK_SRCDIR\FILTER~1
    INCLUDEPATH += $$VTK_SRCDIR\GUISUP~1\QT
    INCLUDEPATH += $$VTK_SRCDIR\GENERI~1
    INCLUDEPATH += $$VTK_SRCDIR\GRAPHICS
    INCLUDEPATH += $$VTK_SRCDIR\HYBRID
    INCLUDEPATH += $$VTK_SRCDIR\IMAGING
    INCLUDEPATH += $$VTK_SRCDIR\IO
    INCLUDEPATH += $$VTK_SRCDIR\RENDER~1
    INCLUDEPATH += $$VTK_DIR
    INCLUDEPATH += netgen_cvs\netgen\libsrc\interface
    INCLUDEPATH += netgen_cvs\netgen\libsrc\general
    LIBS += -Lnetgen_cvs\release
    DEFINES += _USE_MATH_DEFINES
}


RESOURCES += engrid.qrc

INCLUDEPATH += ./
INCLUDEPATH += ./nglib

HEADERS = \
\
boundarycondition.h \
celllayeriterator.h \
cellneighbouriterator.h \
containertricks.h \
correctsurfaceorientation.h \
createvolumemesh.h \
deletecells.h \
deletetetras.h \
deletepickedcell.h \
deletevolumegrid.h \
dialogoperation.h \
egvtkobject.h \
elements.h \
engrid.h \
error.h \
fixstl.h \
foamreader.h \
foamwriter.h \
geometrytools.h \
gmshiooperation.h \
gmshreader.h \
gmshwriter.h \
gridsmoother.h \
iooperation.h \
iterator.h \
layeriterator.h \
neutralwriter.h \
nodelayeriterator.h \
operation.h \
optimisation.h \
polydatareader.h \
polymesh.h \
seedsimpleprismaticlayer.h \
setboundarycode.h \
sortablevector.h \
std_connections.h \
std_includes.h \
stlreader.h \
stlwriter.h \
uniquevector.h \
swaptriangles.h \
tvtkoperation.h \
vtkreader.h \
\
vtkEgBoundaryCodesFilter.h \
vtkEgEliminateShortEdges.h \
vtkEgExtractVolumeCells.h \
vtkEgGridFilter.h \
vtkEgNormalExtrusion.h \
vtkEgPolyDataToUnstructuredGridFilter.h \
\
guicreateboundarylayer.h \
guideletebadaspecttris.h \
guidivideboundarylayer.h \
guiimproveaspectratio.h \
guimainwindow.h \
guinormalextrusion.h \
guiselectboundarycodes.h \
guisetboundarycode.h \
guismoothsurface.h \
 \
 guieditboundaryconditions.h \
 settingstab.h \
 settingsviewer.h

SOURCES = \
main.cpp \
\
boundarycondition.cpp \
celllayeriterator.cpp \
cellneighbouriterator.cpp \
correctsurfaceorientation.cpp \
createvolumemesh.cpp \
deletecells.cpp \
deletepickedcell.cpp \
deletetetras.cpp \
deletevolumegrid.cpp \
egvtkobject.cpp \
elements.cpp \
error.cpp \
fixstl.cpp \
foamreader.cpp \
foamwriter.cpp \
geometrytools.cpp \
gmshiooperation.cpp \
gmshreader.cpp \
gmshwriter.cpp \
gridsmoother.cpp \
iooperation.cpp \
iterator.cpp \
layeriterator.cpp \
neutralwriter.cpp \
nodelayeriterator.cpp \
operation.cpp \
optimisation.cpp \
polydatareader.cpp \
polymesh.cpp \
seedsimpleprismaticlayer.cpp \
setboundarycode.cpp \
stlreader.cpp \
stlwriter.cpp \
swaptriangles.cpp \
vtkreader.cpp \
\
vtkEgBoundaryCodesFilter.cxx \
vtkEgEliminateShortEdges.cxx \
vtkEgExtractVolumeCells.cxx \
vtkEgGridFilter.cxx \
vtkEgNormalExtrusion.cxx \
vtkEgPolyDataToUnstructuredGridFilter.cxx \
\
guicreateboundarylayer.cpp \
guideletebadaspecttris.cpp \
guidivideboundarylayer.cpp \
guiimproveaspectratio.cpp \
guimainwindow.cpp \
guinormalextrusion.cpp \
guiselectboundarycodes.cpp \
guisetboundarycode.cpp \
guismoothsurface.cpp \
 \
 guieditboundaryconditions.cpp \
 settingsviewer.cpp \
 settingstab.cpp

FORMS = \
guicreateboundarylayer.ui \
guideletebadaspecttris.ui \
guidivideboundarylayer.ui \
guieditboundaryconditions.ui \
guimainwindow.ui \
guiimproveaspectratio.ui \
guinormalextrusion.ui \
guioutputwindow.ui \
guiselectboundarycodes.ui \
guisetboundarycode.ui \
guismoothsurface.ui \


