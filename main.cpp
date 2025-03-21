#include <QApplication>
#include <QHBoxLayout>
#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProcess>
#include <QLineEdit>
#include <QScrollBar>
#include <QLabel>
#include <QDir>
#include <QSettings>

#include "settings_window.h"

class LogWindow : public QWidget
{
    Q_OBJECT

public:
    LogWindow(QWidget *parent = nullptr) : QWidget(parent)
    {
        layout = new QVBoxLayout(this);
        settings = new QSettings("DarkMission", "YouTubeDownloader");
        if (settings->value("save_path").isNull())
        {
            settings->setValue("save_path", "~/Music");
        }

        // Label for the input field
        QLabel *urlLabel = new QLabel("Enter YouTube URL:", this);
        layout->addWidget(urlLabel);

        QHBoxLayout *urlLayout = new QHBoxLayout();

        // Create the input field
        urlInput = new QLineEdit(this);
        urlInput->setPlaceholderText("Enter YouTube URL...");
        urlLayout->addWidget(urlInput);

        // Create the settings button
        settingsButton = new QPushButton(this);
        settingsButton->setFixedSize(urlInput->geometry().height(), urlInput->geometry().height());
        settingsButton->setText("⚙️");
        auto font = settingsButton->font();
        //font.setPointSize(15);
        //settingsButton->setFont(font);
        urlLayout->addWidget(settingsButton);

        layout->addLayout(urlLayout); // Add the horizontal layout to the main layout

        connect(settingsButton, &QPushButton::clicked, this, &LogWindow::OpenSettigs);

        // TextEdit for displaying logs
        logTextEdit = new QTextEdit(this);
        logTextEdit->setReadOnly(true);
        layout->addWidget(logTextEdit);

        // Button to start download
        startButton = new QPushButton("Start Download", this);
        layout->addWidget(startButton);

        exitButton = new QPushButton("Done", this);
        layout->addWidget(exitButton);
        exitButton->hide();

        connect(startButton, &QPushButton::clicked, this, &LogWindow::startDownload);

        setLayout(layout);
        setWindowTitle("YT Music Downloader");
        resize(600, 500);
    }

private slots:
    void OpenSettigs()
    {
        SettingsWindow *settingsWindow = new SettingsWindow(this);
        settingsWindow->raise();
        settingsWindow->show();
    }
    void startDownload()
    {
        QString videoURL = urlInput->text();
        if (videoURL.isEmpty())
        {
            logTextEdit->append("Error: You must enter a valid YouTube URL.");
            return;
        }

        QString ytDlpCommand = "/opt/homebrew/bin/yt-dlp";
        QStringList arguments;

        // Set the fixed save path to ~/Downloads/Music
        QString savePath = settings->value("save_path").toString();

        arguments << "-f" << "bestaudio" // Download best audio quality
                  << "--audio-quality" << "0"
                  << "--ffmpeg-location" << "/opt/homebrew/bin/ffmpeg"
                  << "--extract-audio"                             // Extract audio only (no video)
                  << "--audio-format" << settings->value("audio_format").toString()
                  << "--embed-metadata"                            // Embed metadata
                  << "--embed-thumbnail"                           // Embed thumbnail
                  << "--add-metadata"                              // Add metadata
                  << "--output" << savePath + "/%(title)s.%(ext)s" // Set output path and filename
                  << "--verbose"                                   // Show verbose output
                  << videoURL;                                     // YouTube URL

        process = new QProcess(this);
        process->setProgram(ytDlpCommand);
        process->setArguments(arguments);

        connect(process, &QProcess::readyReadStandardOutput, this, &LogWindow::readOutput);
        connect(process, &QProcess::readyReadStandardError, this, &LogWindow::readError);
        connect(process, &QProcess::finished, this, &LogWindow::downloadFinished);

        process->start();
    }

    void readOutput()
    {
        QByteArray output = process->readAllStandardOutput();
        logTextEdit->append(QString(output));
        logTextEdit->verticalScrollBar()->setValue(logTextEdit->verticalScrollBar()->maximum());
    }

    void readError()
    {
        QByteArray error = process->readAllStandardError();
        logTextEdit->append(QString(error));
        logTextEdit->verticalScrollBar()->setValue(logTextEdit->verticalScrollBar()->maximum());
    }

    void downloadFinished(int exitCode, QProcess::ExitStatus exitStatus)
    {
        if (exitStatus == QProcess::NormalExit)
        {
            logTextEdit->append("\nDownload complete!");
            layout->removeWidget(startButton);
            startButton->deleteLater();
            startButton = new QPushButton("New Download", this);
            layout->addWidget(startButton);

            connect(startButton, &QPushButton::clicked, this, &LogWindow::startDownload);

            layout->removeWidget(exitButton);
            exitButton->deleteLater();
            exitButton = new QPushButton("Done", this);
            layout->addWidget(exitButton);
            connect(exitButton, &QPushButton::clicked, qApp, &QApplication::quit);
        }
        else
        {
            logTextEdit->append("\nDownload failed with exit code: " + QString::number(exitCode));
        }
    }

private:
    QLineEdit *urlInput;
    QTextEdit *logTextEdit;
    QPushButton *startButton;
    QPushButton *exitButton;
    QPushButton *settingsButton;
    QProcess *process;
    QVBoxLayout *layout;
    QSettings *settings;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LogWindow window;
    window.show();

    return app.exec();
}
#include "main.moc"
