#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QSettings>
#include <QMainWindow>
#include <QMenu>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>

#include "googlesession.h"
#include "googlesync.h"

class LoginWindow : public QMainWindow
{
	Q_OBJECT
public:
             LoginWindow(QWidget *parent = 0, Qt::WindowFlags wf = 0);


private:
        QLabel* loginLabel;
        QLabel* passwLabel;
        QLabel* state;

        QLineEdit* login;
        QLineEdit* passw;

        QPushButton* startButton;
        QPushButton* exitButton;

        QCheckBox* save;
        QCheckBox* skip;

        QSettings* cfg;
        GoogleSync* sync;
private slots:
        void start();
        void stateChanged(GoogleSession::State state);
};

#endif
