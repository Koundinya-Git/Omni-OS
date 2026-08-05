#pragma once

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QVBoxLayout>

class OmniWidgetBase : public QWidget {
    Q_OBJECT
public:
    explicit OmniWidgetBase(QWidget* parent = nullptr);
protected:
    void paintEvent(QPaintEvent* event) override;
};

class AestheticClock : public OmniWidgetBase {
    Q_OBJECT
public:
    explicit AestheticClock(QWidget* parent = nullptr);
private slots:
    void updateTime();
private:
    QLabel* timeLabel;
    QLabel* dateLabel;
    QTimer* timer;
};

class SystemMonitor : public OmniWidgetBase {
    Q_OBJECT
public:
    explicit SystemMonitor(QWidget* parent = nullptr);
private slots:
    void updateStats();
private:
    QProgressBar* cpuBar;
    QProgressBar* ramBar;
    QTimer* timer;
};

class NowPlaying : public OmniWidgetBase {
    Q_OBJECT
public:
    explicit NowPlaying(QWidget* parent = nullptr);
private:
    QLabel* titleLabel;
    QLabel* artistLabel;
};
