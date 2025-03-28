#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    QCommandLineParser parser;
    parser.setApplicationDescription("ValArhivater - архиватор папок");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption extractHereOption(QStringList() << "e" << "extract-here",
        "Распаковать архив в текущую папку", "archive");
    parser.addOption(extractHereOption);

    QCommandLineOption extractToOption(QStringList() << "t" << "extract-to",
        "Распаковать архив в указанную папку", "archive");
    parser.addOption(extractToOption);

    parser.process(a);

    MainWindow w;
    
    if (parser.isSet(extractHereOption)) {
        QString archivePath = parser.value(extractHereOption);
        QString destinationPath = QDir::currentPath();
        w.extractArchive(archivePath, destinationPath);
        return 0;
    }
    
    if (parser.isSet(extractToOption)) {
        QString archivePath = parser.value(extractToOption);
        w.extractArchive(archivePath);
        return 0;
    }

    w.show();
    return a.exec();
} 