#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Apply deep dark theme to match Omni-OS aesthetics
    a.setStyle(QStyleFactory::create("Fusion"));
    
    QPalette darkPalette;
    QColor bg(5, 5, 5);
    QColor highlight(0, 229, 255);
    QColor text(240, 240, 240);
    
    darkPalette.setColor(QPalette::Window, bg);
    darkPalette.setColor(QPalette::WindowText, text);
    darkPalette.setColor(QPalette::Base, QColor(15, 15, 15));
    darkPalette.setColor(QPalette::AlternateBase, bg);
    darkPalette.setColor(QPalette::ToolTipBase, text);
    darkPalette.setColor(QPalette::ToolTipText, text);
    darkPalette.setColor(QPalette::Text, text);
    darkPalette.setColor(QPalette::Button, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::ButtonText, text);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, highlight);
    darkPalette.setColor(QPalette::Highlight, highlight);
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    a.setPalette(darkPalette);
    a.setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");

    MainWindow w;
    w.resize(1280, 720);
    w.show();
    
    return a.exec();
}
