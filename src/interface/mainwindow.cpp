#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <memory>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , NeedStop(false)
{
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    QCommonStyle style;
    setWindowTitle(QString("Duplicate File FileComparator"));

    ui->actionScanDirectory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));
    ui->actionStopScanning->setIcon(style.standardIcon(QCommonStyle::SP_DialogDiscardButton));
    ui->actionDelete->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    ui->statusAction->setText("Choose directory");
    ui->statusScanned->hide();
    ui->selectedDirectory->hide();

    ui->progressBar->reset();
    ui->progressBar->hide();
    ui->actionStopScanning->setVisible(false);
    ui->actionDelete->setVisible(false);

    connect(ui->actionScanDirectory, &QAction::triggered, this, &MainWindow::SelectDirectory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionCollapseAll, &QAction::triggered, this, &MainWindow::CollapseAllRows);
    connect(ui->actionExpandAll, &QAction::triggered, this, &MainWindow::ExpandAllRows);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::ShowAbout);
    connect(ui->actionStopScanning, &QAction::triggered, this, &MainWindow::Stop);
    connect(ui->actionDelete, &QAction::triggered, this, &MainWindow::Delete);

    Time.start();
}

MainWindow::~MainWindow() {
    NeedStop.store(true);
    for (QThread* thread : WorkingThreads) {
        if (thread != nullptr && !thread->isFinished()) {
            thread->quit();
        }
    }
    for (QThread* thread : WorkingThreads) {
        if (thread != nullptr && !thread->isFinished()) {
            thread->wait();
        }
    }
}

void MainWindow::Stop() {
    NeedStop.store(true);
}

void MainWindow::CollapseAllRows() {
    ui->treeWidget->collapseAll();
}

void MainWindow::ExpandAllRows() {
    ui->treeWidget->expandAll();
}

void MainWindow::ShowAbout() {
    QMessageBox::aboutQt(this);
}

void MainWindow::SelectDirectory() {
    SelectedDirectory = QFileDialog::getExistingDirectory(
            this,
            "Select Directory for Scanning",
            QString(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    SelectedDirectoryInfo = QFileInfo(SelectedDirectory.absolutePath());
    BeautySelectedDirectory = QDir::toNativeSeparators(SelectedDirectoryInfo.absoluteFilePath()).append(QDir::separator());

    Time.restart();
    if (!SelectedDirectoryInfo.exists()) {
        return;
    }

    SetupInterface();

    NeedStop = false;
    QThread* thread = new QThread;
    Worker* worker = new Worker(SelectedDirectory.absolutePath(), this, &NeedStop);
    worker->moveToThread(thread);

    connect(thread, SIGNAL(started()), worker, SLOT(Process()));
    connect(worker, SIGNAL(Finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(Finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT (deleteLater()));
    connect(worker, SIGNAL(SetupFilesNumber(uint64_t)), this, SLOT(SetupProgressBar(uint64_t )));
    connect(worker, SIGNAL(Aborted()), this, SLOT(PostProcessAbort()));
    connect(worker, SIGNAL(Finished()), this, SLOT(PostProcessFinish()));

    WorkingThreads.push_back(thread);
    thread->start();
}

void MainWindow::AddDuplicatesList(const FileList &duplicates) {
    UpdateProgressBar(duplicates.size());
    QRegExp regExp(BeautySelectedDirectory);

    QTreeWidgetItem* item = new TreeWidgetItem();
    item->setText(0, QString(DUPLICATES_PREFIX).append(QString::number(duplicates.size())).append(DUPLICATES_SUFFIX));
    QFileInfo fileInfo(duplicates[0]);
    item->setText(1, QString::number(fileInfo.size()));

    for (const QString& fileName : duplicates) {
        QTreeWidgetItem* childItem = new TreeWidgetItem();

        QFileInfo fileInfo(fileName);
        QString beautyName = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
        beautyName.remove(regExp);

        childItem->setText(0, beautyName);
        item->addChild(childItem);
    }
    ui->treeWidget->addTopLevelItem(item);
}

void MainWindow::SetupProgressBar(uint64_t filesNumber) {
    ui->progressBar->show();
    ui->progressBar->setRange(0, filesNumber);
}

void MainWindow::UpdateProgressBar(uint64_t filesNumber) {
    ui->progressBar->setValue(ui->progressBar->value() + filesNumber);
}

void MainWindow::PostProcessInterface(bool success) {
    ui->progressBar->hide();
    ui->statusScanned->setText(QString("Files scanned: ").append(QString::number(ui->progressBar->value())));
    ui->statusScanned->show();
    if (success) {
        ui->statusAction->setText("Finished");
    } else {
        ui->statusAction->setText("Aborted");
    }
    ui->statusAction->show();
    ui->treeWidget->setSortingEnabled(true);
    qDebug("Time elapsed: %d ms", Time.elapsed());
}

void MainWindow::PostProcessFinish() {
    PostProcessInterface(/*success*/true);
}

void MainWindow::PostProcessAbort() {
    PostProcessInterface(/*success*/false);
}

void MainWindow::SetupInterface() {
    ui->statusAction->hide();
    ui->statusScanned->hide();
    ui->selectedDirectory->setText("Selected directory: " + BeautySelectedDirectory);
    ui->selectedDirectory->show();
    ui->actionStopScanning->setVisible(true);
    ui->actionDelete->setVisible(true);
    ui->treeWidget->setVisible(true);
    ui->treeWidget->clear();
}

void MainWindow::Delete() {
    auto selectedItems = ui->treeWidget->selectedItems();
    QVector<QTreeWidgetItem*> itemsToDelete;

    for (const auto& item : selectedItems) {
        if (item->childCount() == 0) {
            itemsToDelete.push_back(item);
        }
    }

    QString confirmationQuestion = "Do you want to delete selected file(s)?";
    auto answer = QMessageBox::question(this, "Delete file(s)", confirmationQuestion);
    if (answer == QMessageBox::No) {
        return;
    }

    QMap<QString, QTreeWidgetItem*> deletedFiles;
    QVector<QString> skippedFiles;
    for (const auto& item : itemsToDelete) {
        QFile file(item->text(0));
        if (file.remove()) {
            deletedFiles.insert(file.fileName(), item);
        } else {
            skippedFiles.push_back(file.fileName());
        }
    }

    for (const auto& item : deletedFiles) {
        auto* parent = item->parent();
        parent->removeChild(item);
        dynamic_cast<TreeWidgetItem*>(parent)->SetNumber(0, parent->childCount());
        if (parent->childCount() == 0) {
            delete parent;
        }
    }

    QString operationInfo = QString("Can't delete ").append(QString::number(skippedFiles.size())).append(" file(s):\n");
    for (const QString& fileName : skippedFiles) {
        operationInfo.append(fileName).append("\n");
    }

    ui->treeWidget->clearSelection();
    if (!skippedFiles.empty()) {
        QMessageBox::information(this, "Can't delete file(s)", operationInfo);
    }
}

