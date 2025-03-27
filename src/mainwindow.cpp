#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    archiver = new Archiver();

    // Устанавливаем стили
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f0f7ff;
        }
        QPushButton {
            background-color: #2196F3;
            color: white;
            border: none;
            padding: 12px 24px;
            border-radius: 6px;
            font-size: 14px;
            font-weight: bold;
            min-width: 150px;
        }
        QPushButton:hover {
            background-color: #1976D2;
        }
        QPushButton:pressed {
            background-color: #0D47A1;
        }
        QLabel {
            color: #333;
            font-size: 14px;
        }
        QGroupBox {
            border: 2px solid #2196F3;
            border-radius: 8px;
            margin-top: 16px;
            padding: 16px;
            background-color: white;
            font-weight: bold;
            color: #1976D2;
            font-size: 16px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
        QStatusBar {
            background-color: #e3f2fd;
            color: #1976D2;
            font-weight: bold;
        }
    )");

    // Устанавливаем отступы и интервалы
    ui->verticalLayout->setContentsMargins(20, 20, 20, 20);
    ui->verticalLayout->setSpacing(20);
    ui->verticalLayout_2->setSpacing(16);
    ui->verticalLayout_3->setSpacing(16);

    // Устанавливаем цвет заголовка
    ui->titleLabel->setStyleSheet("color: #1976D2;");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete archiver;
}

void MainWindow::on_archiveButton_clicked()
{
    QString sourcePath = QFileDialog::getExistingDirectory(this, "Выберите папку для архивации", QDir::homePath());
    if (sourcePath.isEmpty()) {
        return;
    }

    QString archivePath = QFileDialog::getSaveFileName(this, "Сохранить архив", 
        QDir::homePath() + "/" + QFileInfo(sourcePath).fileName() + ".zip", 
        "ZIP архив (*.zip)");
    
    if (archivePath.isEmpty()) {
        return;
    }

    if (!archivePath.endsWith(".zip")) {
        archivePath += ".zip";
    }

    if (archiver->archiveFile(sourcePath, archivePath)) {
        ui->archiveStatusLabel->setText("Папка успешно заархивирована: " + archivePath);
        ui->archiveStatusLabel->setStyleSheet("color: #4CAF50;");
        QMessageBox::information(this, "Успех", "Папка успешно заархивирована");
    } else {
        ui->archiveStatusLabel->setText("Ошибка при архивации папки");
        ui->archiveStatusLabel->setStyleSheet("color: #F44336;");
        QMessageBox::critical(this, "Ошибка", "Не удалось создать архив");
    }
}

void MainWindow::on_extractButton_clicked()
{
    QString archivePath = QFileDialog::getOpenFileName(this, "Выберите архив для распаковки", 
        QDir::homePath(), "Архивы (*.zip *.tar *.gz)");
    
    if (archivePath.isEmpty()) {
        return;
    }

    QString extractPath = QFileDialog::getExistingDirectory(this, "Выберите папку для распаковки", 
        QDir::homePath());
    
    if (extractPath.isEmpty()) {
        return;
    }

    if (archiver->extractArchive(archivePath, extractPath)) {
        ui->extractStatusLabel->setText("Архив успешно распакован в: " + extractPath);
        ui->extractStatusLabel->setStyleSheet("color: #4CAF50;");
        QMessageBox::information(this, "Успех", "Архив успешно распакован");
    } else {
        ui->extractStatusLabel->setText("Ошибка при распаковке архива");
        ui->extractStatusLabel->setStyleSheet("color: #F44336;");
        QMessageBox::critical(this, "Ошибка", "Не удалось распаковать архив");
    }
} 