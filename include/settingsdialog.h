#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    QString getFormat() const;
    int getCompressionLevel() const;
    bool isSelfExtracting() const;

private:
    QComboBox *formatComboBox;
    QSpinBox *compressionSpinBox;
    QCheckBox *selfExtractingCheckBox;
};

#endif // SETTINGSDIALOG_H 