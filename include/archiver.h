#ifndef ARCHIVER_H
#define ARCHIVER_H

#include <QString>
#include <QStringList>
#include <archive.h>
#include <archive_entry.h>

class Archiver
{
public:
    Archiver();
    ~Archiver();

    bool archiveFile(const QString& sourcePath, const QString& archivePath);
    bool extractArchive(const QString& archivePath, const QString& extractPath);
    QStringList getSupportedFormats() const;
};

#endif // ARCHIVER_H 