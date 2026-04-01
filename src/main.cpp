#include <cstdio>

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFontDatabase>
#include <QIcon>

#ifdef __linux__
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#endif

#include "gui/MainWindow.h"
#include "gui/WelcomeScreen.h"

int main(int argc, char *argv[])
{
    // Redirect stderr and stdout to log files (append mode), matching original Java behavior
    FILE *f;
    f = freopen("swaedit_err.log", "a", stderr);
    (void)f;
    f = freopen("swaedit_out.log", "a", stdout);
    (void)f;

#ifdef __linux__
    // If a bundled fonts.conf exists alongside the binary, tell fontconfig to use it.
    // This must happen before QApplication is created (fontconfig initializes during construction).
    {
        char exePath[4096] = {};
        ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
        if (len > 0) {
            exePath[len] = '\0';
            char *slash = std::strrchr(exePath, '/');
            if (slash) {
                *slash = '\0';
                char fontsConf[4096];
                std::snprintf(fontsConf, sizeof(fontsConf), "%s/fonts.conf", exePath);
                if (access(fontsConf, F_OK) == 0)
                    setenv("FONTCONFIG_PATH", exePath, 0);
            }
        }
    }
#endif

    QApplication app(argc, argv);

#ifdef __linux__
    // Register bundled DejaVu Sans as a fallback font (if present)
    {
        QString fontsDir = QCoreApplication::applicationDirPath() + "/fonts";
        if (QDir(fontsDir).exists()) {
            QFontDatabase::addApplicationFont(fontsDir + "/DejaVuSans.ttf");
            QFontDatabase::addApplicationFont(fontsDir + "/DejaVuSans-Bold.ttf");
        }
    }
#endif

    QString imgDir = QCoreApplication::applicationDirPath() + "/images/";
    app.setWindowIcon(QIcon(imgDir + "icon.png"));

    MainWindow window;

    WelcomeScreen *welcomeScreen = new WelcomeScreen(&window);
    QObject::connect(welcomeScreen, &WelcomeScreen::closed,
                     &window, &MainWindow::splashScreenClosed);
    welcomeScreen->showNow();

    return app.exec();
}
