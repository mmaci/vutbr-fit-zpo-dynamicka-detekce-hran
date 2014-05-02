#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
   // QString filename = QFileDialog::getOpenFileName(this, tr("Open File ..."));

   //QPixmap pix = ui->imgVstup->pixmap();

    vystupImage = vstupImage;//kvuli alokaci rozmeru - asi jde jinak
    for(int x=0; x < vstupImage.width(); x++)
    {
        for(int y=0; y < vstupImage.height(); y++)
        {
            QColor pixlik(vstupImage.pixel(x,y));
            //tady nejaka funkce
            //.........
            int r,g,b;
            r = 255 - pixlik.red();
            g = 255 - pixlik.green();
            b = 255 - pixlik.blue();
            //.........
            QColor vystup(r,g,b);
            vystupImage.setPixel(x,y,vystup.rgb());
        }

    }
    QPixmap pix = QPixmap::fromImage(vystupImage);
    ui->imgVystup->setPixmap(pix.scaled(ui->imgVystup->width(),ui->imgVystup->height(),Qt::KeepAspectRatio));

}

void MainWindow::on_actionUlo_it_triggered()
{
  QString filename = QFileDialog::getSaveFileName(this, tr("Save File ..."),"","*.jpg");
  vystupImage.save(filename,"JPEG");

}

void MainWindow::on_actionOtev_t_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File ..."),"","*.jpg");

    vstupImage.load(filename);
//p.scaled(w,h,Qt::KeepAspectRatio

    QPixmap pix = QPixmap::fromImage(vstupImage);
    ui->imgVstup->setPixmap(pix.scaled(ui->imgVstup->width(),ui->imgVstup->height(),Qt::KeepAspectRatio));
}

void MainWindow::on_actionKonec_triggered()
{
    close();
}
