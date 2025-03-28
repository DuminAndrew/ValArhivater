#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Настройки архивации");
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Формат архива
    QHBoxLayout *formatLayout = new QHBoxLayout;
    QLabel *formatLabel = new QLabel("Формат архива:", this);
    formatComboBox = new QComboBox(this);
    formatComboBox->addItems({"ZIP", "TAR", "TAR.GZ", "7Z"});
    formatLayout->addWidget(formatLabel);
    formatLayout->addWidget(formatComboBox);
    mainLayout->addLayout(formatLayout);

    // Уровень сжатия
    QHBoxLayout *compressionLayout = new QHBoxLayout;
    QLabel *compressionLabel = new QLabel("Уровень сжатия:", this);
    compressionSpinBox = new QSpinBox(this);
    compressionSpinBox->setRange(0, 9);
    compressionSpinBox->setValue(6);
    compressionSpinBox->setToolTip("0 - без сжатия, 9 - максимальное сжатие");
    compressionLayout->addWidget(compressionLabel);
    compressionLayout->addWidget(compressionSpinBox);
    mainLayout->addLayout(compressionLayout);

    // Самораспаковывающийся архив
    QHBoxLayout *selfExtractingLayout = new QHBoxLayout;
    selfExtractingCheckBox = new QCheckBox("Создать самораспаковывающийся архив", this);
    selfExtractingCheckBox->setToolTip("Архив будет содержать программу для распаковки");
    selfExtractingLayout->addWidget(selfExtractingCheckBox);
    mainLayout->addLayout(selfExtractingLayout);

    // Кнопки
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    // Соединения
    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    // Стили
    setStyleSheet(R"(
        QDialog {
            background-color: #f0f7ff;
        }
        QLabel {
            color: #2c3e50;
            font-weight: bold;
        }
        QComboBox, QSpinBox {
            padding: 5px;
            border: 1px solid #4a90e2;
            border-radius: 4px;
            background: white;
            color: #2c3e50;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            image: url(:/icons/arrow.png);
            width: 12px;
            height: 12px;
        }
        QComboBox QAbstractItemView {
            background-color: white;
            selection-background-color: #4a90e2;
            selection-color: white;
            border: 1px solid #4a90e2;
            border-radius: 4px;
        }
        QPushButton {
            background-color: #4a90e2;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #357abd;
        }
        QPushButton:pressed {
            background-color: #2d6da3;
        }
        QCheckBox {
            color: #2c3e50;
        }
    )");
}

QString SettingsDialog::getFormat() const
{
    return formatComboBox->currentText();
}

int SettingsDialog::getCompressionLevel() const
{
    return compressionSpinBox->value();
}

bool SettingsDialog::isSelfExtracting() const
{
    return selfExtractingCheckBox->isChecked();
} 