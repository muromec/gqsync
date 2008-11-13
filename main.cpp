#include <QApplication>

#include "googlesession.h"
#include "googlesync.h"


int main(int argc, char **argv) {


  QApplication app(argc, argv);

  GoogleSync sync;
  sync.start(argv[1], argv[2]);

  return app.exec();
}
