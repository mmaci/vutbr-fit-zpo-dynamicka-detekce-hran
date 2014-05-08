#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <cstdint>

#include "constants.h"
#include "dynamicedgedetector.h"
#include "dynamicedgedetector2.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    public:    
        explicit MainWindow(QWidget *parent = 0);
            ~MainWindow();
        void ChangeColor();

    private slots:

        void on_pushButton_clicked();
        void on_actionOtev_t_triggered();
        void on_actionKonec_triggered();
        void on_actionUlozit_triggered();

        void on_hsR_valueChanged(int value);

        void on_hsG_valueChanged(int value);

        void on_hsB_valueChanged(int value);

        void on_imgVystup_customContextMenuRequested(const QPoint &pos);

private:
        Ui::MainWindow *ui;
        QImage inImage;
        QImage outImage;
        QRgb edgEcolor;
};

#endif // MAINWINDOW_H
