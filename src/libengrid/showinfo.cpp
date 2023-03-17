// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +                                                                      +
// + This file is part of enGrid.                                         +
// +                                                                      +
// + Copyright 2008-2014 enGits GmbH                                      +
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
#include "showinfo.h"

#include "engrid.h"
#include "insertpoints.h"

#include "guimainwindow.h"

#include "geometrytools.h"
using namespace GeometryTools;

#include <vtkCharArray.h>

ShowInfo::ShowInfo(bool b, vtkIdType P, vtkIdType C) : SurfaceOperation()
{
  CellInfo = b;
  PickedPoint = P;
  PickedCell = C;
}

void ShowInfo::operate()
{
  l2g_t  nodes = getPartNodes();
  g2l_t _nodes = getPartLocalNodes();
  l2g_t  cells = getPartCells();
  g2l_t _cells = getPartLocalCells();
  l2l_t  c2c   = getPartC2C();
  l2l_t  n2n   = getPartN2N();
  l2l_t  n2c   = getPartN2C();

  int N_cells=m_Grid->GetNumberOfCells();
  int N_points=m_Grid->GetNumberOfPoints();
  if(CellInfo) {
    if(PickedCell>=0 && PickedCell<N_cells) {
      cout<<"=== INFO ON CELL "<<PickedCell<<" ==="<<endl;
      QVector<vtkIdType> absolute_c2c;
      foreach(int i, c2c[_cells[PickedCell]]) {
        if(i!=-1) absolute_c2c.push_back(cells[i]);
      }
      
      cout<<"absolute_c2c(PickedCell)="<<absolute_c2c<<endl;
      EG_GET_CELL(PickedCell, m_Grid);
      cout<<"pts=";
      for (int i = 0; i < num_pts; i++)
        cout << pts[i] << " ";
      cout<<endl;
      cout<<"coords:"<<endl;
      for (int i = 0; i < num_pts; i++) {
        vec3_t X;
        m_Grid->GetPoint(pts[i],X.data());
        cout<<"pts["<<i<<"]="<<X<<endl;
      }
      cout<<"area="<<cellVA(m_Grid,PickedCell)<<endl;
      //cout<<"Q_L("<<PickedCell<<")="<<Q_L(PickedCell)<<endl;
      cout<<"====================================="<<endl;
    } else {
      cout<<"Invalid cell"<<endl;
    }
  } else {
    if(PickedPoint>=0 && PickedPoint<N_points) {
      cout<<"=== INFO ON POINT "<<PickedPoint<<" ==="<<endl;
      
      QSet<vtkIdType> absolute_n2n;
      foreach(int i, n2n[_nodes[PickedPoint]]) {
        if(i!=-1) absolute_n2n.insert(nodes[i]);
      }
      cout<<"absolute_n2n(PickedPoint)="<<absolute_n2n<<endl;
      
      QSet<vtkIdType> absolute_n2c;
      foreach(int i, n2c[_nodes[PickedPoint]]) {
        if(i!=-1) absolute_n2c.insert(cells[i]);
      }
      cout<<"absolute_n2c(PickedPoint)="<<absolute_n2c<<endl;
      
      EG_VTKDCN(vtkCharArray, node_type, m_Grid, "node_type");//node type
      cout<<"node_type="<<VertexType2Str(node_type->GetValue(PickedPoint))<<endl;
      vec3_t X;
      m_Grid->GetPoint(PickedPoint,X.data());
      cout<<"X="<<X<<endl;
      cout<<"desiredEdgeLength("<<PickedPoint<<")="<<desiredEdgeLength(PickedPoint)<<endl;
      //cout<<"Q_L1("<<PickedPoint<<")="<<Q_L1(PickedPoint)<<endl;
      //cout<<"Q_L2("<<PickedPoint<<")="<<Q_L2(PickedPoint)<<endl;
      cout<<"currentVertexAvgDist("<<PickedPoint<<")="<<currentVertexAvgDist(PickedPoint)<<endl;

      setBoundaryCodes(GuiMainWindow::pointer()->getAllBoundaryCodes());
      updateNodeInfo();
      QVector <vtkIdType> PSP_vector = getPotentialSnapPoints( PickedPoint );
      qDebug()<<"PSP_vector="<<PSP_vector;

      cout<<"====================================="<<endl;
    } else {
      cout<<"Invalid point"<<endl;
    }
  }
}
