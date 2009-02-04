#include "loginwindow.h"
#include <QApplication>

#include <QVBoxLayout>
#include <QGridLayout>

#include "googlesync.h"
#include "googlesession.h"

LoginWindow::LoginWindow(QWidget *parent, Qt::WindowFlags wf) 
  : QMainWindow(parent, wf)
{

  cfg = new QSettings ("ezxdev.org","google");

  QGridLayout *grid = new QGridLayout;

  loginLabel = new QLabel("Email",    this);
  passwLabel = new QLabel("Password", this);
  state      = new QLabel(this);


  login = new QLineEdit(cfg->value("login/email").toString(),this );
  passw = new QLineEdit(cfg->value("login/password").toString(),this);

  passw->setEchoMode(QLineEdit::Password);

  startButton =   new QPushButton("Start sync",this);
  exitButton  =   new QPushButton("Exit",this);

  save = new QCheckBox("Save password", this);
  save->setCheckState( (Qt::CheckState) cfg->value("login/save" ).toInt() );

  skip = new QCheckBox("Skip without numbers", this);
  skip->setCheckState( (Qt::CheckState) cfg->value("login/skip" ).toInt() );

  connect( startButton, SIGNAL( clicked() ),
      SLOT( start () ) );

  connect( exitButton,  SIGNAL( clicked() ),
                  qApp, SLOT( quit () ) );

  grid->addWidget(loginLabel,0,0);
  grid->addWidget(login,0,1);
  grid->addWidget(passwLabel,1,0);
  grid->addWidget(passw,1,1);


  QVBoxLayout *layout = new QVBoxLayout;
  layout->addLayout(grid);
  layout->addWidget(state);
  layout->addWidget(skip);
  layout->addWidget(save);
  layout->addWidget(startButton);
  layout->addWidget(exitButton);

  QWidget *central = new QWidget();
  central->setLayout(layout);

  setCentralWidget(central);

  sync = new GoogleSync();

}


void LoginWindow::start() {
  // Qt::Checked  
  cfg->setValue("login/save", (int) save->checkState() );
  cfg->setValue("login/skip", (int) skip->checkState() );
  cfg->setValue("login/email", login->text() );
  if (save->checkState() == Qt::Checked )
    cfg->setValue("login/password", passw->text() );

  connect(sync, SIGNAL(stateChanged(GoogleSession::State) ),
      this, SLOT(stateChanged(GoogleSession::State) ) );

  sync->start( 
      login->text() , 
      passw->text() ,
      (bool) skip->checkState()
  );
}


void LoginWindow::stateChanged(GoogleSession::State s) {
  state->setText(GoogleSession::stateName(s) );
}
