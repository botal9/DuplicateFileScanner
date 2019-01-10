#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "TreeWidgetItem.h"

#include "library/Worker.h"
#include "library/util.h"

#include <memory>

#include <QApplication>
#include <QCommonStyle>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QThread>
#include <QTime>

#include <QtWidgets/QStyle>
#include <QtWidgets/QAction>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void ShowAbout();
    void SelectDirectory();
    void Stop();
    void PostProcessInterface(bool success);
    void SetupInterface();
    void ExpandAllRows();
    void CollapseAllRows();
    void Delete();
    void ResetThread();

public slots:
    void AddDuplicatesList(const FileList &duplicates);
    void UpdateProgressBar(int filesNumber);
    void SetupProgressBar(int filesNumber);
    void PostProcessFinish();
    void PostProcessAbort();

private:
    std::unique_ptr<Ui::MainWindow> ui;
    std::atomic_bool NeedStop = false;
    QThread* WorkingThread = nullptr;
    QTime Time;

    QDir SelectedDirectory;
    QString BeautySelectedDirectory;
};


#endif // MAINWINDOW_H
