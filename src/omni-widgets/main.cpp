#include <QApplication>
#include "WidgetEngine.h"

int main(int argc, char* argv[]) {
    // Enable Wayland LayerShell specific application attributes
    qputenv("QT_QPA_PLATFORM", "wayland");

    QApplication app(argc, argv);
    app.setApplicationName("omni-widgets");
    app.setOrganizationName("Omni-OS");

    WidgetEngine engine;
    engine.initialize();

    return app.exec();
}
