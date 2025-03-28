#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include "archiver.h"
#include "settingsdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void extractArchive(const QString &archivePath, const QString &destinationPath = QString());

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void on_archiveButton_clicked();
    void on_extractButton_clicked();
    void on_settingsButton_clicked();
    void processDroppedPath(const QString &path);

private:
    Ui::MainWindow *ui;
    Archiver *archiver;
    QString getArchiveExtension() const;
    void createArchive(const QString &sourcePath);
    void setStatusMessage(const QString &message, bool isError = false);
};

#endif // MAINWINDOW_H 