#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include "Utils.hpp"
#include "settings_window.h"

class LogWindow : public QWidget {
  Q_OBJECT

public:
  LogWindow(QWidget *parent = nullptr) : QWidget(parent) {

    layout = new QVBoxLayout(this);
    settings = new QSettings("DarkMission", "YouTubeDownloader");
    if (settings->value("save_path").isNull()) {
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
    settingsButton->setFixedSize(urlInput->geometry().height(),
                                 urlInput->geometry().height());
    settingsButton->setText("⚙️");
    auto font = settingsButton->font();
    // font.setPointSize(15);
    // settingsButton->setFont(font);
    urlLayout->addWidget(settingsButton);

    layout->addLayout(
        urlLayout); // Add the horizontal layout to the main layout

    connect(settingsButton, &QPushButton::clicked, this,
            &LogWindow::OpenSettigs);

    // TextEdit for displaying logs
    logTextEdit = new QTextEdit(this);
    logTextEdit->setReadOnly(true);
    layout->addWidget(logTextEdit);
    if (RunSystemCommand::output("which yt-dlp").isNull()) {
      logTextEdit->append("[Error]yt-dlp not installed!");
    }
    if (RunSystemCommand::output("which ffmpeg").isNull()) {
      logTextEdit->append("[Error]ffmpeg not installed!");
    }

    // Button to start download
    startButton = new QPushButton("Start Download", this);
    layout->addWidget(startButton);

    exitButton = new QPushButton("Done", this);
    layout->addWidget(exitButton);
    exitButton->hide();

    connect(startButton, &QPushButton::clicked, this,
            &LogWindow::startDownload);

    setLayout(layout);
    setWindowTitle("YT Music Downloader");
    resize(600, 500);
  }

private slots:
  void OpenSettigs() {
    SettingsWindow *settingsWindow = new SettingsWindow(this);
    settingsWindow->raise();
    settingsWindow->show();
  }
  void startDownload() {
    QString videoURL = urlInput->text();
    if (videoURL.isEmpty()) {
      logTextEdit->append("Error: You must enter a valid YouTube URL.");
      return;
    }

    QString ytDlpCommand = RunSystemCommand::output("which yt-dlp");
    logTextEdit->append("yt-dlp found on: " + ytDlpCommand);
    logTextEdit->append("ffmpeg found on: " +
                        RunSystemCommand::output("which ffmpeg"));

    QStringList arguments;
    QString savePath = settings->value("save_path").toString();
    QString outputPattern = savePath + "/%(title)s.%(ext)s";

    arguments << "-f" << "bestaudio"
              << "--audio-quality" << "0"
              << "--ffmpeg-location" << RunSystemCommand::output("which ffmpeg")
              << "--extract-audio"
              << "--audio-format" << settings->value("audio_format").toString()
              << "--embed-metadata"
              << "--write-thumbnail"
              << "--no-embed-thumbnail"
              << "--no-mtime"
              << "--add-metadata"
              << "--output" << outputPattern << "--verbose" << videoURL;

    process = new QProcess(this);
    process->setProgram(ytDlpCommand);
    process->setArguments(arguments);

    connect(process, &QProcess::readyReadStandardOutput, this,
            &LogWindow::readOutput);
    connect(process, &QProcess::readyReadStandardError, this,
            &LogWindow::readError);
    connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            &LogWindow::postDownloadProcess);

    process->start();
  }
  void postDownloadProcess(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus != QProcess::NormalExit) {
      logTextEdit->append("\nDownload failed.");
      return;
    }

    logTextEdit->append("\nDownload complete! Cropping cover and embedding...");

    QString savePath = settings->value("save_path").toString();
    QDir dir(savePath);
    QStringList imageFiles =
        dir.entryList(QStringList() << "*.webp" << "*.jpg" << "*.jpeg",
                      QDir::Files, QDir::Time);
    QStringList audioFiles =
        dir.entryList(QStringList() << "*.mp3", QDir::Files, QDir::Time);

    if (imageFiles.isEmpty() || audioFiles.isEmpty()) {
      logTextEdit->append("Couldn't find downloaded image or audio file.");
      return;
    }

    QString imagePath = dir.filePath(imageFiles.first());
    QString audioPath = dir.filePath(audioFiles.first());
    QString croppedImagePath = dir.filePath("cropped.webp");

    // 1. Crop the image
    QProcess *cropProc = new QProcess(this);
    QStringList cropArgs = {"-i", imagePath,       "-vf", "crop=in_h:in_h",
                            "-y", croppedImagePath};
    cropProc->start("ffmpeg", cropArgs);
    cropProc->waitForFinished();

    // 2. Embed the cropped image using eyeD3
    QProcess *embedProc = new QProcess(this);
    QStringList embedArgs = {"--add-image", croppedImagePath + ":FRONT_COVER",
                             audioPath};
    embedProc->start("eyeD3", embedArgs);
    embedProc->waitForFinished();

    // 3. Clean up both the original and cropped image
    QFile::remove(imagePath);
    QFile::remove(croppedImagePath);

    logTextEdit->append("✅ Cropped, embedded, and cleaned up image files.");
  }

  void readOutput() {
    QByteArray output = process->readAllStandardOutput();
    logTextEdit->append(QString(output));
    logTextEdit->verticalScrollBar()->setValue(
        logTextEdit->verticalScrollBar()->maximum());
  }

  void readError() {
    QByteArray error = process->readAllStandardError();
    logTextEdit->append(QString(error));
    logTextEdit->verticalScrollBar()->setValue(
        logTextEdit->verticalScrollBar()->maximum());
  }

  void downloadFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::NormalExit) {
      logTextEdit->append("\nDownload complete!");
      layout->removeWidget(startButton);
      startButton->deleteLater();
      startButton = new QPushButton("New Download", this);
      layout->addWidget(startButton);

      connect(startButton, &QPushButton::clicked, this,
              &LogWindow::startDownload);

      layout->removeWidget(exitButton);
      exitButton->deleteLater();
      exitButton = new QPushButton("Done", this);
      layout->addWidget(exitButton);
      connect(exitButton, &QPushButton::clicked, qApp, &QApplication::quit);
    } else {
      logTextEdit->append("\nDownload failed with exit code: " +
                          QString::number(exitCode));
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

int main(int argc, char *argv[]) {


  QApplication app(argc, argv);

  LogWindow window;
  window.show();

  return app.exec();
}
#include "main.moc"
