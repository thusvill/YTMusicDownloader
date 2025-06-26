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
            &LogWindow::postDownload);

    process->start();
  }
  void postDownload() {
    QString savePath = settings->value("save_path").toString();
    QString audioFormat = settings->value("audio_format").toString();
    QDir dir(savePath);
    QString ffmpegPath = RunSystemCommand::output("which ffmpeg").trimmed();

    // List all audio and thumbnail files (newest first)
    QStringList audioFiles = dir.entryList(QStringList() << "*." + audioFormat,
                                           QDir::Files, QDir::Time);
    QStringList thumbs = dir.entryList(QStringList() << "*.jpg" << "*.webp",
                                       QDir::Files, QDir::Time);

    int processed = 0;

    for (const QString &audioFile : audioFiles) {
      QString audioPath = dir.absoluteFilePath(audioFile);

      // Match thumbnail by filename base
      QString baseName = QFileInfo(audioFile).completeBaseName();
      QString matchingThumb;
      for (const QString &thumb : thumbs) {
        if (QFileInfo(thumb).completeBaseName().startsWith(baseName)) {
          matchingThumb = dir.absoluteFilePath(thumb);
          break;
        }
      }

      if (matchingThumb.isEmpty() || !QFile::exists(matchingThumb)) {
        logTextEdit->append("⚠️ Skipping: No thumbnail found for " + audioFile);
        continue;
      }
      if (QFileInfo(matchingThumb).suffix() == "webp") {
        QString jpgPath = matchingThumb;
        jpgPath.chop(5); // remove ".webp"
        jpgPath += ".jpg";

        QString convertCmd = QString("%1 -y -i \"%2\" \"%3\"")
                                 .arg(ffmpegPath, matchingThumb, jpgPath);
        RunSystemCommand::output(convertCmd);

        QFile::remove(matchingThumb);
        matchingThumb = jpgPath;
      }

      // Crop thumbnail
      QString croppedThumb = dir.absoluteFilePath(
          "cropped_" + QString::number(processed) + ".jpg");
      QString cropCmd =
          QString("%1 -y -i \"%2\" -vf "
                  "\"crop='min(in_w,in_h)':'min(in_w,in_h)'\" \"%3\"")
              .arg(ffmpegPath, matchingThumb, croppedThumb);
      RunSystemCommand::output(cropCmd);

      // Embed cropped thumbnail
      QString finalPath = dir.absoluteFilePath("final_" + audioFile);
      QString embedCmd =
          QString("%1 -y -i \"%2\" -i \"%3\" -map 0 -map 1 -c copy "
                  "-id3v2_version 3 "
                  "-metadata:s:v title=\"Album cover\" -metadata:s:v "
                  "comment=\"Cover (front)\" \"%4\"")
              .arg(ffmpegPath, audioPath, croppedThumb, finalPath);
      RunSystemCommand::output(embedCmd);

      QFile::remove(audioPath);
      QFile::rename(finalPath, audioPath);
      QFile::remove(matchingThumb);
      QFile::remove(croppedThumb);

      logTextEdit->append("✅ Embedded art into: " +
                          QFileInfo(audioPath).fileName());
      processed++;
    }

    if (processed == 0)
      logTextEdit->append(
          "⚠️ No files processed. Check thumbnails or format settings.");
    else
      logTextEdit->append("🎉 Done: " + QString::number(processed) +
                          " tracks processed.");
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
