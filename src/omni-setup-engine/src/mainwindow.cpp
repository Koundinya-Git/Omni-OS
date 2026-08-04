#include "mainwindow.h"
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QProcess>
#include <QTextStream>
#include <QEventLoop>
#include <QTimer>
#include <QKeyEvent>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      introTextIndex(0),
      showCursor(true),
      phase1Active(true),
      introOpacity(1.0f),
      currentScanLineIndex(0),
      currentScanCharIndex(0),
      hardwareScanActive(false),
      hardwareScanComplete(false),
      phase2Active(false)
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    central->setStyleSheet("background-color: transparent;");

    networkManager = new QNetworkAccessManager(this);
    actionNetworkManager = new QNetworkAccessManager(this);

    mediaPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    mediaPlayer->setAudioOutput(audioOutput);
    mediaPlayer->setSource(QUrl("qrc:/assets/ambient.ogg"));
    mediaPlayer->setLoops(QMediaPlayer::Infinite);
    audioOutput->setVolume(0.5f);
    
    setupUI();
    
    initializeParticles();
    particleTimer.setInterval(16);
    connect(&particleTimer, &QTimer::timeout, this, &MainWindow::updateParticles);
    particleTimer.start();

    introTextFull = "Initializing Neural Core...\n"
                    "Establishing Secure Link...\n\n"
                    "This is Omni-OS.\n"
                    "A sovereign operating system.\n"
                    "Built for those who refuse to compromise.\n\n"
                    "Your hardware. Your data. Your rules.\n"
                    "No telemetry. No surveillance. No chains.\n\n"
                    "Powered by local AI that answers only to you.";
    
    introTimer.setInterval(45);
    connect(&introTimer, &QTimer::timeout, this, &MainWindow::typeNextIntroChar);
    
    hardwareTimer.setInterval(20);
    connect(&hardwareTimer, &QTimer::timeout, this, &MainWindow::typeNextHardwareScanChar);

    fadeOutAnimation = new QVariantAnimation(this);
    fadeOutAnimation->setDuration(2000);
    fadeOutAnimation->setStartValue(1.0f);
    fadeOutAnimation->setEndValue(0.0f);
    connect(fadeOutAnimation, &QVariantAnimation::valueChanged, [this](const QVariant& val) {
        introOpacity = val.toFloat();
        update();
    });
    connect(fadeOutAnimation, &QVariantAnimation::finished, this, &MainWindow::transitionToChat);

    QTimer::singleShot(500, this, [this]() {
        mediaPlayer->play();
        introTimer.start();
    });
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    chatContainer = new QWidget(this);
    chatContainer->hide();
    
    QVBoxLayout* layout = new QVBoxLayout(chatContainer);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);
    
    chatContainer->setStyleSheet(
        "QWidget { "
        "  background-color: rgba(10, 10, 10, 242); "
        "  border: 1px solid #00e5ff; "
        "  border-radius: 10px; "
        "}"
    );

    chatHistory = new QTextEdit(chatContainer);
    chatHistory->setReadOnly(true);
    chatHistory->setStyleSheet(
        "QTextEdit { "
        "  background-color: transparent; "
        "  border: none; "
        "  color: white; "
        "  font-family: 'Courier New', monospace; "
        "  font-size: 16px; "
        "}"
    );
    
    chatInput = new QLineEdit(chatContainer);
    chatInput->setPlaceholderText("Type here...");
    chatInput->setStyleSheet(
        "QLineEdit { "
        "  background-color: rgba(0, 0, 0, 150); "
        "  border: 1px solid #00e5ff; "
        "  border-radius: 5px; "
        "  color: white; "
        "  padding: 10px; "
        "  font-family: 'Courier New', monospace; "
        "  font-size: 16px; "
        "}"
        "QLineEdit:focus { border: 2px solid #00e5ff; }"
        "QLineEdit:disabled { border: 1px solid #555; color: #888; }"
    );
    
    QGraphicsDropShadowEffect* inputGlow = new QGraphicsDropShadowEffect(this);
    inputGlow->setBlurRadius(10);
    inputGlow->setColor(QColor("#00e5ff"));
    inputGlow->setOffset(0, 0);
    chatInput->setGraphicsEffect(inputGlow);
    
    layout->addWidget(chatHistory);
    layout->addWidget(chatInput);
    
    connect(chatInput, &QLineEdit::returnPressed, this, &MainWindow::onChatReturnPressed);
    
    QJsonObject sysMsg;
    sysMsg["role"] = "system";
    sysMsg["content"] = "You are Omni, a local AI. Interview the user about their profession, preferred apps, browser, aesthetic preferences, and persona. Keep responses concise. Output action tags like [INSTALL:pkg1,pkg2], [PERSONA:style], [BLOCK:site1,site2] inline where applicable. When you have enough information and the setup is finalized, output the [DONE] tag to exit.";
    chatHistoryJson.append(sysMsg);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (chatContainer) {
        int w = width() * 0.6;
        int h = height() * 0.7;
        chatContainer->setGeometry((width() - w) / 2, (height() - h) / 2, w, h);
    }
}

void MainWindow::initializeParticles()
{
    particles.clear();
    for (int i = 0; i < 150; ++i) {
        Particle p;
        p.x = QRandomGenerator::global()->generateDouble() * 4000 - 1000;
        p.y = QRandomGenerator::global()->generateDouble() * 3000 - 1000;
        p.speed = 1.0f + QRandomGenerator::global()->generateDouble() * 4.0f;
        p.length = 10.0f + QRandomGenerator::global()->generateDouble() * 40.0f;
        p.opacity = 0.1f + QRandomGenerator::global()->generateDouble() * 0.6f;
        particles.push_back(p);
    }
}

void MainWindow::updateParticles()
{
    for (auto& p : particles) {
        p.y += p.speed;
        if (p.y > height() + 100) {
            p.y = -50;
            p.x = QRandomGenerator::global()->generateDouble() * width();
        }
    }
    
    if (introTimer.isActive()) {
        showCursor = (QDateTime::currentMSecsSinceEpoch() / 500) % 2 == 0;
    } else {
        showCursor = true;
    }
    
    update();
}

void MainWindow::typeNextIntroChar()
{
    if (introTextIndex < introTextFull.length()) {
        introTextCurrent += introTextFull[introTextIndex];
        introTextIndex++;
    } else {
        introTimer.stop();
        QTimer::singleShot(2000, this, &MainWindow::startHardwareScan);
    }
}

QString MainWindow::fetchCPUInfo() {
    QFile file("/proc/cpuinfo");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("model name")) {
                return line.section(':', 1).trimmed();
            }
        }
    }
    return "Neural Processing Unit / Simulated";
}

QString MainWindow::fetchGPUInfo() {
    QProcess process;
    process.start("sh", QStringList() << "-c" << "lspci | grep VGA");
    process.waitForFinished();
    QString output = process.readAllStandardOutput().trimmed();
    if (!output.isEmpty()) {
        int idx = output.indexOf("VGA compatible controller:");
        if (idx != -1) return output.mid(idx + 26).trimmed();
        return output;
    }
    return "Neural Display Adapter";
}

QString MainWindow::fetchRAMInfo() {
    QFile file("/proc/meminfo");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("MemTotal:")) return line.section(':', 1).trimmed();
        }
    }
    return "32768 kB";
}

QString MainWindow::fetchTierInfo() {
    QFile file("/etc/omni/hw-tier");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString(file.readAll()).trimmed();
    }
    return "X (Unclassified)";
}

void MainWindow::startHardwareScan()
{
    scanLinesFull.append(QString("[SCAN] CPU: %1").arg(fetchCPUInfo()));
    scanLinesFull.append(QString("[SCAN] GPU: %1").arg(fetchGPUInfo()));
    scanLinesFull.append(QString("[SCAN] RAM: %1").arg(fetchRAMInfo()));
    scanLinesFull.append(QString("[SCAN] TIER: %1").arg(fetchTierInfo()));
    
    hardwareScanActive = true;
    hardwareTimer.start();
}

void MainWindow::typeNextHardwareScanChar()
{
    if (currentScanLineIndex < scanLinesFull.size()) {
        QString fullLine = scanLinesFull[currentScanLineIndex];
        if (currentScanCharIndex < fullLine.length()) {
            scanLineCurrent += fullLine[currentScanCharIndex];
            currentScanCharIndex++;
        } else {
            scanLinesRendered.append(scanLineCurrent);
            scanLineCurrent.clear();
            currentScanCharIndex = 0;
            currentScanLineIndex++;
            
            hardwareTimer.stop();
            QTimer::singleShot(300, this, [this]() {
                hardwareTimer.start();
            });
        }
    } else {
        hardwareTimer.stop();
        hardwareScanComplete = true;
        scanLinesRendered.append(QString("HARDWARE PROFILE LOCKED. TIER %1 CONFIRMED.").arg(fetchTierInfo()));
        
        QTimer::singleShot(2000, this, [this]() {
            fadeOutAnimation->start();
        });
    }
}

void MainWindow::transitionToChat()
{
    phase1Active = false;
    hardwareScanActive = false;
    phase2Active = true;
    
    chatContainer->show();
    
    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(chatContainer);
    chatContainer->setGraphicsEffect(opacityEffect);
    
    QPropertyAnimation* anim = new QPropertyAnimation(opacityEffect, "opacity");
    anim->setDuration(1500);
    anim->setStartValue(0.0f);
    anim->setEndValue(1.0f);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    
    QString initialMsg = "Neural link established. I am Omni - your local AI. Let us configure your system. What do you do - student, developer, creative, or something else?";
    
    QJsonObject aiJsonMsg;
    aiJsonMsg["role"] = "assistant";
    aiJsonMsg["content"] = initialMsg;
    chatHistoryJson.append(aiJsonMsg);
    
    appendChatMessage("Omni", initialMsg, "#00e5ff");
    chatInput->setFocus();
}

void MainWindow::appendChatMessage(const QString& sender, const QString& message, const QString& color)
{
    QString html = QString("<b style='color:%1;'>[%2]:</b> <span style='color:%1;'>%3</span><br><br>")
                   .arg(color, sender, message);
    chatHistory->append(html);
}

void MainWindow::onChatReturnPressed()
{
    QString input = chatInput->text().trimmed();
    if (input.isEmpty()) return;
    
    appendChatMessage("User", input, "white");
    chatInput->clear();
    chatInput->setDisabled(true);
    chatInput->setPlaceholderText("Thinking...");
    
    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = input;
    chatHistoryJson.append(userMsg);
    
    QJsonObject requestBody;
    requestBody["model"] = "llama3.2:1b";
    requestBody["stream"] = false;
    requestBody["messages"] = chatHistoryJson;
    
    QNetworkRequest request(QUrl("http://localhost:11434/api/chat"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = networkManager->post(request, QJsonDocument(requestBody).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onOllamaResponse(reply); });
}

void MainWindow::onOllamaResponse(QNetworkReply* reply)
{
    chatInput->setEnabled(true);
    chatInput->setPlaceholderText("Type here...");
    chatInput->setFocus();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObj = jsonDoc.object();
        
        if (jsonObj.contains("message")) {
            QJsonObject messageObj = jsonObj["message"].toObject();
            QString content = messageObj["content"].toString();
            
            chatHistoryJson.append(messageObj);
            
            QRegularExpression re("\\[(.*?)\\]");
            QRegularExpressionMatchIterator i = re.globalMatch(content);
            QStringList actions;
            while (i.hasNext()) {
                QRegularExpressionMatch match = i.next();
                actions << match.captured(1);
            }
            
            QString displayText = content;
            displayText.remove(re);
            displayText = displayText.trimmed();
            
            if (!displayText.isEmpty()) {
                appendChatMessage("Omni", displayText, "#00e5ff");
            }
            
            if (!actions.isEmpty()) {
                sendActionBridge(actions);
            }
        }
    } else {
        appendChatMessage("System", "Error communicating with Neural Core (Ollama not running?).", "red");
    }
    reply->deleteLater();
}

void MainWindow::sendActionBridge(const QStringList& actions)
{
    for (const QString& tag : actions) {
        QJsonObject payload;
        QNetworkRequest request(QUrl("http://localhost:9191/action"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        if (tag.startsWith("INSTALL:")) {
            QStringList pkgs = tag.mid(8).split(',', Qt::SkipEmptyParts);
            QJsonArray pkgArray;
            for (const QString& p : pkgs) pkgArray.append(p.trimmed());
            payload["type"] = "install";
            payload["packages"] = pkgArray;
        } else if (tag.startsWith("PERSONA:")) {
            payload["type"] = "persona";
            payload["style"] = tag.mid(8).trimmed();
        } else if (tag.startsWith("BLOCK:")) {
            QStringList sites = tag.mid(6).split(',', Qt::SkipEmptyParts);
            QJsonArray siteArray;
            for (const QString& s : sites) siteArray.append(s.trimmed());
            payload["type"] = "block";
            payload["sites"] = siteArray;
        } else if (tag == "DONE") {
            QApplication::quit();
            return;
        } else {
            continue;
        }

        QNetworkReply* reply = actionNetworkManager->post(request, QJsonDocument(payload).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply]() { onActionBridgeResponse(reply); });
    }
}

void MainWindow::onActionBridgeResponse(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
    }
    reply->deleteLater();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    painter.fillRect(rect(), QColor(5, 5, 5));
    
    painter.setPen(Qt::NoPen);
    for (const auto& p : particles) {
        QLinearGradient grad(p.x, p.y, p.x, p.y - p.length);
        QColor col(0, 229, 255, static_cast<int>(p.opacity * 255));
        grad.setColorAt(0, col);
        grad.setColorAt(1, Qt::transparent);
        painter.setBrush(grad);
        painter.drawRect(QRectF(p.x, p.y - p.length, 2, p.length));
    }
    
    if (introOpacity > 0.0f) {
        painter.setOpacity(introOpacity);
        
        QFont font("Courier New", 24, QFont::Bold);
        painter.setFont(font);
        
        QString textToDraw = introTextCurrent;
        if (showCursor && !hardwareScanComplete) {
            textToDraw += QString::fromUtf8("\xe2\x96\x88");
        }
        
        painter.setPen(QColor(0, 229, 255, 100));
        painter.drawText(rect().adjusted(2, 2, 2, 2), Qt::AlignCenter, textToDraw);
        painter.setPen(QColor(0, 229, 255));
        painter.drawText(rect(), Qt::AlignCenter, textToDraw);
        
        if (hardwareScanActive || hardwareScanComplete) {
            QFont hwFont("Courier New", 16, QFont::Bold);
            painter.setFont(hwFont);
            
            int startY = height() / 2 + 150;
            int lineHeight = 30;
            
            int y = startY;
            for (const QString& line : scanLinesRendered) {
                if (line.startsWith("HARDWARE PROFILE LOCKED")) {
                    painter.setPen(QColor(0, 229, 255, 150));
                    painter.drawText(rect().adjusted(2, y+2, 2, y+2), Qt::AlignHCenter | Qt::AlignTop, line);
                    painter.setPen(QColor(0, 229, 255));
                    painter.drawText(rect().adjusted(0, y, 0, y), Qt::AlignHCenter | Qt::AlignTop, line);
                } else {
                    int scanIdx = line.indexOf(']');
                    if (scanIdx != -1) {
                        painter.setPen(QColor(0, 255, 65));
                        painter.drawText(width()/2 - 300, y, line.left(scanIdx + 1));
                        painter.setPen(Qt::white);
                        painter.drawText(width()/2 - 300 + painter.fontMetrics().horizontalAdvance(line.left(scanIdx + 1) + " "), y, line.mid(scanIdx + 2));
                    } else {
                        painter.setPen(Qt::white);
                        painter.drawText(width()/2 - 300, y, line);
                    }
                }
                y += lineHeight;
            }
            
            if (!scanLineCurrent.isEmpty()) {
                QString line = scanLineCurrent;
                int scanIdx = line.indexOf(']');
                if (scanIdx != -1) {
                    painter.setPen(QColor(0, 255, 65));
                    painter.drawText(width()/2 - 300, y, line.left(scanIdx + 1));
                    painter.setPen(Qt::white);
                    painter.drawText(width()/2 - 300 + painter.fontMetrics().horizontalAdvance(line.left(scanIdx + 1) + " "), y, line.mid(scanIdx + 2) + QString::fromUtf8("\xe2\x96\x88"));
                } else {
                    painter.setPen(Qt::white);
                    painter.drawText(width()/2 - 300, y, line + QString::fromUtf8("\xe2\x96\x88"));
                }
            }
        }
    }
}
