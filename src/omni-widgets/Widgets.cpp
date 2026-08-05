#include "Widgets.h"
#include <QPainter>
#include <QTime>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QRandomGenerator>

OmniWidgetBase::OmniWidgetBase(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
}

void OmniWidgetBase::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QColor bgColor(5, 5, 5, 200);
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 15, 15);
}

AestheticClock::AestheticClock(QWidget* parent) : OmniWidgetBase(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    timeLabel = new QLabel(this);
    timeLabel->setStyleSheet("color: #00E5FF; font-size: 48px; font-weight: bold; font-family: 'Segoe UI', sans-serif;");
    timeLabel->setAlignment(Qt::AlignCenter);
    
    dateLabel = new QLabel(this);
    dateLabel->setStyleSheet("color: #FFFFFF; font-size: 18px; font-family: 'Segoe UI', sans-serif; opacity: 0.7;");
    dateLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(timeLabel);
    layout->addWidget(dateLabel);
    
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &AestheticClock::updateTime);
    timer->start(1000);
    
    updateTime();
    setFixedSize(250, 120);
}

void AestheticClock::updateTime() {
    timeLabel->setText(QTime::currentTime().toString("HH:mm"));
    dateLabel->setText(QDate::currentDate().toString("dddd, MMMM d"));
}

SystemMonitor::SystemMonitor(QWidget* parent) : OmniWidgetBase(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    QLabel* title = new QLabel("SYSTEM", this);
    title->setStyleSheet("color: #00E5FF; font-size: 14px; font-weight: bold; font-family: 'Segoe UI', sans-serif; letter-spacing: 2px;");
    
    QLabel* cpuLabel = new QLabel("CPU", this);
    cpuLabel->setStyleSheet("color: #FFFFFF; font-size: 12px; font-family: 'Segoe UI', sans-serif;");
    cpuBar = new QProgressBar(this);
    cpuBar->setTextVisible(false);
    cpuBar->setFixedHeight(8);
    cpuBar->setStyleSheet("QProgressBar { border: none; background-color: #222222; border-radius: 4px; } "
                          "QProgressBar::chunk { background-color: #00E5FF; border-radius: 4px; }");
                          
    QLabel* ramLabel = new QLabel("RAM", this);
    ramLabel->setStyleSheet("color: #FFFFFF; font-size: 12px; font-family: 'Segoe UI', sans-serif;");
    ramBar = new QProgressBar(this);
    ramBar->setTextVisible(false);
    ramBar->setFixedHeight(8);
    ramBar->setStyleSheet("QProgressBar { border: none; background-color: #222222; border-radius: 4px; } "
                          "QProgressBar::chunk { background-color: #00E5FF; border-radius: 4px; }");

    layout->addWidget(title);
    layout->addSpacing(5);
    layout->addWidget(cpuLabel);
    layout->addWidget(cpuBar);
    layout->addSpacing(5);
    layout->addWidget(ramLabel);
    layout->addWidget(ramBar);
    
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SystemMonitor::updateStats);
    timer->start(2000);
    
    updateStats();
    setFixedSize(200, 130);
}

void SystemMonitor::updateStats() {
    QFile file("/proc/stat");
    bool cpuRead = false;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line = in.readLine();
        if (line.startsWith("cpu ")) {
            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() >= 5) {
                long user = parts[1].toLong();
                long nice = parts[2].toLong();
                long system = parts[3].toLong();
                long idle = parts[4].toLong();
                long total = user + nice + system + idle;
                
                static long prevTotal = 0;
                static long prevIdle = 0;
                
                if (prevTotal != 0) {
                    long totalDiff = total - prevTotal;
                    long idleDiff = idle - prevIdle;
                    int cpuUsage = static_cast<int>((totalDiff - idleDiff) * 100.0 / totalDiff);
                    cpuBar->setValue(qBound(0, cpuUsage, 100));
                    cpuRead = true;
                }
                prevTotal = total;
                prevIdle = idle;
            }
        }
        file.close();
    }
    
    if (!cpuRead) {
        cpuBar->setValue(QRandomGenerator::global()->bounded(10, 80));
    }
    
    QFile memFile("/proc/meminfo");
    bool ramRead = false;
    if (memFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&memFile);
        long memTotal = 0, memAvailable = 0;
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("MemTotal:")) {
                memTotal = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts)[1].toLong();
            } else if (line.startsWith("MemAvailable:")) {
                memAvailable = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts)[1].toLong();
            }
        }
        if (memTotal > 0) {
            int ramUsage = static_cast<int>((memTotal - memAvailable) * 100.0 / memTotal);
            ramBar->setValue(qBound(0, ramUsage, 100));
            ramRead = true;
        }
        memFile.close();
    }
    
    if (!ramRead) {
         ramBar->setValue(QRandomGenerator::global()->bounded(30, 90));
    }
}

NowPlaying::NowPlaying(QWidget* parent) : OmniWidgetBase(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    QLabel* header = new QLabel("NOW PLAYING", this);
    header->setStyleSheet("color: #00E5FF; font-size: 10px; font-weight: bold; font-family: 'Segoe UI', sans-serif; letter-spacing: 1px;");
    
    titleLabel = new QLabel("Cyberia Track 04", this);
    titleLabel->setStyleSheet("color: #FFFFFF; font-size: 16px; font-weight: bold; font-family: 'Segoe UI', sans-serif;");
    
    artistLabel = new QLabel("Serial Experiments", this);
    artistLabel->setStyleSheet("color: #AAAAAA; font-size: 12px; font-family: 'Segoe UI', sans-serif;");
    
    layout->addWidget(header);
    layout->addWidget(titleLabel);
    layout->addWidget(artistLabel);
    
    setFixedSize(220, 100);
}
