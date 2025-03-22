#include <QWidget>
#include <QLayout>
#include <QHBoxLayout>
#include <QProcess>

class HorizontalLayout
{
public:
    HorizontalLayout(QWidget *parent)
    {
        layout = new QHBoxLayout(parent);
    }
    QHBoxLayout *Get() { return layout; }

private:
    QHBoxLayout *layout;
};

class RunSystemCommand {
public:
    static QString output(const QString& command) {
        QProcess process;
        process.setProcessChannelMode(QProcess::MergedChannels); // Capture both stdout & stderr
#ifdef Q_OS_WIN
        process.start("cmd.exe", {"/c", command}); // Use Windows cmd
#else
        process.start("/bin/sh", {"-c", command}); // Use Unix shell
#endif
        if (!process.waitForFinished()) {
            return "Error: Process failed to finish.";
        }

        return process.readAllStandardOutput().trimmed(); // Remove extra newlines
    }
};;