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

    bool archiveFile(const QString& sourcePath, const QString& destinationPath, 
                    int compressionLevel = 6, bool selfExtracting = false);
    bool extractArchive(const QString& archivePath, const QString& destinationPath);
    QStringList getSupportedFormats() const;
    QString getFormatFromExtension(const QString &extension) const;

private:
    bool writeDataToArchive(struct archive *a, const QString &sourcePath);
    bool processDirectory(struct archive *a, const QString &path, const QString &basePath);
    void setArchiveFormat(struct archive *a, const QString &format);
    bool writeArchiveEntry(struct archive *a, struct archive_entry *entry, 
                          const QString &filePath, const QString &relativePath);
    bool readArchiveEntry(struct archive *a, struct archive_entry *entry, 
                         const QString &destinationPath);
    QString getRelativePath(const QString &basePath, const QString &filePath);
    static int copyData(struct archive *ar, struct archive *aw);
};

#endif // ARCHIVER_H 