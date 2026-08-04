#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QJsonArray>
#include <QVariantAnimation>
#include <vector>

struct Particle {
    float x, y;
    float speed;
    float length;
    float opacity;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateParticles();
    void typeNextIntroChar();
    void startHardwareScan();
    void typeNextHardwareScanChar();
    void transitionToChat();
    void onChatReturnPressed();
    void onOllamaResponse(QNetworkReply* reply);
    void onActionBridgeResponse(QNetworkReply* reply);

private:
    void initializeParticles();
    void setupUI();
    void appendChatMessage(const QString& sender, const QString& message, const QString& color);
    void sendActionBridge(const QStringList& actions);

    QString fetchCPUInfo();
    QString fetchGPUInfo();
    QString fetchRAMInfo();
    QString fetchTierInfo();

    std::vector<Particle> particles;
    QTimer particleTimer;

    QString introTextFull;
    QString introTextCurrent;
    int introTextIndex;
    QTimer introTimer;
    bool showCursor;
    bool phase1Active;
    float introOpacity;
    QVariantAnimation* fadeOutAnimation;

    QStringList scanLinesFull;
    QStringList scanLinesRendered;
    QString scanLineCurrent;
    int currentScanLineIndex;
    int currentScanCharIndex;
    QTimer hardwareTimer;
    bool hardwareScanActive;
    bool hardwareScanComplete;

    QWidget* chatContainer;
    QTextEdit* chatHistory;
    QLineEdit* chatInput;
    bool phase2Active;
    
    QMediaPlayer* mediaPlayer;
    QAudioOutput* audioOutput;
    
    QNetworkAccessManager* networkManager;
    QNetworkAccessManager* actionNetworkManager;
    
    QJsonArray chatHistoryJson;
};

#endif
