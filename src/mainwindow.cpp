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

    ui->progressBar->reset();
    ui->progressBar->hide();
    ui->treeWidget->hide();
    ui->actionStopScanning->setVisible(false);
    ui->actionDelete->setVisible(false);

    connect(ui->actionScanDirectory, &QAction::triggered, this, &MainWindow::SelectDirectory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::ShowAbout);
    connect(ui->actionStopScanning, &QAction::triggered, this, &MainWindow::Stop);
    //connect(ui->actionDelete, SIGNAL(released()), SLOT(delete_items()));
}

MainWindow::~MainWindow() {
}

void MainWindow::Stop() {
    NeedStop.store(true);
}

void MainWindow::ShowAbout() {
    QMessageBox::aboutQt(this);
}

void MainWindow::SelectDirectory() {
    QString directory = QFileDialog::getExistingDirectory(
            this,
            "Select Directory for Scanning",
            QString(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    QFileInfo directoryIifo(directory);
    if (!directoryIifo.exists()) {
        return;
    }

    //ui->actionScanDirectory->setVisible(false);
    ui->actionStopScanning->setVisible(true);
    ui->actionDelete->setVisible(true);
    ui->treeWidget->setVisible(true);

    QThread* thread = new QThread;
    Worker* worker = new Worker(directory, this, &NeedStop);
    worker->moveToThread(thread);

    connect(thread, SIGNAL(started()), worker, SLOT(Process()));
    connect(worker, SIGNAL(Finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(Finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT (deleteLater()));
    connect(worker, SIGNAL(SetupFilesNumber(int)), this, SLOT(SetupProgressBar(int)));

    thread->start();
}

void MainWindow::AddDuplicatesList(const FileList &duplicates) {
    UpdateProgressBar(duplicates.size());

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, QString("Found " + QString::number(duplicates.size())) + " duplicates");
    QFileInfo fileInfo(duplicates[0]);
    item->setText(1, QString::number(fileInfo.size()));

    for (const QString& file : duplicates) {
        QTreeWidgetItem* child_item = new QTreeWidgetItem();
        child_item->setText(0, file);
        item->addChild(child_item);
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

