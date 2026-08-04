#include "mainwindow.h"
#include <QApplication>
#include <QPalette>
#include <QColor>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("Fusion");

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(5, 5, 5));
    darkPalette.setColor(QPalette::WindowText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Base, QColor(10, 10, 10));
    darkPalette.setColor(QPalette::AlternateBase, QColor(5, 5, 5));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::ToolTipText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Text, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Button, QColor(5, 5, 5));
    darkPalette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(0, 229, 255));
    darkPalette.setColor(QPalette::Highlight, QColor(0, 229, 255));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    a.setPalette(darkPalette);

    MainWindow w;
    w.showFullScreen();
    return a.exec();
}
