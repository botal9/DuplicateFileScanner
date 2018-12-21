//
// Created by vitalya on 11.12.18.
//

#include "FileComparator.h"
#include "retry.h"

#include <fstream>

#include <boost/functional/hash.hpp>

#include <QtCore/QDebug>

static const TimeOptions DefaultTimeOptions(/*Count*/3, 0, 0, 0, /*Milliseconds*/30, 0);

static constexpr std::size_t BUFFER_SIZE = 128 * 1024;
static constexpr std::size_t SMALL_BUFFER_SIZE = 1024;

inline static void TryOpen(const QString& file, std::ifstream& in) {
    in.open(file.toStdString(), std::ifstream::in);
    if (!in.is_open()) {
        throw std::logic_error("Can't open file");
    }
}

FileComparator::FileComparator(std::atomic_bool* needStop)
    : NeedStop(needStop)
{
}

FileComparator::FileComparator(const FileMap& fileMap, std::atomic_bool* needStop)
    : Hash2FileList(fileMap)
    , NeedStop(needStop)
{
}

void FileComparator::setFileMap(const FileMap& fileMap) {
    Hash2FileList = fileMap;
}

void FileComparator::Process() {
    int skippedFiles = 0;
    for (const auto& fileList : Hash2FileList) {
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

bool FileComparator::Compare(const QString &lhs, const QString &rhs) {
    std::ifstream inLeft;
    std::ifstream inRight;

    bool openedLeft = DoWithRetry(DefaultTimeOptions, TryOpen, lhs, inLeft);
    bool openedRight = DoWithRetry(DefaultTimeOptions, TryOpen, rhs, inRight);

    if (!openedLeft || !openedRight) {
        std::string err = "Can't open " + std::string(openedLeft ? "first" : "second") +
                " file while comparing files: " + lhs.toStdString() + rhs.toStdString();
        throw std::logic_error(err);
    }

    char leftBuffer[BUFFER_SIZE];
    char rightBuffer[BUFFER_SIZE];
    bool equal = true;
    while (!inLeft.eof() && equal && !NeedStop->load()) {
        inLeft.read(leftBuffer, BUFFER_SIZE);
        inRight.read(rightBuffer, BUFFER_SIZE);
        auto len = inLeft.gcount();
        if (memcmp(leftBuffer, rightBuffer, static_cast<std::size_t>(len)) != 0) {
            equal = false;
        }
    }
    inLeft.close();
    inRight.close();

    if (NeedStop->load()) {
        return false;
    }
    return equal;
}

uint64_t FileComparator::Hash(const QString& file) {
    std::ifstream in;
    DoWithRetryThrows(DefaultTimeOptions, TryOpen, file, in);

    std::size_t result = 0;
    char buf[BUFFER_SIZE];

    while (!in.eof()) {
        in.read(buf, BUFFER_SIZE);
        auto len = in.gcount();
        boost::hash_combine(result, boost::hash_value(std::string(buf, buf + len)));
    }
    in.close();
    return result;
}

uint64_t FileComparator::FastHash(const QString& file) {
    std::ifstream in;
    DoWithRetryThrows(DefaultTimeOptions, TryOpen, file, in);

    char buf[SMALL_BUFFER_SIZE];

    in.read(buf, SMALL_BUFFER_SIZE);
    auto len = in.gcount();

    in.close();
    return boost::hash_value(std::string(buf, buf + len));
}

void FileComparator::ProcessFileList(const FileList& fileList) {
    QMultiHash<std::size_t, QVector<QString>> hash2file;

    for (const QString& file : fileList) {
        if (NeedStop->load()) {
            break;
        }

        uint64_t hash;

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

            for (const auto& sample : sameFiles) {
                try {
                    if (Compare(file, sample)) {
                        sameFiles.push_back(file);
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
            hash2file.insert(hash, {file});
        }
    }

    uint64_t skippedFiles = 0;
    for (auto& sameFiles : hash2file) {
        if (sameFiles.size() != 1) {
            emit SendDuplicates(sameFiles);
        } else {
            ++skippedFiles;
        }
    }
    emit ProcessedFiles(skippedFiles);
}
