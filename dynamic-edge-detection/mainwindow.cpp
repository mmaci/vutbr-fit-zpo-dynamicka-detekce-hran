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

void MainWindow::ChangeColor()
{
    uint8_t R,G,B;
    R = ui->hsR->value();
    G =ui->hsG->value();
    B = ui->hsB->value();

    edgEcolor= qRgb(R,G,B);
    QPalette pal =  ui->lblColor->palette();
    pal.setColor(QPalette::Background, QColor(R,G,B));
    ui->lblColor->setPalette(pal);
}

void MainWindow::on_pushButton_clicked()
{
   // QString filename = QFileDialog::getOpenFileName(this, tr("Open File ..."));

   //QPixmap pix = ui->imgVstup->pixmap();


    outImage = inImage;//kvuli alokaci rozmeru - asi jde jinak

    if(static_cast<PixelType>(ui->cbCena->currentIndex())== ORIGIN){
        DynamicEdgeDetector det(&outImage, inImage.width(), inImage.height());

        det.setType(RGB);


        det.setMethod(static_cast<DetectionMethod>(ui->cbSmer->currentIndex()));
        //det.setThreshold(ui->hsJemnost->value());
        ChangeColor();
        det.setColor(edgEcolor);

        det.calc();
        det.backwardTrack();

    }else
    {
        DynamicEdgeDetector2 det(&outImage, inImage.width(), inImage.height());

        det.setType(static_cast<PixelType>(ui->cbCena->currentIndex()));


        det.setMethod(static_cast<DetectionMethod>(ui->cbSmer->currentIndex()));
        det.setThreshold(ui->hsJemnost->value());
        ChangeColor();
        det.setColor(edgEcolor);

        det.calc();
        det.backwardTrack();
    }

    //det.backwardTrackEdge(inImage.width()/2, inImage.height()/2);



    /*

    for (uint8_t x = 0; x < inImage.width(); x++)
    {
        for(uint8_t y = 0; y < inImage.height(); y++)
        {
            QColor pixlik(inImage.pixel(x,y));
            //tady nejaka funkce
            //.........
            int r,g,b;
            r = 255 - pixlik.red();
            g = 255 - pixlik.green();
            b = 255 - pixlik.blue();
            //.........
            QColor vystup(r,g,b);
            outImage.setPixel(x,y,vystup.rgb());
        }
    }*/
    QPixmap pix = QPixmap::fromImage(outImage);
    ui->imgVystup->setPixmap(pix.scaled(ui->imgVystup->width(),ui->imgVystup->height(),Qt::KeepAspectRatio));
    ui->actionUlozit->setEnabled(true);
}

void MainWindow::on_actionOtev_t_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File ..."),"","*.jpg");

    inImage.load(filename);
//p.scaled(w,h,Qt::KeepAspectRatio

    QPixmap pix = QPixmap::fromImage(inImage);
    ui->imgVstup->setPixmap(pix.scaled(ui->imgVstup->width(),ui->imgVstup->height(),Qt::KeepAspectRatio));
    ui->gbNastaveni->setEnabled(true);

}

void MainWindow::on_actionKonec_triggered()
{
    close();
}

void MainWindow::on_actionUlozit_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save File ..."),"","*.jpg");
    outImage.save(filename,"JPEG");

}

void MainWindow::on_hsR_valueChanged(int value)
{
    ui->lblR->setText("R ("+  QString("%1").arg(value) +  ")" );
    ChangeColor();
}

void MainWindow::on_hsG_valueChanged(int value)
{
    ui->lblG->setText("G (" +  QString("%1").arg(value) + ")" );
    ChangeColor();
}

void MainWindow::on_hsB_valueChanged(int value)
{
    ui->lblB->setText(QString("B (" +  QString("%1").arg(value) + ")") );
    ChangeColor();
}

void MainWindow::on_imgVystup_customContextMenuRequested(const QPoint &pos)
{

}
