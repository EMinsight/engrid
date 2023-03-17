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
#include <QtCore>
#include <QtGui>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <qglobal.h>

#include "guimainwindow.h"
#include "filetemplate.h"

#include "geometrytools.h"
using namespace GeometryTools;

///\todo replace with shellscript?
void appendLicense(int argc, char ** argv)
{
  int first_year = 2008;
  QString first_year_text;
  first_year_text.setNum(first_year);
  int year = QDate::currentDate().year();
  QString year_text;
  year_text.setNum(year);
  if (year-first_year == 1) {
    year_text = first_year_text + "," + year_text;
  };
  if (year-first_year > 1) {
    year_text = first_year_text + "-" + year_text;
  };
  QString year_end_text = "                                      +\n";
  if (year == first_year) {
    year_end_text = "     " + year_end_text;
  };
  for (int i = 2; i < argc; ++i) {
    QString filename(argv[i]);
    QString comment = "// ";
    QString buffer = "";
    bool script = false;
    if (filename.right(3) == ".sh")   script = true;
    if (filename.right(3) == ".py")   script = true;
    if (filename.right(5) == ".bash") script = true;
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QTextStream f(&file);
    if (script) {
      comment = "# ";
      QString line = f.readLine();
      buffer += line + "\n";
    }
    buffer += comment + "\n";
    buffer += comment + "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
    buffer += comment + "+                                                                      +\n";
    buffer += comment + "+ This file is part of enGrid.                                         +\n";
    buffer += comment + "+                                                                      +\n";
    buffer += comment + "+ Copyright " + year_text + " enGits GmbH" + year_end_text;
    buffer += comment + "+                                                                      +\n";
    buffer += comment + "+ enGrid is free software: you can redistribute it and/or modify       +\n";
    buffer += comment + "+ it under the terms of the GNU General Public License as published by +\n";
    buffer += comment + "+ the Free Software Foundation, either version 3 of the License, or    +\n";
    buffer += comment + "+ (at your option) any later version.                                  +\n";
    buffer += comment + "+                                                                      +\n";
    buffer += comment + "+ enGrid is distributed in the hope that it will be useful,            +\n";
    buffer += comment + "+ but WITHOUT ANY WARRANTY; without even the implied warranty of       +\n";
    buffer += comment + "+ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        +\n";
    buffer += comment + "+ GNU General Public License for more details.                         +\n";
    buffer += comment + "+                                                                      +\n";
    buffer += comment + "+ You should have received a copy of the GNU General Public License    +\n";
    buffer += comment + "+ along with enGrid. If not, see <http://www.gnu.org/licenses/>.       +\n";
    buffer += comment + "+                                                                      +\n";
    buffer += comment + "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
    buffer += comment + "\n";
    {
      bool src_code = false;
      while (!f.atEnd()) {
        QString line = f.readLine();
        if (!src_code) {
          if (line.size() >= comment.length()-1) {
            if (line.left(comment.length()-1) != comment.left(comment.length()-1)) {
              src_code = true;
            };
          } else {
            src_code = true;
          };
        };
        if (src_code) {
          buffer += line + "\n";
        };
      };
    };
    {
      QFile file(filename);
      file.open(QIODevice::WriteOnly);
      QTextStream f(&file);
      f << buffer;
    };
  };
}

///\todo replace with shellscript?
void makeDistribution()
{
  system ("ldd ./engrid > ldd.out");
  system ("mkdir enGrid");
  system ("cp engrid enGrid");
  {
    QFile file("ldd.out");
    file.open(QIODevice::ReadOnly);
    QTextStream f(&file);
    while (!f.atEnd()) {
      QString line = f.readLine();
      QTextStream l(&line, QIODevice::ReadOnly);
      QString word;
      l >> word;
      if (word.left(1) != "/") {
        l >> word;
        l >> word;
      };
      QString cmd = "cp " + word + " enGrid";
      system(qPrintable(cmd));
      cout << qPrintable(cmd) << endl;
    };
  };
  system ("tar czf enGrid_bin.tar.gz enGrid/*");
  system ("rm -rf enGrid");
  system ("rm ldd.out");
};

/** Message handler to allow output of Qt objects in the terminal (stderr) or the logfile/output window of engrid (stdout)
 qDebug() is used for writing custom debug output.
 qWarning() is used to report warnings and recoverable errors in your application.
 qCritical() is used for writing critical error mesages and reporting system errors.
 qFatal() is used for writing fatal error messages shortly before exiting.
 */
void engridMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  if (type == QtDebugMsg) {
    fprintf(stdout, "%s", qPrintable(msg));
  }
  if (type == QtWarningMsg) {
    fprintf(stderr, "%s", qPrintable(msg));
  }
  if (type == QtCriticalMsg) {
    fprintf(stderr, "Critical: %s\n", qPrintable(msg));
  }
  if (type == QtFatalMsg) {
    fprintf(stderr, "Fatal: %s\n", qPrintable(msg));
    abort();
  }
}

double getNumber(QString text)
{
  double myNumber = 0;
  while (true) {
    qDebug() << text;
    string input = "";
    getline(cin, input);

    // This code converts from string to number safely.
    stringstream myStream(input);
    if (myStream >> myNumber)
      break;
    qDebug() << "Invalid number, please try again" << endl;
  }
  return myNumber;
}

int main( int argc, char ** argv )
{

#ifdef QT_DEBUG
  //omp_set_num_threads(1);
#endif
  qInstallMessageHandler(engridMessageHandler);
  Q_INIT_RESOURCE(engrid);
  int app_result=0;

  ///\todo use gnu getopt ? Check windows/mac compatibility.
  if (argc > 1) {
    if (QString(argv[1]) == QString("-h")) {
      QFileInfo file_info(argv[0]);
      cout<<"Usage:"<<endl;
      cout<<qPrintable(file_info.fileName())<<" : start engrid"<<endl;
      cout<<qPrintable(file_info.fileName())<<" -f FILE : start engrid and open FILE"<<endl;
      cout<<qPrintable(file_info.fileName())<<" -h : Display usage instructions"<<endl;
      cout<<qPrintable(file_info.fileName())<<" -appendlic FILE1 FILE2 ...: Append license to files"<<endl;
      cout<<qPrintable(file_info.fileName())<<" -distbin : Create binary distribution"<<endl;
      exit(0);
    }
    if (QString(argv[1]) == QString("-appendlic")) {
      appendLicense(argc, argv);
    }
    if (QString(argv[1]) == QString("-distbin")) {
      makeDistribution();
    }
    if (QString(argv[1]) == QString("-f") && argc == 3) {
      QApplication a( argc, argv );
      QString filename = QString(argv[2]);
      GuiMainWindow w(filename);
      w.show();
      a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
      app_result = a.exec();
    }
  } else {
    QApplication a( argc, argv );
    GuiMainWindow w;
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    app_result = a.exec();
  }

  return app_result;
}
