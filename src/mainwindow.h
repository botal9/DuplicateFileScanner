#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "src/library/Worker.h"

#include <memory>

#include <QApplication>
#include <QCommonStyle>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QThread>

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

public slots:
    void AddDuplicatesList(const FileList &duplicates);
    void UpdateProgressBar(int filesNumber);
    void SetupProgressBar(int filesNumber);

signals:

private:
    std::unique_ptr<Ui::MainWindow> ui;
    std::atomic_bool NeedStop = false;
};


#endif // MAINWINDOW_H
