#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QStyle>
#include <QFileInfo>
#include <QMimeData>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , archiver(new Archiver())
{
    ui->setupUi(this);
    
    // Устанавливаем стили
    QString styleSheet = R"(
        QMainWindow {
            background-color: #f0f7ff;
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
        QLabel {
            color: #2c3e50;
        }
        QGroupBox {
            border: 2px solid #4a90e2;
            border-radius: 6px;
            margin-top: 1ex;
            font-weight: bold;
        }
        QGroupBox::title {
            color: #2c3e50;
        }
        QComboBox {
            padding: 5px;
            border: 1px solid #4a90e2;
            border-radius: 4px;
            background: white;
            color: #4a90e2;
            font-weight: bold;
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
        QComboBox QAbstractItemView::item {
            padding: 5px;
            min-height: 25px;
        }
        QComboBox QAbstractItemView::item:hover {
            background-color: #4a90e2;
            color: white;
        }
        QComboBox QAbstractItemView::item:selected {
            background-color: #357abd;
            color: white;
        }
        QSpinBox {
            padding: 5px;
            border: 1px solid #4a90e2;
            border-radius: 4px;
            background: white;
            color: #4a90e2;
        }
        QLineEdit {
            padding: 5px;
            border: 1px solid #4a90e2;
            border-radius: 4px;
            background: white;
            color: #2c3e50;
        }
        QCheckBox {
            color: #2c3e50;
        }
    )";
    setStyleSheet(styleSheet);

    // Настраиваем отступы для группировок
    ui->archiveGroup->layout()->setContentsMargins(20, 20, 20, 20);
    ui->extractGroup->layout()->setContentsMargins(20, 20, 20, 20);

    // Устанавливаем цвет для заголовка
    QPalette titlePalette = ui->titleLabel->palette();
    titlePalette.setColor(QPalette::WindowText, QColor("#2c3e50"));
    ui->titleLabel->setPalette(titlePalette);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete archiver;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QString path = mimeData->urls().first().toLocalFile();
        processDroppedPath(path);
    }
}

void MainWindow::processDroppedPath(const QString &path)
{
    QFileInfo fileInfo(path);
    if (fileInfo.isDir()) {
        createArchive(path);
    } else if (fileInfo.isFile()) {
        QString extension = fileInfo.suffix().toLower();
        if (extension == "zip" || extension == "tar" || extension == "gz" || extension == "7z" || extension == "rar") {
            extractArchive(path);
        } else {
            setStatusMessage("Неподдерживаемый формат файла", true);
        }
    }
}

QString MainWindow::getArchiveExtension() const
{
    return ".zip"; // По умолчанию ZIP
}

void MainWindow::createArchive(const QString &sourcePath)
{
    QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists()) {
        setStatusMessage("Выбранная папка не существует", true);
        return;
    }

    // Показываем диалог настроек
    SettingsDialog settingsDialog(this);
    if (settingsDialog.exec() != QDialog::Accepted) {
        return;
    }

    QString format = settingsDialog.getFormat();
    QString extension = format == "TAR.GZ" ? ".tar.gz" : "." + format.toLower();
    QString defaultName = sourceInfo.fileName() + extension;
    QString destinationPath = QFileDialog::getSaveFileName(this,
        "Сохранить архив",
        QDir::homePath() + "/" + defaultName,
        "Архивы (*" + extension + ")");

    if (destinationPath.isEmpty()) {
        return;
    }

    if (!destinationPath.endsWith(extension)) {
        destinationPath += extension;
    }

    try {
        QDir sourceDir(sourcePath);
        if (archiver->archiveFile(sourcePath, destinationPath, 
            settingsDialog.getCompressionLevel(),
            settingsDialog.isSelfExtracting())) {
            setStatusMessage("Архив успешно создан: " + destinationPath);
        } else {
            setStatusMessage("Ошибка при создании архива", true);
        }
    } catch (const std::exception &e) {
        setStatusMessage(QString("Ошибка: %1").arg(e.what()), true);
    }
}

void MainWindow::extractArchive(const QString &archivePath, const QString &destinationPath)
{
    QString destPath = destinationPath;
    if (destPath.isEmpty()) {
        destPath = QFileDialog::getExistingDirectory(this,
            "Выберите папку для распаковки",
            QDir::homePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    }

    if (destPath.isEmpty()) {
        return;
    }

    try {
        if (archiver->extractArchive(archivePath, destPath)) {
            setStatusMessage("Архив успешно распакован в: " + destPath);
        } else {
            setStatusMessage("Ошибка при распаковке архива", true);
        }
    } catch (const std::exception &e) {
        setStatusMessage(QString("Ошибка: %1").arg(e.what()), true);
    }
}

void MainWindow::on_archiveButton_clicked()
{
    QString sourcePath = QFileDialog::getExistingDirectory(this,
        "Выберите папку для архивации",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!sourcePath.isEmpty()) {
        createArchive(sourcePath);
    }
}

void MainWindow::on_extractButton_clicked()
{
    QString archivePath = QFileDialog::getOpenFileName(this,
        "Выберите архив для распаковки",
        QDir::homePath(),
        "Архивы (*.zip *.tar *.gz *.7z *.rar);;Все файлы (*)");

    if (!archivePath.isEmpty()) {
        extractArchive(archivePath);
    }
}

void MainWindow::on_settingsButton_clicked()
{
    SettingsDialog settingsDialog(this);
    settingsDialog.exec();
}

void MainWindow::setStatusMessage(const QString &message, bool isError)
{
    QString style = isError ? "color: #e74c3c;" : "color: #27ae60;";
    ui->archiveStatusLabel->setStyleSheet(style);
    ui->extractStatusLabel->setStyleSheet(style);
    
    if (message.contains("архив")) {
        ui->archiveStatusLabel->setText(message);
    } else {
        ui->extractStatusLabel->setText(message);
    }
} 