#include "settings_window.h"
#include <QPushButton>
#include <QFileDialog>
#include "Utils.hpp"
SettingsWindow::SettingsWindow(QWidget *parent) : QDialog(parent)
{
    // Set window flags to make it a regular window
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint); // Remove context help button
    setWindowTitle("Settings");

    // Set up layout and widgets
    QVBoxLayout *settingsLayout = new QVBoxLayout(this);

    QSettings settings("DarkMission", "YouTubeDownloader");

    {
        HorizontalLayout savePath(this);
        auto save_p_text = new QLabel("Save Path: ", this);
        save_p_input = new QLineEdit(this);
        save_p_input->setPlaceholderText("Musics");
        save_p_input->setText(settings.value("save_path").toString());

        QPushButton *selectFolderButton = new QPushButton("📂", this);
        connect(selectFolderButton, &QPushButton::clicked, this, &SettingsWindow::selectFolder);

        savePath.Get()->addWidget(save_p_text);
        savePath.Get()->addWidget(save_p_input);
        savePath.Get()->addWidget(selectFolderButton);
        settingsLayout->addLayout(savePath.Get());
    }
    {
        HorizontalLayout saveFormat(this);
        QLabel *audioFormatLabel = new QLabel("Select Audio Format:", this);
        saveFormat.Get()->addWidget(audioFormatLabel);

        audioFormatComboBox = new QComboBox(this);
        audioFormatComboBox->addItem("MP3", "mp3");
        audioFormatComboBox->addItem("AAC", "aac");
        audioFormatComboBox->addItem("FLAC", "flac");
        audioFormatComboBox->addItem("WAV", "wav");
        audioFormatComboBox->addItem("OGG", "ogg");
        audioFormatComboBox->addItem("ALAC", "alac");
        audioFormatComboBox->addItem("Opus", "opus");
        saveFormat.Get()->addWidget(audioFormatComboBox);

        // Set current format from settings
        QString savedFormat = settings.value("audio_format", "mp3").toString();
        int index = audioFormatComboBox->findData(savedFormat);
        if (index != -1)
        {
            audioFormatComboBox->setCurrentIndex(index);
        }

        settingsLayout->addLayout(saveFormat.Get());
    }


    QPushButton *saveButton = new QPushButton("Save", this);
    settingsLayout->addWidget(saveButton);

    connect(saveButton, &QPushButton::clicked, this, &SettingsWindow::saveSettings);

    setLayout(settingsLayout);
    resize(600, 400); // Resize window to desired size
}
void SettingsWindow::selectFolder()
{
    // Open folder selection dialog
    QString folder = QFileDialog::getExistingDirectory(this, "Select Save Folder", QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!folder.isEmpty())
    {
        save_p_input->setText(folder); // Set the selected folder to the input field
    }
}
void SettingsWindow::saveSettings()
{
    QSettings settings("DarkMission", "YouTubeDownloader");
    settings.setValue("save_path", save_p_input->text());

    QString selectedAudioFormat = audioFormatComboBox->currentData().toString();
    settings.setValue("audio_format", selectedAudioFormat);

    close();
}