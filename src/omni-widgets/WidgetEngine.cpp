#include "WidgetEngine.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDebug>
#include <QWindow>
#include <QMargins>
#include <LayerShellQt/Window>

WidgetEngine::WidgetEngine(QObject* parent) : QObject(parent) {
}

void WidgetEngine::initialize() {
    loadConfiguration();
    
    QJsonArray widgetsArray = config["widgets"].toArray();
    if (widgetsArray.isEmpty()) {
        qWarning() << "No widgets defined in config, loading defaults.";
        spawnWidget("AestheticClock", 100, 100);
        spawnWidget("SystemMonitor", 100, 240);
        spawnWidget("NowPlaying", 100, 390);
    } else {
        for (const QJsonValue& val : widgetsArray) {
            QJsonObject wObj = val.toObject();
            spawnWidget(wObj["type"].toString(), wObj["x"].toInt(100), wObj["y"].toInt(100));
        }
    }
}

void WidgetEngine::loadConfiguration() {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/omni-widgets/config.json";
    QFile configFile(configPath);
    
    if (configFile.open(QIODevice::ReadOnly)) {
        QByteArray data = configFile.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull() && doc.isObject()) {
            config = doc.object();
        }
        configFile.close();
    } else {
        qWarning() << "Could not open config file at" << configPath << ", using defaults.";
    }
}

void WidgetEngine::spawnWidget(const QString& type, int x, int y) {
    QWidget* widget = nullptr;
    
    if (type == "AestheticClock") {
        widget = new AestheticClock();
    } else if (type == "SystemMonitor") {
        widget = new SystemMonitor();
    } else if (type == "NowPlaying") {
        widget = new NowPlaying();
    } else {
        qWarning() << "Unknown widget type:" << type;
        return;
    }
    
    setupLayerShell(widget, x, y);
    widget->show();
    activeWidgets.append(widget);
}

void WidgetEngine::setupLayerShell(QWidget* widget, int x, int y) {
    widget->setAttribute(Qt::WA_NativeWindow);
    widget->winId(); // Force creation of the native window
    QWindow* window = widget->windowHandle();
    if (window) {
        LayerShellQt::Window* lsWindow = LayerShellQt::Window::get(window);
        if (lsWindow) {
            lsWindow->setLayer(LayerShellQt::Window::LayerBottom);
            lsWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
            
            // Anchor to Top Left and use margins for x, y positioning
            lsWindow->setAnchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorLeft);
            
            QMargins margins(x, y, 0, 0);
            lsWindow->setMargins(margins);
        }
    }
}
