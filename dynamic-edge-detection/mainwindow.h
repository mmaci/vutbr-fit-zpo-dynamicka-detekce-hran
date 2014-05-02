#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    public:    
    explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();


private slots:

    void on_pushButton_clicked();

    void on_actionUlo_it_triggered();

    void on_actionOtev_t_triggered();

    void on_actionKonec_triggered();

private:
        Ui::MainWindow *ui;
        QImage vstupImage;
        QImage vystupImage;
};

#endif // MAINWINDOW_H
