//
// Created by vitalya on 08.12.18.
//

#include "FileComparator.h"
#include "retry.h"

#include <iostream>
#include <fstream>

#include <boost/functional/hash.hpp>

//static const TimeOptions DefaultTimeOptions{.Count = 3, .Milliseconds = 100};
static const TimeOptions DefaultTimeOptions(/*Count*/3, 0, 0, 0, /*Milliseconds*/100, 0);


inline static void TryOpen(const QString& file, std::ifstream& in) {
    in.open(file.toStdString(), std::ifstream::in);
    if (!in.is_open()) {
        throw std::logic_error("Can't open file");
    }
}


FileComparator::FileComparator(const FileList& files, SpscQueue& queue)
    : QRunnable()
    , Files(files)
    , Queue(queue)
{}

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
    while (!inl.eof() && equal) {
        inl.read(bufl, BUFFER_SIZE);
        inr.read(bufr, BUFFER_SIZE);
        auto len = inl.gcount();
        if (memcmp(bufl, bufr, static_cast<std::size_t>(len)) != 0) {
            equal = false;
        }
    }
    inl.close();
    inr.close();
    return equal;
}

void FileComparator::run() {
    for (const QString& file : Files) {
        std::size_t hash;

        try {
            hash = Hash(file);
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            continue;
        }

        bool pushed = false;
        auto range = hash2file.equal_range(hash);
        for (auto it = range.first; it != range.second; it++) {
            if (pushed) {
               break;
            }

            auto& sameFiles = it.value();
            assert(!sameFiles.empty());

            for (const auto& sample : sameFiles) {
                if (pushed) {
                    break;
                }

                try {
                    if (Compare(file, sample)) {
                        sameFiles.push_back(file);
                        pushed = true;
                    }
                } catch (std::exception& e) {
                    continue;
                }
            }
        }

        if (!pushed) {
            hash2file.insert(hash, {file});
        }
    }

    for (const auto& sameFiles : hash2file) {
        Queue.push(sameFiles);
    }
}


