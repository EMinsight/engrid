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
#include "error.h"
#include <QMessageBox>
#include <QtDebug>

Error::Error()
{
  type = ExitProgram;
  text = "unknown error";
}

void Error::setType(error_t a_type)
{
  type = a_type;
}

void Error::setText(QString a_text)
{
  auto dbg_text = a_text.toStdString();
  text = a_text;
}

void Error::display()
{
  if (type == CancelOperation) {
    QMessageBox::information(NULL, "Operation canceled!", "The operation has been canceled by a user request.");
  } else {
    QMessageBox::critical(NULL,"Error",text);
    if (type == ExitProgram) exit(EXIT_FAILURE);
  }
}


