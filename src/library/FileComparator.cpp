//
// Created by vitalya on 11.12.18.
//

#include "FileComparator.h"
#include "retry.h"

#include <fstream>

#include <boost/functional/hash.hpp>

#include <QtCore/QDebug>

static const TimeOptions DefaultTimeOptions(/*Count*/1, 0, 0, 0, /*Milliseconds*/20, 0);

static constexpr std::size_t BUFFER_SIZE = 128 * 1024;
static constexpr std::size_t SMALL_BUFFER_SIZE = 1024;

inline static void TryOpen(const QString& file, std::ifstream& in) {
    in.open(file.toStdString(), std::ifstream::in);
    if (!in.is_open()) {
        throw std::logic_error("Can't open file");
    }
}

FileComparator::FileComparator(const FileMap &fileMap, std::atomic_bool* needStop)
    : Hash2FileList(fileMap)
    , NeedStop(needStop)
{
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
    std::ifstream inl;
    std::ifstream inr;

    bool okl = DoWithRetry(DefaultTimeOptions, TryOpen, lhs, inl);
    bool okr = DoWithRetry(DefaultTimeOptions, TryOpen, rhs, inr);

    if (!okl || !okr) {
        std::string err = "Can't open file while comparing, file: " + (!okl ? lhs.toStdString() : rhs.toStdString());
        throw std::logic_error(err);
    }

    char bufl[BUFFER_SIZE];
    char bufr[BUFFER_SIZE];
    bool equal = true;
    while (!inl.eof() && equal && !NeedStop->load()) {
        inl.read(bufl, BUFFER_SIZE);
        inr.read(bufr, BUFFER_SIZE);
        auto len = inl.gcount();
        if (memcmp(bufl, bufr, static_cast<std::size_t>(len)) != 0) {
            equal = false;
        }
    }
    inl.close();
    inr.close();

    if (NeedStop->load()) {
        return false;
    }
    return equal;
}

std::size_t FileComparator::Hash(const QString& file) {
    std::ifstream in;

    bool ok = DoWithRetry(DefaultTimeOptions, TryOpen, file, in);
    if (!ok) {
        throw std::logic_error("Can't calculate file hash, file: " + file.toStdString());
    }

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

std::size_t FileComparator::FastHash(const QString &file) {
    std::ifstream in;

    bool ok = DoWithRetry(DefaultTimeOptions, TryOpen, file, in);
    if (!ok) {
        throw std::logic_error("Can't calculate file hash, file: " + file.toStdString());
    }

    char buf[SMALL_BUFFER_SIZE];

    in.read(buf, SMALL_BUFFER_SIZE);
    auto len = in.gcount();

    in.close();
    return boost::hash_value(std::string(buf, buf + len));
}

void FileComparator::ProcessFileList(const FileList &fileList) {
    QMultiHash<std::size_t, QVector<QString>> hash2file;

    for (const QString& file : fileList) {
        if (NeedStop->load()) {
            break;
        }

        std::size_t hash;

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
                    continue;
                }
            }
        }

        if (!pushed && !NeedStop->load()) {
            hash2file.insert(hash, {file});
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
