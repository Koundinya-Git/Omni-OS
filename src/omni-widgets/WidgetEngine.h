#pragma once

#include <QObject>
#include <QVector>
#include <QJsonObject>
#include "Widgets.h"

class WidgetEngine : public QObject {
    Q_OBJECT
public:
    explicit WidgetEngine(QObject* parent = nullptr);
    void initialize();
    
private:
    void loadConfiguration();
    void spawnWidget(const QString& type, int x, int y);
    void setupLayerShell(QWidget* widget, int x, int y);

    QVector<QWidget*> activeWidgets;
    QJsonObject config;
};
