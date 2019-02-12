#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <memory>
#include <thread>

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
    Stop();
    ResetWorkingThread();
    ResetDeletingThread();
}

void MainWindow::Stop() {
    NeedStop = true;
    emit StopAll();
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
    QFileInfo selectedDirectoryInfo = QFileInfo(SelectedDirectory.absolutePath());
    BeautySelectedDirectory = QDir::toNativeSeparators(selectedDirectoryInfo.absoluteFilePath()).append(QDir::separator());

    Time.restart();
    if (!selectedDirectoryInfo.exists()) {
        return;
    }

    NeedStop = false;
    SetupInterface();

    WorkingThread = new QThread();
    Worker* worker = new Worker(SelectedDirectory.absolutePath(), this);
    worker->moveToThread(WorkingThread);

    connect(WorkingThread, SIGNAL(started()), worker, SLOT(Process()));
    connect(worker, SIGNAL(Finished()), WorkingThread, SLOT(quit()));
    connect(worker, SIGNAL(Finished()), this, SLOT(PostProcessFinish()));
    connect(WorkingThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(WorkingThread, SIGNAL(finished()), WorkingThread, SLOT (deleteLater()));
    connect(worker, SIGNAL(SetupFilesNumber(int)), this, SLOT(SetupProgressBar(int)));
    connect(worker, SIGNAL(Aborted()), this, SLOT(PostProcessAbort()));
    connect(this, SIGNAL(StopAll()), worker, SLOT(Stop()), Qt::DirectConnection);

    WorkingThread->start();
}

void MainWindow::AddDuplicatesList(const FileList& duplicates) {
    UpdateProgressBar(duplicates.size());
    QRegExp regExp(BeautySelectedDirectory);

    QTreeWidgetItem* item = new TreeWidgetItem();
    item->setText(0, QString(DUPLICATES_PREFIX).append(QString::number(duplicates.size())).append(DUPLICATES_SUFFIX));
    QFileInfo fileInfo(duplicates[0]);
    item->setText(1, QString::number(fileInfo.size()));

    for (const QFile& file : duplicates) {
        QTreeWidgetItem* childItem = new TreeWidgetItem();

        QFileInfo fileInfo(file);
        QString beautyName = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
        beautyName.remove(regExp);

        childItem->setText(0, beautyName);
        item->addChild(childItem);
    }
    ui->treeWidget->addTopLevelItem(item);
}

void MainWindow::SetupProgressBar(int filesNumber) {
    ui->progressBar->show();
    ui->progressBar->setRange(0, filesNumber);
}

void MainWindow::UpdateProgressBar(int filesNumber) {
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
    WorkingThread = nullptr;
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
    ui->treeWidget->setSortingEnabled(false);
}

void MainWindow::Delete() {
    auto selectedItems = ui->treeWidget->selectedItems();
    QVector<QTreeWidgetItem*> itemsToDelete;

    for (const auto& item : selectedItems) {
        if (item->childCount() == 0) {
            itemsToDelete.push_back(item);
        }
    }
    if (itemsToDelete.empty()) {
        return;
    }

    QString confirmationQuestion = "Do you want to delete selected file(s)?";
    auto answer = QMessageBox::question(this, "Delete file(s)", confirmationQuestion);
    if (answer == QMessageBox::No) {
        return;
    }

    ui->treeWidget->setSortingEnabled(false);

    DeletingThread = new QThread();
    Deleter* deleter = new Deleter(std::move(itemsToDelete), SelectedDirectory.absolutePath());
    deleter->moveToThread(DeletingThread);

    connect(DeletingThread, SIGNAL(started()), deleter, SLOT(DeleteItems()));
    connect(deleter, SIGNAL(Finished()), DeletingThread, SLOT(quit()));
    connect(deleter, SIGNAL(Finished(const FileList&, const FileList&)), this, SLOT(PostProcessDelete(const FileList&, const FileList&)));
    connect(DeletingThread, SIGNAL(finished()), deleter, SLOT(deleteLater()));
    connect(DeletingThread, SIGNAL(finished()), DeletingThread, SLOT (deleteLater()));
    connect(this, SIGNAL(StopAll()), deleter, SLOT(Stop()), Qt::DirectConnection);

    DeletingThread->start();
}

void MainWindow::PostProcessDelete(const FileList& deletedFiles, const FileList& skippedFiles) {
    ui->treeWidget->setSortingEnabled(true);

    QString operationInfo = QString("Can't delete ").append(QString::number(skippedFiles.size())).append(" file(s):\n");
    for (const QString& fileName : skippedFiles) {
        operationInfo.append(fileName).append("\n");
    }

    ui->treeWidget->clearSelection();
    if (!skippedFiles.empty()) {
        QMessageBox::information(this, "Can't delete file(s)", operationInfo);
    }
    DeletingThread = nullptr;
}

void MainWindow::ResetWorkingThread() {
    if (WorkingThread != nullptr && !WorkingThread->isFinished()) {
        WorkingThread->quit();
        WorkingThread->wait();
    }
}

void MainWindow::ResetDeletingThread() {
    if (DeletingThread != nullptr && !DeletingThread->isFinished()) {
        DeletingThread->quit();
        DeletingThread->wait();
    }
}

