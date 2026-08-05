#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QSqlDatabase>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onSliderValueChanged(int value);
    void onStarSessionClicked();
    void onSearchReturnPressed();
    void onSearchReplyFinished(QNetworkReply *reply);

private:
    void setupUi();
    void connectToDatabase();
    void loadInitialData();
    void updateFrame(qint64 timestamp);
    void updateImageLabel();
    
    QLabel *m_imageLabel;
    QSlider *m_timelineSlider;
    QLabel *m_windowClassLabel;
    QLabel *m_windowTitleLabel;
    QPushButton *m_starButton;
    QLineEdit *m_searchBar;
    QNetworkAccessManager *m_networkManager;
    
    QSqlDatabase m_db;
    qint64 m_minTimestamp;
    qint64 m_maxTimestamp;
    qint64 m_currentTimestamp;
    QPixmap m_currentPixmap;
};

#endif // MAINWINDOW_H
