#include <cstdio>

#include <QApplication>
#include <QCoreApplication>
#include <QIcon>

#include "gui/MainWindow.h"
#include "gui/WelcomeScreen.h"

int main(int argc, char *argv[])
{
    // Redirect stderr and stdout to log files (append mode), matching original Java behavior
    (void)freopen("swaedit_err.log", "a", stderr);
    (void)freopen("swaedit_out.log", "a", stdout);

    QApplication app(argc, argv);

    QString imgDir = QCoreApplication::applicationDirPath() + "/images/";
    app.setWindowIcon(QIcon(imgDir + "icon.png"));

    MainWindow window;

    WelcomeScreen *welcomeScreen = new WelcomeScreen(&window);
    QObject::connect(welcomeScreen, &WelcomeScreen::closed,
                     &window, &MainWindow::splashScreenClosed);
    welcomeScreen->showNow();

    return app.exec();
}
