//
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +                                                                      +
// + This file is part of enGrid.                                         +
// +                                                                      +
// + Copyright 2008,2009 Oliver Gloth                                     +
// +                                                                      +
// + enGrid is free software: you can redistribute it and/or modify       +
// + it under the terms of the GNU General Public License as published by +
// + the Free Software Foundation, either version 3 of the License, or    +
// + (at your option) any later version.                                  +
// +                                                                      +
// + enGrid is distributed in the hope that it will be useful,            +
// + but WITHOUT ANY WARRANTY; without even the implied warranty of       +
// + MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        +
// + GNU General Public License for more details.                         +
// +                                                                      +
// + You should have received a copy of the GNU General Public License    +
// + along with enGrid. If not, see <http://www.gnu.org/licenses/>.       +
// +                                                                      +
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
#include "guimainwindow.h"
#include "guiselectboundarycodes.h"
#include "guiimproveaspectratio.h"
#include "guinormalextrusion.h"
#include "guisetboundarycode.h"

#include "vtkEgPolyDataToUnstructuredGridFilter.h"
#include "stlreader.h"
#include "gmshreader.h"
#include "gmshwriter.h"
#include "neutralwriter.h"
#include "stlwriter.h"
#include "correctsurfaceorientation.h"
#include "guieditboundaryconditions.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkProperty.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>

#include <QFileDialog>
#include <QFileSystemWatcher>

#include "settingsviewer.h"

GuiOutputWindow::GuiOutputWindow()
{
  ui.setupUi(this);
};

QString GuiMainWindow::cwd = ".";
QSettings GuiMainWindow::qset("enGits","enGrid");
GuiMainWindow* GuiMainWindow::THIS = NULL;
QMutex GuiMainWindow::mutex;

GuiMainWindow::GuiMainWindow() : QMainWindow(NULL)
{
  ui.setupUi(this);
  THIS = this;
  dock_widget = new QDockWidget(tr("output"), this);
  dock_widget->setFeatures(QDockWidget::AllDockWidgetFeatures);
  output_window = new GuiOutputWindow();
  dock_widget->setWidget(output_window);
  addDockWidget(Qt::LeftDockWidgetArea, dock_widget);
  ui.menuView->addAction(dock_widget->toggleViewAction());
  
  connect(ui.actionImportSTL,              SIGNAL(activated()),       this, SLOT(importSTL()));
  connect(ui.actionImportGmsh1Ascii,       SIGNAL(activated()),       this, SLOT(importGmsh1Ascii()));
  connect(ui.actionImportGmsh2Ascii,       SIGNAL(activated()),       this, SLOT(importGmsh2Ascii()));
  connect(ui.actionExportGmsh1Ascii,       SIGNAL(activated()),       this, SLOT(exportGmsh1Ascii()));
  connect(ui.actionExportGmsh2Ascii,       SIGNAL(activated()),       this, SLOT(exportGmsh2Ascii()));
  connect(ui.actionExportNeutral,          SIGNAL(activated()),       this, SLOT(exportNeutral()));
  connect(ui.actionExportAsciiStl,         SIGNAL(activated()),       this, SLOT(exportAsciiStl()));
  connect(ui.actionExportBinaryStl,        SIGNAL(activated()),       this, SLOT(exportBinaryStl()));
  connect(ui.actionExit,                   SIGNAL(activated()),       this, SLOT(exit()));
  connect(ui.actionZoomAll,                SIGNAL(activated()),       this, SLOT(zoomAll()));
  connect(ui.actionOpen,                   SIGNAL(activated()),       this, SLOT(open()));
  connect(ui.actionSave,                   SIGNAL(activated()),       this, SLOT(save()));
  connect(ui.actionSaveAs,                 SIGNAL(activated()),       this, SLOT(saveAs()));
  connect(ui.actionBoundaryCodes,          SIGNAL(activated()),       this, SLOT(selectBoundaryCodes()));
  connect(ui.actionNormalExtrusion,        SIGNAL(activated()),       this, SLOT(normalExtrusion()));
  connect(ui.actionViewAxes,               SIGNAL(changed()),         this, SLOT(setAxesVisibility()));
  connect(ui.actionChangeOrientation,      SIGNAL(activated()),       this, SLOT(changeSurfaceOrientation()));
  connect(ui.actionCheckOrientation,       SIGNAL(activated()),       this, SLOT(checkSurfaceOrientation()));
  connect(ui.actionImproveAspectRatio,     SIGNAL(activated()),       this, SLOT(improveAspectRatio()));
  connect(ui.actionRedraw,                 SIGNAL(activated()),       this, SLOT(updateActors()));
  connect(ui.actionClearOutputWindow,      SIGNAL(activated()),       this, SLOT(clearOutput()));
  connect(ui.actionEditBoundaryConditions, SIGNAL(activated()),       this, SLOT(editBoundaryConditions()));
  connect(ui.actionMeshingOptions,          SIGNAL(activated()),      this, SLOT(MeshingOptions()));
  
  connect(ui.actionViewXP, SIGNAL(activated()), this, SLOT(viewXP()));
  connect(ui.actionViewXM, SIGNAL(activated()), this, SLOT(viewXM()));
  connect(ui.actionViewYP, SIGNAL(activated()), this, SLOT(viewYP()));
  connect(ui.actionViewYM, SIGNAL(activated()), this, SLOT(viewYM()));
  connect(ui.actionViewZP, SIGNAL(activated()), this, SLOT(viewZP()));
  connect(ui.actionViewZM, SIGNAL(activated()), this, SLOT(viewZM()));
  
  
#include "std_connections.h"
  
  if (qset.contains("working_directory")) {
    cwd = qset.value("working_directory").toString();
  };
  grid = vtkUnstructuredGrid::New();
  renderer = vtkRenderer::New();
  getRenderWindow()->AddRenderer(renderer);
  surface_actor = vtkActor::New();
  surface_wire_actor = vtkActor::New();
  
  tetra_mapper        = vtkPolyDataMapper::New();
  pyramid_mapper      = vtkPolyDataMapper::New();
  wedge_mapper        = vtkPolyDataMapper::New();
  hexa_mapper         = vtkPolyDataMapper::New();
  volume_wire_mapper  = vtkPolyDataMapper::New();
  surface_mapper      = vtkPolyDataMapper::New();
  surface_wire_mapper = vtkPolyDataMapper::New();
  
  backface_property = vtkProperty::New();
  
  tetra_actor       = NULL;
  pyramid_actor     = NULL;
  wedge_actor       = NULL;
  hexa_actor        = NULL;
  volume_wire_actor = NULL;
  
  surface_filter = vtkGeometryFilter::New();
  bcodes_filter = vtkEgBoundaryCodesFilter::New();
  renderer->AddActor(surface_actor);
  renderer->AddActor(surface_wire_actor);
  pick_sphere = vtkSphereSource::New();
  pick_mapper = vtkPolyDataMapper::New();
  pick_actor = NULL;
  
  extr_vol        = vtkEgExtractVolumeCells::New();
  extr_tetras     = vtkEgExtractVolumeCells::New();
  extr_pyramids   = vtkEgExtractVolumeCells::New();
  extr_wedges     = vtkEgExtractVolumeCells::New();
  extr_hexas      = vtkEgExtractVolumeCells::New();
  
  volume_geometry  = vtkGeometryFilter::New();
  tetra_geometry   = vtkGeometryFilter::New();
  pyramid_geometry = vtkGeometryFilter::New();
  wedge_geometry   = vtkGeometryFilter::New();
  hexa_geometry   = vtkGeometryFilter::New();
  
  extr_tetras->SetAllOff();
  extr_tetras->SetTetrasOn();
  extr_pyramids->SetAllOff();
  extr_pyramids->SetPyramidsOn();
  extr_wedges->SetAllOff();
  extr_wedges->SetWedgesOn();
  extr_hexas->SetAllOff();
  extr_hexas->SetHexasOn();
  
  boundary_pd = vtkPolyData::New();
  tetras_pd   = vtkPolyData::New();
  wedges_pd   = vtkPolyData::New();
  pyras_pd    = vtkPolyData::New();
  hexas_pd    = vtkPolyData::New();
  volume_pd   = vtkPolyData::New();
  
  current_filename = "untitled.vtu";
  setWindowTitle(current_filename + " - enGrid");
  
  status_bar = new QStatusBar(this);
  setStatusBar(status_bar);
  status_label = new QLabel(this);
  status_bar->addWidget(status_label);
  QString txt = "0 volume cells (0 tetras, 0 hexas, 0 pyramids, 0 prisms), ";
  txt += "0 surface cells (0 triangles, 0 quads), 0 nodes";
  status_label->setText(txt);
  
  //centralWidget()->layout()->setContentsMargins(0,0,0,0);
  
  axes = vtkCubeAxesActor2D::New();
  axes->SetCamera(getRenderer()->GetActiveCamera());
  getRenderer()->AddActor(axes);
  setAxesVisibility();
  
  picker = vtkCellPicker::New();
  getInteractor()->SetPicker(picker);
  
  vtkCallbackCommand *cbc = vtkCallbackCommand::New();
  cbc->SetCallback(pickCallBack);
  picker->AddObserver(vtkCommand::EndPickEvent, cbc);
  cbc->Delete();
  
  if (qset.contains("tmp_directory")) {
    log_file_name = qset.value("tmp_directory").toString() + "/enGrid_output.txt";
  } else {
    log_file_name = "/tmp/enGrid_output.txt";
  };
  system_stdout = stdout;
  freopen (log_file_name.toAscii().data(), "w", stdout);
  
  busy = false;
  updateStatusBar();
  
  connect(&garbage_timer, SIGNAL(timeout()), this, SLOT(periodicUpdate()));
  garbage_timer.start(1000);
  
  connect(&log_timer, SIGNAL(timeout()), this, SLOT(updateOutput()));
  log_timer.start(1000);
  
  N_chars = 0;
  
};

GuiMainWindow::~GuiMainWindow()
{
};

void GuiMainWindow::updateOutput()
{
  QFile log_file(log_file_name);
  log_file.open(QIODevice::ReadOnly);
  QByteArray buffer = log_file.readAll();
  if (buffer.size() > N_chars) {
    QByteArray newchars = buffer.right(buffer.size() - N_chars);
    N_chars = buffer.size();
    QString txt(newchars);
    if (txt.right(1) == "\n") {
      txt = txt.left(txt.size()-1);
    };
    output_window->ui.textEditOutput->append(txt);
  };
};

void GuiMainWindow::exit()
{
  QCoreApplication::exit();
};

vtkRenderWindow* GuiMainWindow::getRenderWindow() 
{
  return ui.qvtkWidget->GetRenderWindow(); 
};

vtkRenderer* GuiMainWindow::getRenderer()
{
  return renderer;
};

QVTKInteractor* GuiMainWindow::getInteractor() 
{
  return ui.qvtkWidget->GetInteractor(); 
};

QString GuiMainWindow::getCwd()
{
  return cwd;
};

void GuiMainWindow::setCwd(QString dir)
{
  cwd = dir;
  qset.setValue("working_directory",dir);
};

void GuiMainWindow::updateActors()
{
  if (!tryLock()) return;
  try {
    {
      {
        if (!grid->GetCellData()->GetScalars("cell_index")) {
          EG_VTKSP(vtkLongArray_t, cell_idx);
          cell_idx->SetName("cell_index");
          cell_idx->SetNumberOfValues(grid->GetNumberOfCells());
          grid->GetCellData()->AddArray(cell_idx);
        };
      };
      EG_VTKDCC(vtkLongArray_t, cell_index, grid, "cell_index");
      for (vtkIdType cellId = 0; cellId < grid->GetNumberOfCells(); ++cellId) {
        cell_index->SetValue(cellId, cellId);
      };
    };
    
    axes->SetInput(grid);
    
    double xmin =  1e99;
    double xmax = -1e99;
    double ymin =  1e99;
    double ymax = -1e99;
    double zmin =  1e99;
    double zmax = -1e99;
    for (vtkIdType nodeId = 0; nodeId < grid->GetNumberOfPoints(); ++nodeId) {
      vec3_t x;
      grid->GetPoints()->GetPoint(nodeId, x.data());
      xmin = min(x[0], xmin);
      xmax = max(x[0], xmax);
      ymin = min(x[1], ymin);
      ymax = max(x[1], ymax);
      zmin = min(x[2], zmin);
      zmax = max(x[2], zmax);
    };
    
    if (surface_actor) {
      getRenderer()->RemoveActor(surface_actor);
      surface_actor->Delete();
      surface_actor = NULL;
    };
    if (surface_wire_actor) {
      getRenderer()->RemoveActor(surface_wire_actor);
      surface_wire_actor->Delete();
      surface_wire_actor = NULL;
    };
    if (tetra_actor) {
      getRenderer()->RemoveActor(tetra_actor);
      tetra_actor->Delete();
      tetra_actor = NULL;
    };
    if (pyramid_actor) {
      getRenderer()->RemoveActor(pyramid_actor);
      pyramid_actor->Delete();
      pyramid_actor = NULL;
    };
    if (wedge_actor) {
      getRenderer()->RemoveActor(wedge_actor);
      wedge_actor->Delete();
      wedge_actor = NULL;
    };
    if (hexa_actor) {
      getRenderer()->RemoveActor(hexa_actor);
      hexa_actor->Delete();
      hexa_actor = NULL;
    };
    if (volume_wire_actor) {
      getRenderer()->RemoveActor(volume_wire_actor);
      volume_wire_actor->Delete();
      volume_wire_actor = NULL;
    };
    if (pick_actor) {
      getRenderer()->RemoveActor(pick_actor);
      pick_actor->Delete();
      pick_actor = NULL;
    };
    
    if (ui.checkBoxSurface->isChecked()) {
      bcodes_filter->SetBoundaryCodes(&display_boundary_codes);
      bcodes_filter->SetInput(grid);
      surface_filter->SetInput(bcodes_filter->GetOutput());
      surface_filter->Update();
      boundary_pd->DeepCopy(surface_filter->GetOutput());
      surface_mapper->SetInput(boundary_pd);
      surface_wire_mapper->SetInput(boundary_pd);
      surface_actor = vtkActor::New();
      surface_actor->GetProperty()->SetRepresentationToSurface();
      surface_actor->GetProperty()->SetColor(0,1,0);
      surface_actor->SetBackfaceProperty(backface_property);
      surface_actor->GetBackfaceProperty()->SetColor(1,0,0);
      surface_wire_actor = vtkActor::New();
      surface_wire_actor->GetProperty()->SetRepresentationToWireframe();
      surface_wire_actor->GetProperty()->SetColor(0,0,1);
      surface_actor->SetMapper(surface_mapper);
      surface_wire_actor->SetMapper(surface_wire_mapper);
      getRenderer()->AddActor(surface_actor);
      getRenderer()->AddActor(surface_wire_actor);
      bcodes_filter->Update();
      vtkIdType cellId = getPickedCell();
      if (cellId >= 0) {
        vtkIdType *pts, Npts;
        grid->GetCellPoints(cellId, Npts, pts);
        vec3_t x(0,0,0);
        for (vtkIdType i = 0; i < Npts; ++i) {
          vec3_t xp;
          grid->GetPoints()->GetPoint(pts[i], xp.data());
          x += double(1)/Npts * xp;
        };
        pick_sphere->SetCenter(x.data());
        double R = 1e99;
        for (vtkIdType i = 0; i < Npts; ++i) {
          vec3_t xp;
          grid->GetPoints()->GetPoint(pts[i], xp.data());
          R = min(R, 0.25*(xp-x).abs());
        };
        pick_sphere->SetRadius(R);
        pick_mapper->SetInput(pick_sphere->GetOutput());
        pick_actor = vtkActor::New();
        pick_actor->SetMapper(pick_mapper);
        pick_actor->GetProperty()->SetRepresentationToSurface();
        pick_actor->GetProperty()->SetColor(1,0,0);
        getRenderer()->AddActor(pick_actor);
      };
      
    };
    
    vec3_t x, n;
    x[0] = ui.lineEditClipX->text().toDouble();
    x[1] = ui.lineEditClipY->text().toDouble();
    x[2] = ui.lineEditClipZ->text().toDouble();
    n[0] = ui.lineEditClipNX->text().toDouble();
    n[1] = ui.lineEditClipNY->text().toDouble();
    n[2] = ui.lineEditClipNZ->text().toDouble();
    n.normalise();
    x = x + ui.lineEditOffset->text().toDouble()*n;
    extr_vol->SetAllOff();
    if (ui.checkBoxTetra->isChecked()) {
      extr_vol->SetTetrasOn();
      extr_tetras->SetInput(grid);
      if (ui.checkBoxClip->isChecked()) {
        extr_tetras->SetClippingOn();
        extr_tetras->SetX(x);
        extr_tetras->SetN(n);
      } else {
        extr_tetras->SetClippingOff();
      };
      tetra_actor = vtkActor::New();
      tetra_geometry->SetInput(extr_tetras->GetOutput());
      tetra_geometry->Update();
      tetras_pd->DeepCopy(tetra_geometry->GetOutput());
      tetra_mapper->SetInput(tetras_pd);
      tetra_actor = vtkActor::New();
      tetra_actor->SetMapper(tetra_mapper);
      tetra_actor->GetProperty()->SetColor(1,0,0);
      getRenderer()->AddActor(tetra_actor);
    };
    if (ui.checkBoxPyramid->isChecked()) {
      extr_vol->SetPyramidsOn();
      extr_pyramids->SetInput(grid);
      if (ui.checkBoxClip->isChecked()) {
        extr_pyramids->SetClippingOn();
        extr_pyramids->SetX(x);
        extr_pyramids->SetN(n);
      } else {
        extr_pyramids->SetClippingOff();
      };
      pyramid_actor = vtkActor::New();
      pyramid_geometry->SetInput(extr_pyramids->GetOutput());
      pyramid_geometry->Update();
      pyras_pd->DeepCopy(pyramid_geometry->GetOutput());
      pyramid_mapper->SetInput(pyras_pd);
      pyramid_actor = vtkActor::New();
      pyramid_actor->SetMapper(pyramid_mapper);
      pyramid_actor->GetProperty()->SetColor(1,1,0);
      getRenderer()->AddActor(pyramid_actor);
    };
    if (ui.checkBoxWedge->isChecked()) {
      extr_vol->SetWedgesOn();
      extr_wedges->SetInput(grid);
      if (ui.checkBoxClip->isChecked()) {
        extr_wedges->SetClippingOn();
        extr_wedges->SetX(x);
        extr_wedges->SetN(n);
      } else {
        extr_wedges->SetClippingOff();
      };
      wedge_actor = vtkActor::New();
      wedge_geometry->SetInput(extr_wedges->GetOutput());
      wedge_geometry->Update();
      wedges_pd->DeepCopy(wedge_geometry->GetOutput());
      wedge_mapper->SetInput(wedges_pd);
      wedge_actor = vtkActor::New();
      wedge_actor->SetMapper(wedge_mapper);
      wedge_actor->GetProperty()->SetColor(0,1,0);
      getRenderer()->AddActor(wedge_actor);
    };
    if (ui.checkBoxHexa->isChecked()) {
      extr_vol->SetHexasOn();
      extr_hexas->SetInput(grid);
      if (ui.checkBoxClip->isChecked()) {
        extr_hexas->SetClippingOn();
        extr_hexas->SetX(x);
        extr_hexas->SetN(n);
      } else {
        extr_hexas->SetClippingOff();
      };
      hexa_actor = vtkActor::New();
      hexa_geometry->SetInput(extr_hexas->GetOutput());
      hexa_geometry->Update();
      hexas_pd->DeepCopy(hexa_geometry->GetOutput());
      hexa_mapper->SetInput(hexas_pd);
      hexa_actor = vtkActor::New();
      hexa_actor->SetMapper(hexa_mapper);
      hexa_actor->GetProperty()->SetColor(0,0.7,1);
      getRenderer()->AddActor(hexa_actor);
    };
    
    // wireframe
    extr_vol->SetInput(grid);
    if (ui.checkBoxClip->isChecked()) {
      extr_vol->SetClippingOn();
      extr_vol->SetX(x);
      extr_vol->SetN(n);
    } else {
      extr_vol->SetClippingOff();
    };
    volume_wire_actor = vtkActor::New();
    volume_geometry->SetInput(extr_vol->GetOutput());
    volume_geometry->Update();
    volume_pd->DeepCopy(volume_geometry->GetOutput());
    volume_wire_mapper->SetInput(volume_pd);
    volume_wire_actor->SetMapper(volume_wire_mapper);
    volume_wire_actor->GetProperty()->SetRepresentationToWireframe();
    volume_wire_actor->GetProperty()->SetColor(0,0,1);
    getRenderer()->AddActor(volume_wire_actor);
    
    
    updateStatusBar();
    getRenderWindow()->Render();
  } catch (Error err) {
    err.display();
  };
  unlock();
};

void GuiMainWindow::importSTL()
{
  StlReader stl;
  stl();
  updateBoundaryCodes(true);
  updateActors();
  updateStatusBar();
  zoomAll();
};

void GuiMainWindow::importGmsh1Ascii()
{
  GmshReader gmsh;
  gmsh.setV1Ascii();
  gmsh();
  updateBoundaryCodes(true);
  updateActors();
  updateStatusBar();
  zoomAll();
};

void GuiMainWindow::exportGmsh1Ascii()
{
  GmshWriter gmsh;
  gmsh.setV1Ascii();
  gmsh();
};

void GuiMainWindow::importGmsh2Ascii()
{
  GmshReader gmsh;
  gmsh.setV2Ascii();
  gmsh();
  updateBoundaryCodes(true);
  updateActors();
  updateStatusBar();
  zoomAll();
};

void GuiMainWindow::exportGmsh2Ascii()
{
  GmshWriter gmsh;
  gmsh.setV2Ascii();
  gmsh();
};

void GuiMainWindow::exportNeutral()
{
  NeutralWriter neutral;
  neutral();
};

void GuiMainWindow::zoomAll()
{
  getRenderer()->ResetCamera();
  getRenderWindow()->Render();
};

void GuiMainWindow::open()
{
  current_filename = QFileDialog::getOpenFileName
    (
      NULL,
      "open grid from file",
      getCwd(),
      "VTK unstructured grid files (*.vtu *.VTU)"
    );
  if (!current_filename.isNull()) {
    GuiMainWindow::setCwd(QFileInfo(current_filename).absolutePath());
    EG_VTKSP(vtkXMLUnstructuredGridReader,vtu);
    vtu->SetFileName(current_filename.toAscii().data());
    vtu->Update();
    grid->DeepCopy(vtu->GetOutput());
    createBasicFields(grid, grid->GetNumberOfCells(), grid->GetNumberOfPoints(), false);
    setWindowTitle(current_filename + " - enGrid");  
    openBC();
    updateBoundaryCodes(true);
    updateActors();
    updateStatusBar();
    zoomAll();
  };
};

void GuiMainWindow::undo()
{
};

void GuiMainWindow::redo()
{
};

void GuiMainWindow::openBC()
{
  QString bc_file = current_filename + ".bcs";
  QFile file(bc_file);
  bcmap.clear();
  if (file.exists()) {
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream f(&file);
    while (!f.atEnd()) {
      QString name, type;
      int i;
      f >> i >> name >> type;
      bcmap[i] = BoundaryCondition(name,type);
    };
  };
};

void GuiMainWindow::saveBC()
{
  QString bc_file = current_filename + ".bcs";
  QFile file(bc_file);
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream f(&file);
  foreach(int i, all_boundary_codes) {
    BoundaryCondition bc = bcmap[i];
    f << i << " " << bc.getName() << " " << bc.getType() << "\n";
  };
};

void GuiMainWindow::save()
{
  cout << current_filename.toAscii().data() << endl;
  if (current_filename == "untitled.vtu") {
    saveAs();
  } else {
    EG_VTKDCC(vtkDoubleArray, cell_VA, grid, "cell_VA");
    for (vtkIdType cellId = 0; cellId < grid->GetNumberOfCells(); ++cellId) {
      cell_VA->SetValue(cellId, GeometryTools::cellVA(grid, cellId, true));
    };
    EG_VTKSP(vtkXMLUnstructuredGridWriter,vtu);
    addVtkTypeInfo();
    createIndices(grid);
    vtu->SetFileName(current_filename.toAscii().data());
    vtu->SetDataModeToBinary();
    vtu->SetInput(grid);
    vtu->Write();
    saveBC();
  };
};

void GuiMainWindow::saveAs()
{
  current_filename = QFileDialog::getSaveFileName
    (
      NULL,
      "write grid to file",
      getCwd(),
      "VTK unstructured grid files (*.vtu *.VTU)"
    );
  if (!current_filename.isNull()) {
    if (current_filename.right(4) != ".vtu") {
      if (current_filename.right(4) != ".VTU") {
        current_filename += ".vtu";
      };
    };
    EG_VTKDCC(vtkDoubleArray, cell_VA, grid, "cell_VA");
    for (vtkIdType cellId = 0; cellId < grid->GetNumberOfCells(); ++cellId) {
      cell_VA->SetValue(cellId, GeometryTools::cellVA(grid, cellId, true));
    };
    GuiMainWindow::setCwd(QFileInfo(current_filename).absolutePath());
    EG_VTKSP(vtkXMLUnstructuredGridWriter,vtu);
    addVtkTypeInfo();
    createIndices(grid);
    vtu->SetFileName(current_filename.toAscii().data());
    vtu->SetDataModeToBinary();
    vtu->SetInput(grid);
    vtu->Write();
    saveBC();
    setWindowTitle(current_filename + " - enGrid");
  };
};

void GuiMainWindow::updateStatusBar()
{
  QString num, txt = "enGrid is currently busy with an operation ...";
  if (!busy) {
    txt = "";
  };
  if (!tryLock()) {
    status_label->setText(txt);
    return;
  };
  vtkIdType Ncells = grid->GetNumberOfCells();
  vtkIdType Nnodes = grid->GetNumberOfPoints();
  vtkIdType Ntris  = 0;
  vtkIdType Nquads = 0;
  vtkIdType Ntets  = 0;
  vtkIdType Npyras = 0;
  vtkIdType Nprism = 0;
  vtkIdType Nhexas = 0;
  for (vtkIdType i = 0; i < Ncells; ++i) {
    int ct = grid->GetCellType(i);
    if      (ct == VTK_TRIANGLE)   ++Ntris;
    else if (ct == VTK_QUAD)       ++Nquads;
    else if (ct == VTK_TETRA)      ++Ntets;
    else if (ct == VTK_WEDGE)      ++Nprism;
    else if (ct == VTK_PYRAMID)    ++Npyras;
    else if (ct == VTK_HEXAHEDRON) ++Nhexas;
  };
  num.setNum(Ntets + Npyras + Nprism + Nhexas); txt += num + " volume cells(";
  num.setNum(Ntets);  txt += num + " tetras, ";
  num.setNum(Npyras); txt += num + " pyramids, ";
  num.setNum(Nprism); txt += num + " prisms, ";
  num.setNum(Nhexas); txt += num + " hexas), ";
  num.setNum(Ntris + Nquads); txt += num + " surface cells(";
  num.setNum(Ntris);  txt += num + " triangles, ";
  num.setNum(Nquads); txt += num + " quads), ";
  num.setNum(Nnodes); txt += num + " nodes";
  
  QString pick_txt = ", picked cell: ";
  vtkIdType id_cell = getPickedCell();
  if (id_cell < 0) {
    pick_txt += "none";
  } else {
    vtkIdType type_cell = grid->GetCellType(id_cell);
    if      (type_cell == VTK_TRIANGLE)   pick_txt += "triangle";
    else if (type_cell == VTK_QUAD)       pick_txt += "quad";
    else if (type_cell == VTK_TETRA)      pick_txt += "tetrahedron";
    else if (type_cell == VTK_PYRAMID)    pick_txt += "pyramid";
    else if (type_cell == VTK_WEDGE)      pick_txt += "prism";
    else if (type_cell == VTK_HEXAHEDRON) pick_txt += "hexahedron";
    vtkIdType N_pts, *pts;
    grid->GetCellPoints(id_cell, N_pts, pts);
    pick_txt += " [";
    for (int i_pts = 0; i_pts < N_pts; ++i_pts) {
      QString num;
      num.setNum(pts[i_pts]);
      pick_txt += num;
      if (i_pts < N_pts-1) {
        pick_txt += ",";
      };
    };
    pick_txt += "]";
  };
  txt += pick_txt;
  
  status_label->setText(txt);
  unlock();
};

void GuiMainWindow::selectBoundaryCodes()
{
  GuiSelectBoundaryCodes bcodes;
  bcodes.setDisplayBoundaryCodes(display_boundary_codes);
  bcodes.setBoundaryCodes(all_boundary_codes);
  bcodes();
  bcodes.getThread().wait();
  bcodes.getSelectedBoundaryCodes(display_boundary_codes);
  updateActors();
};

void GuiMainWindow::updateBoundaryCodes(bool all_on)
{
  try {
    all_boundary_codes.clear();
    EG_VTKDCC(vtkIntArray, cell_code, grid, "cell_code");
    for (vtkIdType i = 0; i < grid->GetNumberOfCells(); ++i) {
      int ct = grid->GetCellType(i);
      if ((ct == VTK_TRIANGLE) || (ct == VTK_QUAD)) {
        all_boundary_codes.insert(cell_code->GetValue(i));
      };
    };
    if (all_on) {
      display_boundary_codes.clear();
      foreach (int bc, all_boundary_codes) {
        display_boundary_codes.insert(bc);
      };
    } else {
      QSet<int> dbcs;
      foreach (int bc, display_boundary_codes) {
        if (all_boundary_codes.contains(bc)) {
          dbcs.insert(bc);
        };
      };
      display_boundary_codes.clear();
      foreach (int bc, all_boundary_codes) {
        if (dbcs.contains(bc)) {
          display_boundary_codes.insert(bc);
        };
      };
    };
  } catch (Error err) {
    err.display();
  };
};

void GuiMainWindow::normalExtrusion()
{
  GuiNormalExtrusion extr;
  extr();
  updateBoundaryCodes(false);
  updateActors();
};

void GuiMainWindow::setAxesVisibility()
{
  if (ui.actionViewAxes->isChecked()) axes->SetVisibility(1);
  else                                axes->SetVisibility(0);
  getRenderWindow()->Render();
};

void GuiMainWindow::addVtkTypeInfo()
{
  EG_VTKSP(vtkIntArray, vtk_type);
  vtk_type->SetName("vtk_type");
  vtk_type->SetNumberOfValues(grid->GetNumberOfCells());
  for (vtkIdType cellId = 0; cellId < grid->GetNumberOfCells(); ++cellId) {
    vtk_type->SetValue(cellId, grid->GetCellType(cellId));
  };
  grid->GetCellData()->AddArray(vtk_type);
};

void GuiMainWindow::pickCallBack
(
  vtkObject *caller, 
  unsigned long int eid, 
  void *clientdata, 
  void *calldata
)
{
  caller = caller;
  eid = eid;
  clientdata = clientdata;
  calldata = calldata;
  THIS->updateActors();
  THIS->updateStatusBar();
};

vtkIdType GuiMainWindow::getPickedCell()
{
  vtkIdType picked_cell = -1;
  if (THIS->grid->GetNumberOfCells() > 0) {
    THIS->bcodes_filter->Update();
    EG_VTKDCC(vtkLongArray_t, cell_index, THIS->bcodes_filter->GetOutput(), "cell_index");
    vtkIdType cellId = THIS->picker->GetCellId();
    if (cellId >= 0) {
      if (cellId < THIS->bcodes_filter->GetOutput()->GetNumberOfCells()) {
        picked_cell = cell_index->GetValue(cellId);
      };
    };
  };
  return picked_cell;
};

void GuiMainWindow::changeSurfaceOrientation()
{
  for (vtkIdType cellId = 0; cellId < grid->GetNumberOfCells(); ++cellId) {
    vtkIdType Npts, *pts;
    grid->GetCellPoints(cellId, Npts, pts);
    QVector<vtkIdType> nodes(Npts);
    for (vtkIdType j = 0; j < Npts; ++j) nodes[j]          = pts[j];
    for (vtkIdType j = 0; j < Npts; ++j) pts[Npts - j - 1] = nodes[j];
  };
  updateActors();
};

void GuiMainWindow::checkSurfaceOrientation()
{
  CorrectSurfaceOrientation corr_surf;
  vtkIdType picked_cell = getPickedCell();
  if (picked_cell >= 0) {
    corr_surf.setStart(picked_cell);
  };
  corr_surf();
  updateActors();
};

void GuiMainWindow::improveAspectRatio()
{
  GuiImproveAspectRatio impr_ar;
  impr_ar();
  updateActors();
};

void GuiMainWindow::exportAsciiStl()
{
  StlWriter stl;
  stl();
};

void GuiMainWindow::exportBinaryStl()
{
};

void GuiMainWindow::periodicUpdate()
{
  Operation::collectGarbage(); 
  updateStatusBar();
};

void GuiMainWindow::viewXP()
{
  getRenderer()->ResetCamera();
  double x[3];
  getRenderer()->GetActiveCamera()->GetFocalPoint(x);
  x[0] += 1;
  getRenderer()->GetActiveCamera()->SetPosition(x);
  getRenderer()->GetActiveCamera()->ComputeViewPlaneNormal();
  getRenderer()->GetActiveCamera()->SetViewUp(0,1,0);
  getRenderer()->ResetCamera();
  getRenderWindow()->Render();
};

void GuiMainWindow::viewXM()
{
  getRenderer()->ResetCamera();
  double x[3];
  getRenderer()->GetActiveCamera()->GetFocalPoint(x);
  x[0] -= 1;
  getRenderer()->GetActiveCamera()->SetPosition(x);
  getRenderer()->GetActiveCamera()->ComputeViewPlaneNormal();
  getRenderer()->GetActiveCamera()->SetViewUp(0,1,0);
  getRenderer()->ResetCamera();
  getRenderWindow()->Render();
};

void GuiMainWindow::viewYP()
{
  getRenderer()->ResetCamera();
  double x[3];
  getRenderer()->GetActiveCamera()->GetFocalPoint(x);
  x[1] += 1;
  getRenderer()->GetActiveCamera()->SetPosition(x);
  getRenderer()->GetActiveCamera()->ComputeViewPlaneNormal();
  getRenderer()->GetActiveCamera()->SetViewUp(0,0,-1);
  getRenderer()->ResetCamera();
  getRenderWindow()->Render();
};

void GuiMainWindow::viewYM()
{
  getRenderer()->ResetCamera();
  double x[3];
  getRenderer()->GetActiveCamera()->GetFocalPoint(x);
  x[1] -= 1;
  getRenderer()->GetActiveCamera()->SetPosition(x);
  getRenderer()->GetActiveCamera()->ComputeViewPlaneNormal();
  getRenderer()->GetActiveCamera()->SetViewUp(0,0,-1);
  getRenderer()->ResetCamera();
  getRenderWindow()->Render();
};

void GuiMainWindow::viewZP()
{
  getRenderer()->ResetCamera();
  double x[3];
  getRenderer()->GetActiveCamera()->GetFocalPoint(x);
  x[2] += 1;
  getRenderer()->GetActiveCamera()->SetPosition(x);
  getRenderer()->GetActiveCamera()->ComputeViewPlaneNormal();
  getRenderer()->GetActiveCamera()->SetViewUp(0,1,0);
  getRenderer()->ResetCamera();
  getRenderWindow()->Render();
};

void GuiMainWindow::viewZM()
{
  getRenderer()->ResetCamera();
  double x[3];
  getRenderer()->GetActiveCamera()->GetFocalPoint(x);
  x[2] -= 1;
  getRenderer()->GetActiveCamera()->SetPosition(x);
  getRenderer()->GetActiveCamera()->ComputeViewPlaneNormal();
  getRenderer()->GetActiveCamera()->SetViewUp(0,1,0);
  getRenderer()->ResetCamera();
  getRenderWindow()->Render();
};

void GuiMainWindow::callFixSTL()
{
  FixSTL *fix;
  fix = new FixSTL();
  fix->setGui();
  (*fix)();
  updateBoundaryCodes(false);
  updateActors();
};

void GuiMainWindow::editBoundaryConditions()
{
  GuiEditBoundaryConditions editbcs;
  editbcs.setBoundaryCodes(all_boundary_codes);
  editbcs.setMap(&bcmap);
  editbcs();
};

void GuiMainWindow::MeshingOptions()
{
  cout<<"GuiMainWindow::MeshingOptions()"<<endl;
/*  GridSmoother::ReadSettings(qset);
  GridSmoother::WriteSettings(qset);*/
  GridSmoother A;
  GridSmoother::CreateDefaultSettings(qset);
  
/*  qset.beginGroup("gridsmoother");
  qset.setValue();
  qset.endGroup("gridsmoother");*/
  
/*  void setNumIterations         (int N) { N_iterations  = N; };
  void setNumRelaxations        (int N) { N_relaxations = N; };
  void setNumBoundaryCorrections(int N) { N_boundary_corrections = N; };
  void setNumSmoothLayers       (int N) { N_smooth_layers = N; };
  void setRelaxationFactor(double v) { relax = v; };
  void prismsOn() { smooth_prisms = true; };
  void prismsOff() { smooth_prisms = false; };
  void setWSharp1(double w) { w_sharp1 = w; };
  void setWSharp2(double w) { w_sharp2 = w; };*/
  
  SettingsViewer settings(&qset);
  settings.exec();
  GridSmoother B;
  cout << "---------------------------------------" << endl;
  cout << "B.funcOld()=" << B.funcOld()  << endl;
  cout << "B.funcNew()=" << B.funcNew()  << endl;
  cout << "B.get_smooth_prisms()=" << B.get_smooth_prisms() << endl;
  cout << "B.get_N_iterations()=" << B.get_N_iterations() << endl;
  cout << "B.get_relax()=" << B.get_relax() << endl;
  cout << "---------------------------------------" << endl;
  
};