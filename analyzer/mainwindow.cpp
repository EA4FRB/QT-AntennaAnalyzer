/*
(C) Copyright 2015 Jeremy Burton

This file is part of Sark-100-antenna-analyzer.

Sark-100-antenna-analyzer is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Sark-100-antenna-analyzerr is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <locale.h>

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>

#include "scandata.h"

#include "settingsdlg.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

const Version
    MainWindow::version = Version(1,10,13,"");

ScanData scandata;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    setlinebuf(stdout);

    Config::read();

    ui->setupUi(this);
    ui->statusBar->addWidget(ui->label_Status);
    ui->statusBar->addWidget(ui->progressBar);

    setWindowTitle(Config::App);

    connect(ui->actionLoad, SIGNAL(triggered()), this, SLOT(Slot_Load()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(Slot_Save()));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(Slot_Settings()));
    connect(ui->actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->actionAbout_QT, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionAbout_Analyzer, SIGNAL(triggered()), this, SLOT(Slot_about()));

//    connect(ui->menuDevice, SIGNAL(aboutToShow()), this, SLOT(Slot_menuDevice_Show()));
//    connect(ui->menuDevice, SIGNAL(triggered(QAction *)), this, SLOT(Slot_menuDevice_Select(QAction *)));
    connect(ui->menuDevice, SIGNAL(triggered()), this, SLOT(Slot_menuDevice_Select()));

    connect(ui->canvas1, SIGNAL(cursorMoved(double)), this, SLOT(Slot_cursor_move(double)));

    connect(ui->scanBtn,SIGNAL(clicked()),this,SLOT(Slot_ScanSingle_click()));
    connect(ui->scanDummyBtn,SIGNAL(clicked()),this,SLOT(Slot_ScanCont_click()));

    connect(ui->copyBtn, SIGNAL(clicked()), this, SLOT(Slot_copy()));

    connect(ui->band_cb, SIGNAL(currentIndexChanged(int)), this, SLOT(Slot_band_change(int)));
    connect(ui->fcentre, SIGNAL(valueChanged(double)), this, SLOT(Slot_fcentre_change(double)));
    connect(ui->fspan, SIGNAL(valueChanged(double)), this, SLOT(Slot_fspan_change(double)));
    connect(ui->point_count, SIGNAL(valueChanged(int)), this, SLOT(Slot_point_count_change(int)));

    connect(ui->monStartBtn,SIGNAL(clicked()),this,SLOT(Slot_monStart_click()));
    connect(ui->monStopBtn,SIGNAL(clicked()),this,SLOT(Slot_monStop_click()));

    QCheckBox *ctrls[] = {ui->plotz_chk,ui->plotx_chk,ui->plotr_chk,NULL};
    for (int i=0; ctrls[i]; i++)
        connect(ctrls[i], SIGNAL(stateChanged(int)), this, SLOT(Slot_plot_change(int)));

    deviceIO = new DeviceIO();
    if (deviceIO->IsUp())
        ui->label_Status->setText((QString)"Connected");
    else
        ui->label_Status->setText((QString)"Disconnected");


    ui->band_cb->setCurrentIndex(14);

    ui->scan_data->setHorizontalHeaderLabels(QStringList() << "freq" << "SWR" << "Z" << "R" << "X");

    ui->canvas1->cursor = ui->cursor;

    ui->tabWidget->setCurrentIndex(1);
    //ui->tabWidget->setTabEnabled(0,false);
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(Slot_tabWidget_change(int)));

    montimer.setParent(this);
    connect(&montimer, SIGNAL(timeout()), this, SLOT(Slot_montimer_timeout()));

    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(Slot_Update()));
}

void MainWindow::Slot_tabWidget_change(int)
{
    montimer.stop();

    bContRun = false;
    timer->stop();
}

MainWindow::~MainWindow()
{
    delete timer;
    delete deviceIO;
    delete ui;
}

void MainWindow::RaiseEvent(event_t event,int arg)
{
    switch (event)
    {
      case progress_event:
        ui->progressBar->setValue(arg);
        break;
    }
}

void MainWindow::populate_table()
{
    //Populate data table
    ui->scan_data->setRowCount(scandata.points.size());

    for (unsigned int i=0;i<scandata.points.size();i++)
    {
        Sample *point = &scandata.points[i];

        ui->scan_data->setItem(i, 0, new QTableWidgetItem(QString("%1").arg(point->freq/1000000.0,0,'f')));
        ui->scan_data->setItem(i, 1, new QTableWidgetItem(QString("%1").arg(point->swr)));
        ui->scan_data->setItem(i, 2, new QTableWidgetItem(QString("%1").arg(point->Z)));
        ui->scan_data->setItem(i, 3, new QTableWidgetItem(QString("%1").arg(point->R)));
        ui->scan_data->setItem(i, 4, new QTableWidgetItem(QString("%1").arg(point->X)));
        //ui->scan_data->setItem(i, 5, new QTableWidgetItem(QString("%1").arg(point->X2)));
    }
}

void MainWindow::draw_graph1()
{
    GraphScale *scale;
    //double n;

    if (scandata.points.size()==0)
        return;
    scale = ui->canvas1->xscale;
    scale->vmin = scandata.freq_start;
    scale->vmax = scandata.freq_end;
    scale->SetIncAuto();

    scale = ui->canvas1->yscale1;
    scale->vmin = 1.0;
    scale->vmax = scandata.points[scandata.swr_max_idx].swr>Config::swr_max ? Config::swr_max : scandata.points[scandata.swr_max_idx].swr;
    scale->SetIncAuto();

    scale = ui->canvas1->yscale2;
    scale->vmin = 0.0; //scandata.points[scandata.Z_min_idx].Z;
    scale->vmax = 0.0;
    if (ui->canvas1->ztrace->enabled && scandata.points[scandata.Z_max_idx].Z>scale->vmax) scale->vmax=scandata.points[scandata.Z_max_idx].Z;
    //if (ui->canvas1->xtrace->enabled && scandata.points[scandata.X_max_idx].X>scale->vmax) scale->vmax=scandata.points[scandata.X_max_idx].X;
    if (ui->canvas1->xtrace->enabled) scale->Expand(scandata.points[scandata.X_min_idx].X,scandata.points[scandata.X_max_idx].X);
    if (ui->canvas1->rtrace->enabled && scandata.points[scandata.R_max_idx].R>scale->vmax) scale->vmax=scandata.points[scandata.R_max_idx].R;
    if (scale->vmax==0.0) scale->vmax=1.0;
    scale->SetIncAuto();
    scale->SetMinAuto();

    ui->canvas1->swrtrace->points.resize(scandata.points.size());
    for (unsigned int i=0;i<scandata.points.size();i++)
        ui->canvas1->swrtrace->points[i] = scandata.points[i].swr;

    ui->canvas1->ztrace->points.resize(scandata.points.size());
    for (unsigned int i=0;i<scandata.points.size();i++)
        ui->canvas1->ztrace->points[i] = scandata.points[i].Z;

    ui->canvas1->xtrace->points.resize(scandata.points.size());
    for (unsigned int i=0;i<scandata.points.size();i++)
        ui->canvas1->xtrace->points[i] = scandata.points[i].X;

    ui->canvas1->rtrace->points.resize(scandata.points.size());
    for (unsigned int i=0;i<scandata.points.size();i++)
        ui->canvas1->rtrace->points[i] = scandata.points[i].R;

    ui->canvas1->ZTargetline->val = Config::Z_Target;
    ui->canvas1->SWRTargetline->val = Config::swr_bw_max;

    ui->canvas1->swrminline->val = scandata.points[scandata.swr_min_idx].freq;

    ui->canvas1->update();

    //Show stats at the bottom of the graph
    ui->swr_min_disp->setText(QString("%1 (f=%2MHz, Z=%3%4, bw=%5MHz)")
            .arg(scandata.points[scandata.swr_min_idx].swr,0,'f',2)
            .arg(scandata.points[scandata.swr_min_idx].freq/1000000)
            .arg(scandata.points[scandata.swr_min_idx].Z,0,'f',2).arg(QChar(0x03A9))
            .arg((scandata.points[scandata.swr_bw_hi_idx].freq-scandata.points[scandata.swr_bw_lo_idx].freq)/1000000,0,'f',2));

}


void MainWindow::Slot_Update()
{
    if (deviceIO->IsUp())
        ScanProc();

    if (bContRun)
        timer->start(500);
    else
        timer->stop();
}

void MainWindow::Slot_ScanSingle_click() /* Single sweep */
{
    bContRun = false;
    if (bIsScanning)
        return;
    if (deviceIO->IsUp())
        timer->start(10);
    else
        timer->stop();
}

void MainWindow::Slot_ScanCont_click()  /* Continuous */
{
    if (bContRun)   /* stops */
    {
        bContRun = false;
        timer->stop();
    }
    else
    {
        if (deviceIO->IsUp())
        {
            timer->start(10);
            bContRun = true;
        }
    }
}

void MainWindow::ScanProc()
{
    if (deviceIO->IsUp())
        ui->label_Status->setText((QString)"Connected");
    else
        ui->label_Status->setText((QString)"Disconnected");

    scandata.freq_start = (ui->fcentre->value()-ui->fspan->value()/2.0)*1000000;
    scandata.freq_end = (ui->fcentre->value()+ui->fspan->value()/2.0)*1000000;
    scandata.SetPointCount(ui->point_count->value());

    if (deviceIO->IsUp())
    {
        bIsScanning = true;
        deviceIO->Cmd_Scan((long)(scandata.freq_start),
                  (long)(scandata.freq_end),
                  (long)((scandata.freq_end-scandata.freq_start)/scandata.GetPointCount()),
                  this);
        deviceIO->Cmd_Off();
        populate_table();
        draw_graph1();
        bIsScanning = false;
    }
}

void MainWindow::Slot_cursor_move(double pos)
{
//printf("pos=%lf\n",pos);
    int n = pos*(double)(scandata.points.size()-1);

    if (n<0 || n>=scandata.GetPointCount())
        return;

    Sample *sample = &scandata.points[n];

    ui->cursor_disp->setText(QString("f=%1MHz, swr=%2, Z=%3%4")
            .arg(sample->freq/1000000)
            .arg(sample->swr,0,'f',2)
            .arg(sample->Z,0,'f',2).arg(QChar(0x03A9)));
}

void MainWindow::set_band(double f, double span)
{
    ui->fcentre->setValue(f);
    ui->fspan->setValue(span);
}

void MainWindow::set_scan_disp()
{
    ui->fstart_disp->setText(QString("%1").arg(ui->fcentre->value()-ui->fspan->value()/2.0));
    ui->fend_disp->setText(QString("%1").arg(ui->fcentre->value()+ui->fspan->value()/2.0));
    ui->fstep_disp->setText(QString("%1").arg(ui->fspan->value()*1000.0/(double)ui->point_count->value()));
}

void MainWindow::Slot_band_change(int idx)
{
    ui->band_cb->blockSignals(true);
    ui->fcentre->blockSignals(true);
    ui->fspan->blockSignals(true);

    switch (idx)
    {
        case 0: break;
        case 1: set_band(1.5, 1.0);  break;     //160m
        case 2: set_band(3.5, 3.0);  break;     //80m
        case 3: set_band(6.5, 3.0);  break;     //40m
        case 4: set_band(9.5, 3.0);  break;     //30m
        case 5: set_band(12.0, 2.0);  break;    //25m
        case 6: set_band(15.0, 4.0);  break;    //20m
        case 7: set_band(18.0, 2.0);  break;    //17m
        case 8: set_band(21.0, 4.0);  break;    //15m
        case 9: set_band(24.5, 3.0);  break;    //12m
        case 10: set_band(27.0,	2.0); break;    //11m
        case 11: set_band(29.5,	3.0); break;    //10m
        case 12: set_band(40,	18.0); break;   //8m
        case 13: set_band(51,	4.0); break;    //6m
        case 14: set_band(16.5,	27.0); break;   //HF (3-30MHz)
        case 15: set_band(27.5,	5.0); break;    //12-10m (25-30MHz)
        case 16: set_band(13.5,	5.0); break;    //RFID HF
        case 17: set_band(115.5,229.0); break;  //1-230MHz
        case 18: set_band(223.5,3.0); break;    //1.25m
        case 19: set_band(435.0, 30.0); break;  //70cm
    }

    ui->band_cb->blockSignals(false);
    ui->fcentre->blockSignals(false);
    ui->fspan->blockSignals(false);
    set_scan_disp();
}

void MainWindow::Slot_fcentre_change(double v)
{
    ui->fcentre->blockSignals(true);

    double n = ui->fspan->value()/2.0;
    if ((v-n)<0.1)   { v=0.1+n; ui->fcentre->setValue(v); }
    if ((v+n)>700.0)  { v=700.0-n; ui->fcentre->setValue(v); }
    ui->band_cb->setCurrentIndex(0);

    ui->fcentre->blockSignals(false);
    set_scan_disp();
}

void MainWindow::Slot_fspan_change(double v)
{
    ui->fspan->blockSignals(true);

    double centre = ui->fcentre->value();
    if ((centre-v/2.0)<0.1)  { v=2.0*(centre-1);    ui->fspan->setValue(v); }
    if ((centre+v/2.0)>700.0) { v=2.0*(700.0-centre); ui->fspan->setValue(v); }
    ui->band_cb->setCurrentIndex(0);

    ui->fspan->blockSignals(false);
    set_scan_disp();
}

void MainWindow::Slot_point_count_change(int)
{
    set_scan_disp();
}

void MainWindow::Slot_plot_change(int)
{
    ui->canvas1->ztrace->enabled = ui->plotz_chk->checkState()==Qt::Checked;
    ui->canvas1->xtrace->enabled = ui->plotx_chk->checkState()==Qt::Checked;
    ui->canvas1->rtrace->enabled = ui->plotr_chk->checkState()==Qt::Checked;
    draw_graph1();
//    ui->canvas1->update();
}

void MainWindow::Slot_Load()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Open Layout",Config::dir_data,"Scan Data (*.analyzer)");
    if (!fileName.isEmpty())
    {
        QDomDocument doc( "AnalyzerML" );
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this, tr("Analyzer"),
                                  tr("Cannot read file %1:\n%2.")
                                  .arg(fileName)
                                  .arg(file.errorString()));
            return;
        }
        if (!doc.setContent(&file))
        {
            file.close();
            return;
        }
        file.close();

        QDomElement root = doc.documentElement();
        if (root.tagName() != "analyzer")
        {
          QMessageBox::warning(this, "Analyzer",
                                    "Cannot load file\nThis is not a valid analyzer file.");
          return;
        }

        fromDom(root);

        //setCurrentFile(fileName);

        //fileName.replace('\\','/');
        //addRecentFile(fileName);
        QFileInfo fi(fileName);
        if (Config::dir_data != fi.dir().path())
        {
          Config::dir_data = fi.dir().path();
          Config::write();
        }

        populate_table();
        draw_graph1();

        ui->fcentre->setValue((scandata.freq_end+scandata.freq_start)/2000000.0);
        ui->fspan->setValue((scandata.freq_end-scandata.freq_start)/1000000.0);

        statusBar()->showMessage(tr("Analyzer data loaded"), 5000);
    }
}

void MainWindow::fromDom(QDomElement &e0)
{
    //freq_start = e0.attribute("fstart", "0").toDouble();

    for (QDomNode n1 = e0.firstChild(); !n1.isNull(); n1 = n1.nextSibling())
    {
        if (!n1.isElement()) continue;  // Skip any non-element nodes

        QDomElement e1 = n1.toElement();

        if (e1.tagName() == "scandata")
            scandata.fromDom(e1);
        else if (e1.tagName() == "notes")
            ui->notes_txt->setPlainText(e1.text());
    }
}

void MainWindow::toDom(QDomDocument &doc)
{
  QDomElement element = doc.createElement("analyzer");

  toDom_Text(doc,element,"notes",ui->notes_txt->toPlainText());

  scandata.toDom(doc,element);

  doc.appendChild(element);
}

void MainWindow::Slot_Save()
{
    QString filename = QFileDialog::getSaveFileName(this,"Save Scan Data As",Config::dir_data,"Scan Data (*.analyzer)");
    if (filename.isEmpty())
      return;

//    if (!copy)
//      setCurrentFile(fileName);	// Set filename & layoutname here because layoutname is written to the file.

    QDomDocument doc( "AnalyzerML" );

    toDom(doc);

    QFile file(filename);
    if( !file.open( QIODevice::WriteOnly ) )
      return;

    QTextStream ts( &file );
    ts.setCodec(Config::DOM_ENCODING);  //If we always save as UTF-8 unicode filenames should work. Othewise on windows it saves in some other encoding.
    ts << doc.toString();

    file.close();
    //statusBar()->showMessage(tr("Data saved"), 2000);

    QFileInfo fi(filename);
    if (Config::dir_data != fi.dir().path())
    {
      Config::dir_data = fi.dir().path();
      Config::write();
    }
}

void MainWindow::Slot_menuDevice_Show()
{
}

void MainWindow::Slot_menuDevice_Select()
{
  if (deviceIO)
      delete deviceIO;

  deviceIO = new DeviceIO();
}

void MainWindow::Slot_about()
{
      QMessageBox::about(this, "About Antenna Analyzer",
            QString("Antenna Analyzer - scanning &amp; graph plotting program for the SARK110 Antenna Analyzer.<br><br>"
               "Version %1.%2.%3 %4<br><br>"
               "&copy; 2015 Jeremy Burton - Ported to the SARK110 by EA4FRB 2018").arg(version.major).arg(version.minor).arg(version.build).arg(version.subversion));
}

void MainWindow::Slot_Settings()
{
    SettingsDlg dlg;

    if (dlg.exec() == QDialog::Accepted)
    {
      scandata.UpdateStats();
      draw_graph1();
    }
}

void MainWindow::Slot_copy()
{
  QString txt("freq\tSWR\tZ\tR\tX\n");

  for (unsigned int i=0;i<scandata.points.size();i++)
     txt += QString("%1\t%2\t%3\t%4\t%5\n")
             .arg(scandata.points[i].freq/1000000.0,0,'f')
             .arg(scandata.points[i].swr)
             .arg(scandata.points[i].Z)
             .arg(scandata.points[i].R)
             .arg(scandata.points[i].X);

  qApp->clipboard()->setText(txt);
}

void MainWindow::Slot_monStart_click()
{
    bContRun = false;
    timer->stop();

    ui->SWR_Bar->vmin = ui->SWR_Bar->vorig = 1;
    ui->SWR_Bar->vmax = 10;
    ui->SWR_Bar->SetIncAuto();
    ui->SWR_Bar->value = 1;

    ui->Z_Bar->brush = QBrush(qRgb(255,85,0));
    ui->Z_Bar->vmin = 0;
    ui->Z_Bar->vmax = 200;
    ui->Z_Bar->value = 50;
    ui->Z_Bar->SetIncAuto();

    ui->R_Bar->brush = QBrush(Qt::darkGreen);
    ui->R_Bar->vmin = 0;
    ui->R_Bar->vmax = 200;
    ui->R_Bar->value = 50;
    ui->R_Bar->SetIncAuto();

    ui->X_Bar->brush = QBrush(Qt::red);
    ui->X_Bar->vmin = -100;
    ui->X_Bar->vmax = 200;
    ui->X_Bar->value = 0;
    ui->X_Bar->SetIncAuto();

    if (deviceIO->IsUp())
    {
        Sample sample;
        deviceIO->Cmd_Single((long)(ui->monfreq->value()*1000000), sample);
    }

    Slot_montimer_timeout();
    montimer.start((long)(ui->monrate->value()));
}

void MainWindow::Slot_monStop_click()
{
    montimer.stop();

    if (deviceIO->IsUp())
        deviceIO->Cmd_Off();
}

void MainWindow::Slot_montimer_timeout()
{
    Sample sample;

    if (deviceIO->IsUp())
    {
        deviceIO->Cmd_Single((long)(ui->monfreq->value()*1000000), sample);

        ui->SWR_lbl->setText(QString("%1:1").arg(sample.swr, 0,'f',1));
        ui->SWR_Bar->value = sample.swr;
        ui->SWR_Bar->update();

        ui->Z_lbl->setText(QString("%1").arg(sample.Z, 0,'f',1));
        ui->Z_Bar->value = sample.Z;
        ui->Z_Bar->update();

        ui->R_lbl->setText(QString("%1").arg(sample.R, 0,'f',1));
        ui->R_Bar->value = sample.R;
        ui->R_Bar->update();

        ui->X_lbl->setText(QString("%1").arg(sample.X, 0,'f',1));
        ui->X_Bar->value = sample.X;
        ui->X_Bar->update();
    }
}
