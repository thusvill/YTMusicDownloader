#include <QApplication>
#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProcess>
#include <QLineEdit>
#include <QScrollBar>
#include <QLabel>
#include <QDir>

class LogWindow : public QWidget {
    Q_OBJECT

public:
    LogWindow(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);

        
        QLabel *urlLabel = new QLabel("Enter YouTube URL:", this);
        layout->addWidget(urlLabel);

        
        urlInput = new QLineEdit(this);
        layout->addWidget(urlInput);

        
        logTextEdit = new QTextEdit(this);
        logTextEdit->setReadOnly(true);  
        layout->addWidget(logTextEdit);

        
        startButton = new QPushButton("Start Download", this);
        layout->addWidget(startButton);

        connect(startButton, &QPushButton::clicked, this, &LogWindow::startDownload);

        setLayout(layout);
        setWindowTitle("Download Log");
        resize(600, 400);
    }

private slots:
    void startDownload() {
        QString videoURL = urlInput->text();
        if (videoURL.isEmpty()) {
            logTextEdit->append("Error: You must enter a valid YouTube URL.");
            return;
        }

        QString ytDlpCommand = "/opt/homebrew/bin/yt-dlp"; 
        QStringList arguments;
        
        
        QString savePath = QDir::homePath() + "/Downloads/Music";
        
        arguments << "-f" << "bestaudio"
                  << "--ffmpeg-location" << "/opt/homebrew/bin/ffmpeg"
                  << "--extract-audio"
                  << "--audio-format" << "mp3"
                  << "--embed-metadata"
                  << "--embed-thumbnail"
                  << "--add-metadata"
                  << "--output" << savePath + "/%(title)s.%(ext)s"
                  << "--verbose"
                  << videoURL;

        process = new QProcess(this);
        process->setProgram(ytDlpCommand);
        process->setArguments(arguments);

        connect(process, &QProcess::readyReadStandardOutput, this, &LogWindow::readOutput);
        connect(process, &QProcess::readyReadStandardError, this, &LogWindow::readError);
        connect(process, &QProcess::finished, this, &LogWindow::downloadFinished);
        
        process->start();
    }

    void readOutput() {
        QByteArray output = process->readAllStandardOutput();
        logTextEdit->append(QString(output));
        logTextEdit->verticalScrollBar()->setValue(logTextEdit->verticalScrollBar()->maximum());
    }

    void readError() {
        QByteArray error = process->readAllStandardError();
        logTextEdit->append(QString(error));
        logTextEdit->verticalScrollBar()->setValue(logTextEdit->verticalScrollBar()->maximum());
    }

    void downloadFinished(int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit) {
            logTextEdit->append("\nDownload complete!");
        } else {
            logTextEdit->append("\nDownload failed with exit code: " + QString::number(exitCode));
        }
    }

private:
    QLineEdit *urlInput;
    QTextEdit *logTextEdit;
    QPushButton *startButton; 
    QProcess *process;        
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    LogWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
