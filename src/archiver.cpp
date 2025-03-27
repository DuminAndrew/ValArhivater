#include "archiver.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <fcntl.h>
#include <unistd.h>

Archiver::Archiver()
{
}

Archiver::~Archiver()
{
}

bool Archiver::archiveFile(const QString& sourcePath, const QString& archivePath)
{
    QFileInfo fileInfo(sourcePath);
    if (!fileInfo.exists()) {
        qDebug() << "Source path does not exist:" << sourcePath;
        return false;
    }

    struct archive *a = archive_write_new();
    if (!a) {
        qDebug() << "Failed to create archive writer";
        return false;
    }

    if (archive_write_set_format_zip(a) != ARCHIVE_OK) {
        qDebug() << "Failed to set ZIP format";
        archive_write_free(a);
        return false;
    }

    if (archive_write_open_filename(a, archivePath.toUtf8().constData()) != ARCHIVE_OK) {
        qDebug() << "Failed to open archive for writing:" << archivePath;
        archive_write_free(a);
        return false;
    }

    QDir sourceDir(sourcePath);
    QStringList files = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    
    for (const QString& file : files) {
        QString fullPath = sourceDir.filePath(file);
        QFileInfo fileInfo(fullPath);
        
        struct archive_entry *entry = archive_entry_new();
        if (!entry) {
            qDebug() << "Failed to create archive entry";
            archive_write_close(a);
            archive_write_free(a);
            return false;
        }

        // Устанавливаем относительный путь в архиве
        QString relativePath = QDir(sourcePath).relativeFilePath(fullPath);
        archive_entry_set_pathname(entry, relativePath.toUtf8().constData());
        
        if (fileInfo.isDir()) {
            archive_entry_set_filetype(entry, AE_IFDIR);
            archive_entry_set_perm(entry, 0755);
        } else {
            archive_entry_set_filetype(entry, AE_IFREG);
            archive_entry_set_size(entry, fileInfo.size());
            archive_entry_set_perm(entry, 0644);
        }

        if (archive_write_header(a, entry) != ARCHIVE_OK) {
            qDebug() << "Failed to write archive header for:" << relativePath;
            archive_entry_free(entry);
            archive_write_close(a);
            archive_write_free(a);
            return false;
        }

        if (fileInfo.isFile()) {
            int fd = open(fullPath.toUtf8().constData(), O_RDONLY);
            if (fd < 0) {
                qDebug() << "Failed to open file:" << fullPath;
                archive_entry_free(entry);
                archive_write_close(a);
                archive_write_free(a);
                return false;
            }

            char buff[8192];
            ssize_t len;
            while ((len = read(fd, buff, sizeof(buff))) > 0) {
                if (archive_write_data(a, buff, len) < 0) {
                    qDebug() << "Failed to write data to archive for:" << relativePath;
                    close(fd);
                    archive_entry_free(entry);
                    archive_write_close(a);
                    archive_write_free(a);
                    return false;
                }
            }

            if (len < 0) {
                qDebug() << "Failed to read from file:" << fullPath;
                close(fd);
                archive_entry_free(entry);
                archive_write_close(a);
                archive_write_free(a);
                return false;
            }

            close(fd);
        }

        archive_entry_free(entry);
    }

    archive_write_close(a);
    archive_write_free(a);

    return true;
}

bool Archiver::extractArchive(const QString& archivePath, const QString& extractPath)
{
    if (!QFileInfo(archivePath).exists()) {
        qDebug() << "Archive file does not exist:" << archivePath;
        return false;
    }

    QDir extractDir(extractPath);
    if (!extractDir.exists() && !extractDir.mkpath(".")) {
        qDebug() << "Failed to create extraction directory:" << extractPath;
        return false;
    }

    struct archive *a = archive_read_new();
    if (!a) {
        qDebug() << "Failed to create archive reader";
        return false;
    }

    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    int r = archive_read_open_filename(a, archivePath.toUtf8().constData(), 10240);
    if (r != ARCHIVE_OK) {
        qDebug() << "Failed to open archive for reading:" << archivePath;
        archive_read_free(a);
        return false;
    }

    struct archive_entry *entry;
    char buff[8192];
    int result = ARCHIVE_OK;

    while ((result = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
        QString pathname = QString::fromUtf8(archive_entry_pathname(entry));
        QString fullPath = extractDir.filePath(pathname);

        if (archive_entry_filetype(entry) == AE_IFDIR) {
            if (!QDir().mkpath(fullPath)) {
                qDebug() << "Failed to create directory:" << fullPath;
                archive_read_close(a);
                archive_read_free(a);
                return false;
            }
            continue;
        }

        QFile file(fullPath);
        QFileInfo fileInfo(fullPath);
        QDir().mkpath(fileInfo.path());

        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "Failed to open file for writing:" << fullPath;
            archive_read_close(a);
            archive_read_free(a);
            return false;
        }

        while (true) {
            ssize_t size = archive_read_data(a, buff, sizeof(buff));
            if (size < 0) {
                qDebug() << "Failed to read data from archive";
                file.close();
                archive_read_close(a);
                archive_read_free(a);
                return false;
            }
            if (size == 0) {
                break;
            }
            if (file.write(buff, size) != size) {
                qDebug() << "Failed to write data to file:" << fullPath;
                file.close();
                archive_read_close(a);
                archive_read_free(a);
                return false;
            }
        }

        file.close();
    }

    if (result != ARCHIVE_EOF) {
        qDebug() << "Error reading archive:" << archive_error_string(a);
        archive_read_close(a);
        archive_read_free(a);
        return false;
    }

    archive_read_close(a);
    archive_read_free(a);
    return true;
}

QStringList Archiver::getSupportedFormats() const
{
    return {"zip", "tar", "gz"};
} 