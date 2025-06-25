#include <QHBoxLayout>
#include <QLayout>
#include <QProcess>
#include <QWidget>

class HorizontalLayout {
public:
  HorizontalLayout(QWidget *parent) { layout = new QHBoxLayout(parent); }
  QHBoxLayout *Get() { return layout; }

private:
  QHBoxLayout *layout;
};

class RunSystemCommand {
public:
  static QString output(const QString &command) {
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    QString path = env.value("PATH");
    path += ":/opt/homebrew/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin";
    env.insert("PATH", path);
    process.setProcessEnvironment(env);

#ifdef Q_OS_WIN
    QString shell = "cmd.exe";
    QStringList args = {"/c", command};
#else
    QString shell = "/bin/bash";
    QStringList args = {"-c", command};
#endif

    process.start(shell, args);

    if (!process.waitForStarted()) {
      return "Error: Failed to start process: " + shell;
    }

    if (!process.waitForFinished()) {
      return "Error: Process failed to finish.";
    }

    int exitCode = process.exitCode();
    QString result = process.readAllStandardOutput().trimmed();
    if (exitCode != 0) {
      result.prepend("Warning: Non-zero exit code.\n");
    }

    return result;
  }
};