//
// Created by vitalya on 09.01.19.
//

#include <gtest/gtest.h>
#include <src/library/Worker.h>

#include <iostream>
#include <fstream>

#include <QObject>
#include <QFile>
#include <QDir>
#include <QFileDevice>

static constexpr bool REMOVE_AFTER_ENDING = true;

namespace {
    class Checker : public QObject {
        Q_OBJECT

    public:
        Checker(DirectoryScanner* directoryScanner, FileComparator* fileComparator) {
            connect(fileComparator, SIGNAL(SendDuplicates(const FileList&)), this, SLOT(UpdateDuplicates(const FileList&)));
            connect(directoryScanner, SIGNAL(Finished()), this, SLOT(UpdateSuccess()));
            connect(fileComparator, SIGNAL(Finished()), this, SLOT(UpdateSuccess()));
        }

    public slots:
        void UpdateDuplicates(const FileList& fileList) {
            Duplicates.push_back(fileList);
            ProcessedFilesByComparator += fileList.size();
        }

        void UpdateSuccess() {
            ++Success;
        }

    public:
        void clear() {
            ProcessedFilesByComparator = 0;
            ProcessedFilesByScanner = 0;
            Duplicates.clear();
            Success = 0;
        }

        bool Successfully() {
            return Success == 2;
        }

    public:
        int ProcessedFilesByComparator = 0;
        uint64_t ProcessedFilesByScanner = 0;
        QVector<FileList> Duplicates;
        int Success = 0;
    };

    void createFile(const QString& path, const QString& fileName, const QString& fileData) {
        QFile file(path + fileName);
        file.open(QIODevice::WriteOnly);
        QTextStream stream1(&file);
        stream1 << fileData << endl;
        file.close();
    }

    void process(DirectoryScanner*& directoryScanner, FileComparator*& fileComparator,
            Checker*& checker, const QString& path)
    {
        FileMap fileMap;
        std::atomic_bool needStop = false;

        directoryScanner = new DirectoryScanner(path, &needStop);
        fileComparator = new FileComparator(&needStop);
        checker = new Checker(directoryScanner, fileComparator);

        directoryScanner->RecursiveScanReduceBySize(fileMap, checker->ProcessedFilesByScanner);
        fileComparator->setFileMap(fileMap);
        fileComparator->Process();

        if (REMOVE_AFTER_ENDING) {
            QDir dir;
            dir.cd(path);
            dir.removeRecursively();
        }
    }
}

TEST(correctness, empty_files)
{
    QString dirName("empty_files");
    QString fileName1("file1.empty");
    QString fileName2("file2.empty");

    QString path = QDir::currentPath().append(QDir::separator()).append(dirName).append(QDir::separator());
    QDir dir;
    dir.mkpath(path);

    createFile(path, fileName1, "");
    createFile(path, fileName2, "");

    DirectoryScanner* directoryScanner = nullptr;
    FileComparator* fileComparator = nullptr;
    Checker* checker = nullptr;

    process(directoryScanner, fileComparator, checker, path);

    std::sort(checker->Duplicates.front().begin(), checker->Duplicates.front().end());
    EXPECT_EQ(checker->Duplicates.front().front(), path + fileName1);
    EXPECT_EQ(checker->Duplicates.front().back(), path + fileName2);

    EXPECT_TRUE(checker->Successfully());
    EXPECT_EQ(checker->ProcessedFilesByScanner, 2);
    EXPECT_EQ(checker->ProcessedFilesByComparator, 2);
    EXPECT_EQ(checker->Duplicates.size(), 1);
    EXPECT_EQ(checker->Duplicates.front().size(), 2);

    delete checker;
    delete directoryScanner;
    delete fileComparator;
}

TEST(correctness, one_file)
{
    QString dirName("one_file");
    QString text("abacabadabacaba");
    QString fileName("file.alone");

    QString path = QDir::currentPath().append(QDir::separator()).append(dirName).append(QDir::separator());
    QDir dir;
    dir.mkpath(path);

    createFile(path, fileName, text);

    DirectoryScanner* directoryScanner = nullptr;
    FileComparator* fileComparator = nullptr;
    Checker* checker = nullptr;

    process(directoryScanner, fileComparator, checker, path);

    EXPECT_TRUE(checker->Successfully());
    EXPECT_EQ(checker->ProcessedFilesByScanner, 1);
    EXPECT_EQ(checker->ProcessedFilesByComparator, 0);
    EXPECT_TRUE(checker->Duplicates.empty());

    delete checker;
    delete directoryScanner;
    delete fileComparator;
}

TEST(correctness, same_files)
{
    QString dirName("same_files");
    QString text("abacaba");
    QString fileName1("file1.same");
    QString fileName2("file2.same");
    QString fileName3("file3.same");

    QString path = QDir::currentPath().append(QDir::separator()).append(dirName).append(QDir::separator());
    QDir dir;
    dir.mkpath(path);

    createFile(path, fileName1, text);
    createFile(path, fileName2, text);
    createFile(path, fileName3, text);

    DirectoryScanner* directoryScanner = nullptr;
    FileComparator* fileComparator = nullptr;
    Checker* checker = nullptr;

    process(directoryScanner, fileComparator, checker, path);

    EXPECT_TRUE(checker->Successfully());
    EXPECT_EQ(checker->ProcessedFilesByScanner, 3);
    EXPECT_EQ(checker->ProcessedFilesByComparator, 3);
    EXPECT_EQ(checker->Duplicates.size(), 1);
    EXPECT_EQ(checker->Duplicates.front().size(), 3);

    delete checker;
    delete directoryScanner;
    delete fileComparator;
}

TEST(correctness, different_files_same_hash)
{
    QString dirName("different_files");
    QString text1(2048, 'a');
    QString text2(2028, 'a');
    text1.append(QString(2048, 'b'));
    text2.append(QString(2048, 'c'));
    QString fileName1("file1.aba");
    QString fileName2("file2.aba");
    QString fileName3("file3.abacaba");
    QString fileName4("file4.abacaba");

    QString path = QDir::currentPath().append(QDir::separator()).append(dirName).append(QDir::separator());
    QDir dir;
    dir.mkpath(path);

    createFile(path, fileName1, text1);
    createFile(path, fileName2, text1);
    createFile(path, fileName3, text2);
    createFile(path, fileName4, text2);

    DirectoryScanner* directoryScanner = nullptr;
    FileComparator* fileComparator = nullptr;
    Checker* checker = nullptr;

    process(directoryScanner, fileComparator, checker, path);

    EXPECT_TRUE(checker->Successfully());
    EXPECT_EQ(checker->ProcessedFilesByScanner, 4);
    EXPECT_EQ(checker->ProcessedFilesByComparator, 4);
    EXPECT_EQ(checker->Duplicates.size(), 2);
    EXPECT_EQ(checker->Duplicates[0].size(), 2);
    EXPECT_EQ(checker->Duplicates[1].size(), 2);

    delete checker;
    delete directoryScanner;
    delete fileComparator;
}

TEST(correctness, different_files)
{
    QString dirName("different_files");
    QString text1("aba");
    QString text2("abacaba");
    QString fileName1("file1.aba");
    QString fileName2("file2.aba");
    QString fileName3("file3.abacaba");
    QString fileName4("file4.abacaba");

    QString path = QDir::currentPath().append(QDir::separator()).append(dirName).append(QDir::separator());
    QDir dir;
    dir.mkpath(path);

    createFile(path, fileName1, text1);
    createFile(path, fileName2, text1);
    createFile(path, fileName3, text2);
    createFile(path, fileName4, text2);

    DirectoryScanner* directoryScanner = nullptr;
    FileComparator* fileComparator = nullptr;
    Checker* checker = nullptr;

    process(directoryScanner, fileComparator, checker, path);

    EXPECT_TRUE(checker->Successfully());
    EXPECT_EQ(checker->ProcessedFilesByScanner, 4);
    EXPECT_EQ(checker->ProcessedFilesByComparator, 4);
    EXPECT_EQ(checker->Duplicates.size(), 2);
    EXPECT_EQ(checker->Duplicates[0].size(), 2);
    EXPECT_EQ(checker->Duplicates[1].size(), 2);

    delete checker;
    delete directoryScanner;
    delete fileComparator;
}

TEST(correctness, big_files)
{
    QString dirName("same_files");
    QString text(10000000, 'a'); // 10^7
    QString fileName1("file1.same");
    QString fileName2("file2.same");
    QString fileName3("file3.same");

    QString path = QDir::currentPath().append(QDir::separator()).append(dirName).append(QDir::separator());
    QDir dir;
    dir.mkpath(path);

    createFile(path, fileName1, text);
    createFile(path, fileName2, text);
    createFile(path, fileName3, text);

    DirectoryScanner* directoryScanner = nullptr;
    FileComparator* fileComparator = nullptr;
    Checker* checker = nullptr;

    process(directoryScanner, fileComparator, checker, path);

    EXPECT_TRUE(checker->Successfully());
    EXPECT_EQ(checker->ProcessedFilesByScanner, 3);
    EXPECT_EQ(checker->ProcessedFilesByComparator, 3);
    EXPECT_EQ(checker->Duplicates.size(), 1);
    EXPECT_EQ(checker->Duplicates.front().size(), 3);

    delete checker;
    delete directoryScanner;
    delete fileComparator;
}

TEST(correctness, subdirectory)
{
    QString dirName("directory");
    QString subdirName("subdirectory");
    QString text1("1111111");
    QString text2("abacaba");
    QString fileName1("file1.aba");
    QString fileName2("file2.aba");
    QString fileName3("file3.abacaba");

    QString path = QDir::currentPath().append(QDir::separator()).append(dirName).append(QDir::separator());
    QString subPath = path + subdirName + QDir::separator();
    QDir dir;
    dir.mkpath(subPath);

    createFile(path, fileName1, text1);
    createFile(subPath, fileName2, text1);
    createFile(subPath, fileName3, text2);

    DirectoryScanner* directoryScanner = nullptr;
    FileComparator* fileComparator = nullptr;
    Checker* checker = nullptr;

    process(directoryScanner, fileComparator, checker, path);

    EXPECT_TRUE(checker->Successfully());
    EXPECT_EQ(checker->ProcessedFilesByScanner, 3);
    EXPECT_EQ(checker->ProcessedFilesByComparator, 2);
    EXPECT_EQ(checker->Duplicates.size(), 1);
    EXPECT_EQ(checker->Duplicates[0].size(), 2);

    delete checker;
    delete directoryScanner;
    delete fileComparator;
}

#include "tests.moc"
