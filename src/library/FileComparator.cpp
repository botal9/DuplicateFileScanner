//
// Created by vitalya on 11.12.18.
//

#include "FileComparator.h"
#include "retry.h"

#include <fstream>

#include <boost/functional/hash.hpp>

#include <QCryptographicHash>
#include <QDebug>
#include <QIODevice>
#include <QFile>

static const TimeOptions DefaultTimeOptions(/*Count*/3, 0, 0, 0, /*Milliseconds*/30, 0);

static constexpr qint64 BUFFER_SIZE = 128 * 1024;
static constexpr qint64 SMALL_BUFFER_SIZE = 1024;

inline static void TryOpenQFile(QFile& file) {
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::logic_error("Can't open file");
    }
}

FileComparator::FileComparator(std::atomic_bool* needStop)
    : NeedStop(needStop)
{
}

FileComparator::FileComparator(const FileMap& fileMap, std::atomic_bool* needStop)
    : Size2FileList(fileMap)
    , NeedStop(needStop)
{
}

void FileComparator::setFileMap(const FileMap& fileMap) {
    Size2FileList = fileMap;
}

void FileComparator::Process() {
    int skippedFiles = 0;
    for (const auto& fileList : Size2FileList) {
        if (NeedStop->load()) {
            break;
        }
        if (fileList.size() == 1) {
            ++skippedFiles;
            continue;
        }
        ProcessFileList(fileList);
    }
    emit ProcessedFiles(skippedFiles);
    emit Finished();
}

bool FileComparator::Compare(QFile& lhs, QFile& rhs) {
    DoWithRetryThrows(DefaultTimeOptions, TryOpenQFile, lhs);
    DoWithRetryThrows(DefaultTimeOptions, TryOpenQFile, rhs);

    char leftBuffer[BUFFER_SIZE];
    char rightBuffer[BUFFER_SIZE];
    bool equal = true;
    while (!lhs.atEnd() && equal && !NeedStop->load()) {
        lhs.read(leftBuffer, BUFFER_SIZE);
        qint64 len = rhs.read(rightBuffer, BUFFER_SIZE);
        if (memcmp(leftBuffer, rightBuffer, static_cast<std::size_t>(len)) != 0) {
            equal = false;
        }
    }
    lhs.close();
    rhs.close();

    if (NeedStop->load()) {
        return false;
    }
    return equal;
}

QByteArray FileComparator::Hash(QFile& file) {
    DoWithRetryThrows(DefaultTimeOptions, TryOpenQFile, file);

    QCryptographicHash hash(QCryptographicHash::Sha256);

    while (!file.atEnd()) {
        if (NeedStop) {
            file.close();
            return QByteArray();
        }
        hash.addData(file.read(BUFFER_SIZE));
    }
    file.close();

    return hash.result();
}

QByteArray FileComparator::FastHash(QFile& file) {
    DoWithRetryThrows(DefaultTimeOptions, TryOpenQFile, file);

    QCryptographicHash hash(QCryptographicHash::Sha256);

    hash.addData(file.read(SMALL_BUFFER_SIZE));
    file.close();

    return hash.result();
}

void FileComparator::ProcessFileList(const FileList& fileList) {
    QMultiHash<QByteArray, QVector<QString>> hash2file;

    for (const QString& fileName : fileList) {
        if (NeedStop->load()) {
            break;
        }

        QByteArray hash;
        QFile file(fileName);

        try {
            hash = FastHash(file);
        } catch (std::exception& e) {
            qDebug() << e.what();
            continue;
        }

        bool pushed = false;
        auto range = hash2file.equal_range(hash);
        for (auto it = range.first; it != range.second; it++) {
            if (pushed || NeedStop->load()) {
                break;
            }

            auto& sameFiles = it.value();
            assert(!sameFiles.empty());

            for (const QString& sampleName : sameFiles) {
                try {
                    QFile sampleFile(sampleName);
                    if (Compare(file, sampleFile)) {
                        sameFiles.push_back(fileName);
                        pushed = true;
                    }
                    break;
                } catch (std::exception& e) {
                    qDebug() << e.what();
                    continue;
                }
            }
        }

        if (!pushed && !NeedStop->load()) {
            auto it = hash2file.insert(hash, {fileName});
        }
    }

    int skippedFiles = 0;
    for (auto& sameFiles : hash2file) {
        if (sameFiles.size() != 1) {
            emit SendDuplicates(sameFiles);
        } else {
            ++skippedFiles;
        }
    }
    emit ProcessedFiles(skippedFiles);
}
