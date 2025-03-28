#include "archiver.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <QCoreApplication>

Archiver::Archiver()
{
}

Archiver::~Archiver()
{
}

bool Archiver::archiveFile(const QString& sourcePath, const QString& destinationPath,
                         int compressionLevel, bool selfExtracting)
{
    struct archive *a;
    struct archive_entry *entry;
    int r;

    a = archive_write_new();
    if (!a) {
        qDebug() << "Failed to create archive";
        return false;
    }

    // Устанавливаем формат архива
    QString format = getFormatFromExtension(QFileInfo(destinationPath).suffix());
    setArchiveFormat(a, format);

    // Устанавливаем уровень сжатия
    if (format == "zip" || format == "7z") {
        if (compressionLevel == 0) {
            archive_write_add_filter_none(a);
        } else {
            archive_write_add_filter_gzip(a);
            QString level = QString::number(compressionLevel);
            archive_write_set_options(a, QString("compression-level=%1").arg(level).toUtf8().constData());
        }
    }

    r = archive_write_open_filename(a, destinationPath.toUtf8().constData());
    if (r != ARCHIVE_OK) {
        qDebug() << "Failed to open archive for writing";
        archive_write_free(a);
        return false;
    }

    QFileInfo sourceInfo(sourcePath);
    if (sourceInfo.isDir()) {
        QDir sourceDir(sourcePath);
        QStringList files = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        
        for (const QString &file : files) {
            QString filePath = sourcePath + "/" + file;
            QString relativePath = getRelativePath(sourcePath, filePath);
            
            entry = archive_entry_new();
            if (!entry) {
                qDebug() << "Failed to create archive entry";
                continue;
            }

            QFileInfo fileInfo(filePath);
            archive_entry_set_pathname(entry, relativePath.toUtf8().constData());
            archive_entry_set_size(entry, fileInfo.size());
            archive_entry_set_filetype(entry, fileInfo.isDir() ? AE_IFDIR : AE_IFREG);
            archive_entry_set_perm(entry, fileInfo.permissions());

            if (archive_write_header(a, entry) != ARCHIVE_OK) {
                qDebug() << "Failed to write header";
                archive_entry_free(entry);
                continue;
            }

            if (!fileInfo.isDir()) {
                if (!writeArchiveEntry(a, entry, filePath, relativePath)) {
                    qDebug() << "Failed to write file data";
                }
            }

            archive_entry_free(entry);
        }
    } else {
        entry = archive_entry_new();
        if (!entry) {
            qDebug() << "Failed to create archive entry";
            archive_write_close(a);
            archive_write_free(a);
            return false;
        }

        archive_entry_set_pathname(entry, sourceInfo.fileName().toUtf8().constData());
        archive_entry_set_size(entry, sourceInfo.size());
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, sourceInfo.permissions());

        if (archive_write_header(a, entry) != ARCHIVE_OK) {
            qDebug() << "Failed to write header";
            archive_entry_free(entry);
            archive_write_close(a);
            archive_write_free(a);
            return false;
        }

        if (!writeArchiveEntry(a, entry, sourcePath, sourceInfo.fileName())) {
            qDebug() << "Failed to write file data";
        }

        archive_entry_free(entry);
    }

    archive_write_close(a);
    archive_write_free(a);

    // Если требуется создать самораспаковывающийся архив - исправь работу функции 
    if (selfExtracting) {
        QString extractorPath = QCoreApplication::applicationDirPath() + "/extractor";
        QFile::copy(":/extractor/extractor", extractorPath);
        QFile::setPermissions(extractorPath, QFile::Permissions(QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther));

        QFile sfxScript(":/extractor/sfx.sh");
        if (sfxScript.open(QIODevice::ReadOnly)) {
            QString script = sfxScript.readAll();
            script.replace("ARCHIVE_PATH", destinationPath);
            script.replace("EXTRACTOR_PATH", extractorPath);

            QFile sfxFile(destinationPath + ".sh");
            if (sfxFile.open(QIODevice::WriteOnly)) {
                sfxFile.write(script.toUtf8());
                sfxFile.close();
                QFile::setPermissions(sfxFile.fileName(), QFile::Permissions(QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther));
            }
            sfxScript.close();
        }
    }

    return true;
}

bool Archiver::extractArchive(const QString& archivePath, const QString& destinationPath)
{
    struct archive *a;
    struct archive_entry *entry;
    int r;

    a = archive_read_new();
    if (!a) {
        qDebug() << "Failed to create archive";
        return false;
    }

    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    r = archive_read_open_filename(a, archivePath.toUtf8().constData(), 10240);
    if (r != ARCHIVE_OK) {
        qDebug() << "Failed to open archive for reading";
        archive_read_free(a);
        return false;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        QString pathname = archive_entry_pathname(entry);
        QString fullPath = destinationPath + "/" + pathname;

        if (archive_entry_filetype(entry) == AE_IFDIR) {
            QDir().mkpath(fullPath);
        } else {
            QDir().mkpath(QFileInfo(fullPath).path());
            if (!readArchiveEntry(a, entry, fullPath)) {
                qDebug() << "Failed to read file data";
                archive_read_close(a);
                archive_read_free(a);
                return false;
            }
        }

        archive_entry_free(entry);
    }

    archive_read_close(a);
    archive_read_free(a);
    return true;
}

QString Archiver::getFormatFromExtension(const QString &extension) const
{
    QString ext = extension.toLower();
    if (ext == "zip") return "zip";
    if (ext == "tar") return "tar";
    if (ext == "gz" || ext == "tgz") return "gzip";
    if (ext == "7z") return "7zip";
    if (ext == "rar") return "rar";
    return "zip"; // По умолчанию
}

void Archiver::setArchiveFormat(struct archive *a, const QString &format)
{
    if (format == "zip") {
        archive_write_set_format_zip(a);
    } else if (format == "tar") {
        archive_write_set_format_ustar(a);
    } else if (format == "gzip") {
        archive_write_set_format_ustar(a);
        archive_write_add_filter_gzip(a);
    } else if (format == "7zip") {
        archive_write_set_format_7zip(a);
    }
}

bool Archiver::writeArchiveEntry(struct archive *a, struct archive_entry *entry,
                               const QString &filePath, const QString &relativePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file for reading:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    if (archive_write_data(a, data.constData(), data.size()) != data.size()) {
        qDebug() << "Failed to write data to archive";
        return false;
    }

    return true;
}

bool Archiver::readArchiveEntry(struct archive *a, struct archive_entry *entry,
                              const QString &destinationPath)
{
    QFile file(destinationPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open file for writing:" << destinationPath;
        return false;
    }

    const void *buff;
    size_t size;
    la_int64_t offset;

    while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK) {
        if (file.write(static_cast<const char*>(buff), size) != size) {
            qDebug() << "Failed to write data to file";
            file.close();
            return false;
        }
    }

    file.close();
    return true;
}

QString Archiver::getRelativePath(const QString &basePath, const QString &filePath)
{
    QDir baseDir(basePath);
    return baseDir.relativeFilePath(filePath);
}

QStringList Archiver::getSupportedFormats() const
{
    return {"ZIP", "TAR", "TAR.GZ", "7Z"};
} 