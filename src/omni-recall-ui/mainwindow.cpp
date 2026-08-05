#include "mainwindow.h"
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QInputDialog>
#include <QMessageBox>
#include <QResizeEvent>
#include <QVariant>
#include <QDebug>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_minTimestamp(0),
      m_maxTimestamp(100),
      m_currentTimestamp(0)
{
    setupUi();
    connectToDatabase();
    loadInitialData();
}

MainWindow::~MainWindow()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

void MainWindow::setupUi()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    
    // Left/Center side (Image and slider)
    QVBoxLayout *leftLayout = new QVBoxLayout();
    
    m_imageLabel = new QLabel("No Frame Loaded", this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setStyleSheet("QLabel { background-color: #050505; color: #666666; border: 1px solid #333333; }");
    m_imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    m_timelineSlider = new QSlider(Qt::Horizontal, this);
    m_timelineSlider->setStyleSheet(
        "QSlider::groove:horizontal { border: 1px solid #333; height: 8px; background: #222; margin: 2px 0; border-radius: 4px; }"
        "QSlider::handle:horizontal { background: #00e5ff; border: 1px solid #00e5ff; width: 16px; margin: -4px 0; border-radius: 8px; }"
    );
    
    m_searchBar = new QLineEdit(this);
    m_searchBar->setPlaceholderText("Search frames semantically...");
    m_searchBar->setStyleSheet("QLineEdit { background: #111; color: #00e5ff; border: 1px solid #333; padding: 8px; border-radius: 4px; }");
    
    leftLayout->addWidget(m_searchBar, 0);
    leftLayout->addWidget(m_imageLabel, 1);
    leftLayout->addWidget(m_timelineSlider, 0);
    
    // Right sidebar
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setAlignment(Qt::AlignTop);
    rightLayout->setContentsMargins(10, 10, 10, 10);
    
    QLabel *classTitle = new QLabel("<b>Window Class:</b>", this);
    classTitle->setStyleSheet("color: #00e5ff;");
    m_windowClassLabel = new QLabel("-", this);
    m_windowClassLabel->setWordWrap(true);
    
    QLabel *windowTitle = new QLabel("<b>Window Title:</b>", this);
    windowTitle->setStyleSheet("color: #00e5ff;");
    m_windowTitleLabel = new QLabel("-", this);
    m_windowTitleLabel->setWordWrap(true);
    
    m_starButton = new QPushButton("Star this session", this);
    m_starButton->setStyleSheet(
        "QPushButton { background-color: #111; color: #00e5ff; border: 1px solid #00e5ff; padding: 8px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #00e5ff; color: #000; }"
        "QPushButton:pressed { background-color: #00b8cc; border-color: #00b8cc; }"
    );
    
    rightLayout->addWidget(classTitle);
    rightLayout->addWidget(m_windowClassLabel);
    rightLayout->addSpacing(15);
    rightLayout->addWidget(windowTitle);
    rightLayout->addWidget(m_windowTitleLabel);
    rightLayout->addSpacing(30);
    rightLayout->addWidget(m_starButton);
    rightLayout->addStretch(1);
    
    mainLayout->addLayout(leftLayout, 4);
    mainLayout->addLayout(rightLayout, 1);
    
    connect(m_timelineSlider, &QSlider::valueChanged, this, &MainWindow::onSliderValueChanged);
    connect(m_starButton, &QPushButton::clicked, this, &MainWindow::onStarSessionClicked);
    connect(m_searchBar, &QLineEdit::returnPressed, this, &MainWindow::onSearchReturnPressed);
    
    m_networkManager = new QNetworkAccessManager(this);
}

void MainWindow::connectToDatabase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QDir::homePath() + "/.local/share/omni/recall.db";
    m_db.setDatabaseName(dbPath);
    
    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text() << "at path:" << dbPath;
    }
}

void MainWindow::loadInitialData()
{
    if (!m_db.isOpen()) return;
    
    QSqlQuery query("SELECT MIN(timestamp), MAX(timestamp) FROM recall_frames");
    if (query.next()) {
        m_minTimestamp = query.value(0).toLongLong();
        m_maxTimestamp = query.value(1).toLongLong();
        
        if (m_maxTimestamp > m_minTimestamp) {
            m_timelineSlider->setRange(0, 1000); // 1000 granular steps for the timeline
            m_timelineSlider->setValue(1000); // Start at the most recent frame
        }
    }
}

void MainWindow::onSliderValueChanged(int value)
{
    if (m_maxTimestamp <= m_minTimestamp) return;
    
    double progress = static_cast<double>(value) / 1000.0;
    qint64 targetTimestamp = m_minTimestamp + static_cast<qint64>(progress * (m_maxTimestamp - m_minTimestamp));
    
    updateFrame(targetTimestamp);
}

void MainWindow::updateFrame(qint64 timestamp)
{
    if (!m_db.isOpen()) return;
    
    QSqlQuery query;
    // Find the closest frame to the requested timestamp
    query.prepare("SELECT timestamp, image_path, window_class, window_title "
                  "FROM recall_frames "
                  "ORDER BY ABS(timestamp - ?) ASC LIMIT 1");
    query.addBindValue(timestamp);
    
    if (query.exec() && query.next()) {
        m_currentTimestamp = query.value(0).toLongLong();
        QString imagePath = query.value(1).toString();
        QString windowClass = query.value(2).toString();
        QString windowTitle = query.value(3).toString();
        
        if (imagePath.endsWith(".tar.xz")) {
            QString filenameInArchive = QString::number(m_currentTimestamp) + ".png";
            QProcess tarProcess;
            tarProcess.start("tar", QStringList() << "-xOf" << imagePath << filenameInArchive);
            if (tarProcess.waitForFinished(5000) && tarProcess.exitCode() == 0) {
                QByteArray imageData = tarProcess.readAllStandardOutput();
                m_currentPixmap.loadFromData(imageData);
            } else {
                qWarning() << "Failed to extract frame from archive:" << imagePath << filenameInArchive;
                m_currentPixmap = QPixmap();
            }
        } else {
            m_currentPixmap = QPixmap(imagePath);
        }
        
        if (m_currentPixmap.isNull()) {
            m_imageLabel->setText("Image not found:\n" + imagePath);
        } else {
            updateImageLabel();
        }
        
        m_windowClassLabel->setText(windowClass.isEmpty() ? "Unknown" : windowClass);
        m_windowTitleLabel->setText(windowTitle.isEmpty() ? "Unknown" : windowTitle);
    }
}

void MainWindow::updateImageLabel()
{
    if (!m_currentPixmap.isNull()) {
        // Keeps aspect ratio and scales smoothly without distorting the layout
        m_imageLabel->setPixmap(m_currentPixmap.scaled(m_imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateImageLabel(); // Ensure image scales accurately with window resizes
}

void MainWindow::onStarSessionClicked()
{
    if (!m_db.isOpen() || m_currentTimestamp == 0) {
        QMessageBox::warning(this, "Error", "No frame currently loaded to star.");
        return;
    }
    
    bool ok;
    QString title = QInputDialog::getText(this, "Star Session",
                                         "Enter a title for this session:",
                                         QLineEdit::Normal,
                                         "Important Session", &ok);
    if (!ok || title.trimmed().isEmpty()) return;
    
    int durationMins = QInputDialog::getInt(this, "Star Session",
                                           "Enter duration in minutes:",
                                           60, 1, 10000, 1, &ok);
    if (!ok) return;
    
    // We assume timestamps are in milliseconds (UNIX time * 1000) for standard calculations.
    // If standard unix timestamps (seconds) are used, this will represent a large span, but remains logically correct based on requested math.
    qint64 durationMs = static_cast<qint64>(durationMins) * 60 * 1000;
    qint64 startTime = m_currentTimestamp - durationMs;
    qint64 endTime = m_currentTimestamp;
    
    QSqlQuery query;
    query.prepare("INSERT INTO starred_spans (start_time, end_time, title) VALUES (?, ?, ?)");
    query.addBindValue(startTime);
    query.addBindValue(endTime);
    query.addBindValue(title);
    
    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error",
                             "Failed to save starred session:\n" + query.lastError().text());
    } else {
        QMessageBox::information(this, "Success", "Session starred successfully!");
    }
}

void MainWindow::onSearchReturnPressed()
{
    QString query = m_searchBar->text().trimmed();
    if (query.isEmpty()) return;
    
    QUrl url("http://localhost:8000/api/search");
    QUrlQuery queryParams;
    queryParams.addQueryItem("q", query);
    url.setQuery(queryParams);
    
    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onSearchReplyFinished(reply);
    });
}

void MainWindow::onSearchReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Search request failed:" << reply->errorString();
        QMessageBox::warning(this, "Search Error", "Failed to connect to search API.");
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        qWarning() << "Invalid JSON response from search API";
        return;
    }
    
    QJsonArray arr = doc.array();
    if (arr.isEmpty()) {
        QMessageBox::information(this, "Search", "No results found.");
        return;
    }
    
    qint64 topId = arr.at(0).toInteger();
    
    if (!m_db.isOpen()) return;
    
    QSqlQuery q;
    q.prepare("SELECT timestamp FROM recall_frames WHERE id = ?");
    q.addBindValue(topId);
    
    if (q.exec() && q.next()) {
        qint64 ts = q.value(0).toLongLong();
        
        if (m_maxTimestamp > m_minTimestamp) {
            double progress = static_cast<double>(ts - m_minTimestamp) / (m_maxTimestamp - m_minTimestamp);
            int sliderValue = static_cast<int>(progress * 1000);
            
            m_timelineSlider->blockSignals(true);
            m_timelineSlider->setValue(sliderValue);
            m_timelineSlider->blockSignals(false);
        }
        
        updateFrame(ts);
    } else {
        qWarning() << "Search returned frame ID" << topId << "but it was not found in the database.";
    }
}
