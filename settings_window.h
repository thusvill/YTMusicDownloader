#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSettings>
#include <QDir>

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    SettingsWindow(QWidget *parent = nullptr);

private slots:
    void saveSettings();
    void selectFolder();

private:
    QLineEdit *save_p_input;
    QComboBox *audioFormatComboBox;
};

#endif // SETTINGSWINDOW_H