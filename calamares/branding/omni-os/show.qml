import QtQuick 2.15
import QtQuick.Controls 2.15
import calamares.slideshow 1.0

Presentation {
    id: presentation
    width: 800
    height: 520
    
    Timer {
        interval: 8000
        running: true
        repeat: true
        onTriggered: presentation.goToNextSlide()
    }

    Rectangle {
        anchors.fill: parent
        color: "#1a1b26"
    }

    Slide {
        Rectangle {
            anchors.fill: parent
            color: "transparent"
            Text {
                anchors.centerIn: parent
                text: "<b>Welcome to Omni-OS</b><br/><br/>The sovereign, AI-powered operating system."
                color: "#c0caf5"
                font.pixelSize: 24
                font.family: "JetBrainsMono Nerd Font, monospace"
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    Slide {
        Rectangle {
            anchors.fill: parent
            color: "transparent"
            Text {
                anchors.centerIn: parent
                text: "<b>Your AI Assistant</b><br/><br/>Local Ollama integration, zero cloud dependency."
                color: "#7dcfff"
                font.pixelSize: 24
                font.family: "JetBrainsMono Nerd Font, monospace"
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    Slide {
        Rectangle {
            anchors.fill: parent
            color: "transparent"
            Text {
                anchors.centerIn: parent
                text: "<b>Unbreakable by Design</b><br/><br/>Tri-Kernel + Btrfs snapshots for maximum stability."
                color: "#bb9af7"
                font.pixelSize: 24
                font.family: "JetBrainsMono Nerd Font, monospace"
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    Slide {
        Rectangle {
            anchors.fill: parent
            color: "transparent"
            Text {
                anchors.centerIn: parent
                text: "<b>Deep Work Mode</b><br/><br/>Distraction blocking & Observer telemetry to keep you focused."
                color: "#e0af68"
                font.pixelSize: 24
                font.family: "JetBrainsMono Nerd Font, monospace"
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    Slide {
        Rectangle {
            anchors.fill: parent
            color: "transparent"
            Text {
                anchors.centerIn: parent
                text: "<b>Omni-Recall</b><br/><br/>Search your past with local OCR timeline capabilities."
                color: "#7dcfff"
                font.pixelSize: 24
                font.family: "JetBrainsMono Nerd Font, monospace"
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    Slide {
        Rectangle {
            anchors.fill: parent
            color: "transparent"
            Text {
                anchors.centerIn: parent
                text: "<b>Personalized for You</b><br/><br/>AI setup wizard adapts to your workflow and major."
                color: "#c0caf5"
                font.pixelSize: 24
                font.family: "JetBrainsMono Nerd Font, monospace"
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
