#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocalSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QGraphicsDropShadowEffect>
#include <QPixmap>
#include <QScreen>
#include <QDebug>
#include <cstdlib>

class IpcClient : public QObject {
    Q_OBJECT
public:
    IpcClient() : sock(new QLocalSocket(this)) {
        QString path = qEnvironmentVariable("GREETD_SOCK");
        if (path.isEmpty()) {
            qWarning() << "GREETD_SOCK is missing. Are we running under greetd?";
            std::exit(1);
        }
        sock->connectToServer(path);
        sock->waitForConnected();
    }

    void send(const QJsonObject& obj) {
        auto data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
        uint32_t len = data.size();
        sock->write((const char*)&len, 4);
        sock->write(data);
        sock->waitForBytesWritten();
    }

    QJsonObject recv() {
        sock->waitForReadyRead();
        QByteArray header = sock->read(4);
        if (header.size() < 4) return {};
        
        uint32_t len = *reinterpret_cast<const uint32_t*>(header.constData());
        QByteArray data;
        while (data.size() < len) {
            sock->waitForReadyRead();
            data.append(sock->read(len - data.size()));
        }
        return QJsonDocument::fromJson(data).object();
    }

private:
    QLocalSocket* sock;
};

class GreeterUI : public QWidget {
    Q_OBJECT
public:
    GreeterUI(QWidget* parent = nullptr) : QWidget(parent) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        
        setGeometry(QGuiApplication::primaryScreen()->geometry());
        setStyleSheet("background-color: #0d0f18;");

        auto* layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignCenter);

        auto* logo = new QLabel(this);
        QPixmap pm("/usr/share/pixmaps/omni-logo.png");
        if (!pm.isNull()) {
            logo->setPixmap(pm.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            
            auto* glow = new QGraphicsDropShadowEffect(this);
            glow->setBlurRadius(50);
            glow->setColor(QColor(0, 255, 255, 150));
            glow->setOffset(0, 0);
            logo->setGraphicsEffect(glow);
        }
        logo->setAlignment(Qt::AlignCenter);
        layout->addWidget(logo);
        layout->addSpacing(40);

        auto* title = new QLabel("OMNI-OS", this);
        title->setStyleSheet("color: #00ffff; font-size: 36px; font-weight: bold; letter-spacing: 10px;");
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);
        layout->addSpacing(20);

        passInput = new QLineEdit(this);
        passInput->setEchoMode(QLineEdit::Password);
        passInput->setPlaceholderText("ENTER AUTHORIZATION CODE");
        passInput->setFixedSize(400, 60);
        passInput->setStyleSheet(
            "QLineEdit { background-color: rgba(255, 255, 255, 0.05); color: #ffffff; border: 2px solid #00ffff; border-radius: 10px; padding: 10px; font-size: 18px; }"
            "QLineEdit:focus { background-color: rgba(0, 255, 255, 0.1); border: 2px solid #ff00ff; }"
        );
        connect(passInput, &QLineEdit::returnPressed, this, &GreeterUI::login);
        layout->addWidget(passInput, 0, Qt::AlignCenter);
        layout->addSpacing(20);

        statusMsg = new QLabel("", this);
        statusMsg->setStyleSheet("color: #ff0055; font-size: 14px;");
        statusMsg->setAlignment(Qt::AlignCenter);
        layout->addWidget(statusMsg);
    }

private slots:
    void login() {
        QString pwd = passInput->text();
        if (pwd.isEmpty()) return;

        try {
            QJsonObject req;
            req["type"] = "create_session";
            req["username"] = "omni";
            ipc.send(req);
            
            auto res = ipc.recv();
            if (res["type"].toString() == "auth_message") {
                QJsonObject auth;
                auth["type"] = "post_auth_message_response";
                auth["response"] = pwd;
                ipc.send(auth);
                res = ipc.recv();
            }

            if (res["type"].toString() == "success") {
                QJsonObject start;
                start["type"] = "start_session";
                start["cmd"] = QJsonArray{"Hyprland"};
                ipc.send(start);
                
                if (ipc.recv()["type"].toString() == "success") {
                    QApplication::quit();
                } else {
                    statusMsg->setText("Failed to start Hyprland");
                }
            } else {
                statusMsg->setText("Access Denied");
                passInput->clear();
            }
        } catch (...) {
            statusMsg->setText("Greetd IPC crashed");
        }
    }

private:
    IpcClient ipc;
    QLineEdit* passInput;
    QLabel* statusMsg;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    GreeterUI win;
    win.show();
    return app.exec();
}

#include "main.moc"
