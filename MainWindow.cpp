/*
    Copyright 2013 Felix Müller.

    This file is part of VSSim.

    VSSim is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    VSSim is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with VSSim.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qRegisterMetaType<Simulator::SimulationData>( "Simulator::SimulationData" );
}

MainWindow::~MainWindow()
{
    delete ui;
    if( mSimulator )
    {
        on_Simulator_finished();
    }
}

void MainWindow::on_startSimulationButton_clicked()
{
    unsigned int incomingRate = ui->incomingRate->text().toInt();
    unsigned int serviceDuration = ui->serviceRate->text().toInt();

    if( incomingRate <= 0 || serviceDuration <= 0 )
    {
        QMessageBox *msg = new QMessageBox( this );
        msg->setText( "Invalid values entered for lambdas!" );
        msg->show();
        return;
    }

    ui->progressBar->setValue( 0 );

    if( !mSimulator )
    {
        mSimulator.reset( new Simulator( incomingRate, serviceDuration ) );
        connect( mSimulator.data(), SIGNAL( finished() ), this,  SLOT( on_Simulator_finished() ) );
        connect( mSimulator.data(), SIGNAL( updateValues(Simulator::SimulationData) ),
                 this, SLOT( on_Simulator_updateValues(Simulator::SimulationData)) );
        mSimulator->start();
    }
    else
    {
        on_Simulator_finished();
    }
}

void MainWindow::on_Simulator_finished()
{
    if( mSimulator )
    {
        mSimulator->quit();
        mSimulator->wait();
        mSimulator.reset();
    }
}

void MainWindow::on_Simulator_updateValues( Simulator::SimulationData data )
{
    ui->simTime->setText( QString::number( data.simulationTime / 1000 ) );
    ui->valueN->setText( QString::number( data.n ) );
    ui->valueT->setText( QString::number( data.t ) );
    ui->valueNQ->setText( QString::number( data.nq ) );
    ui->valueTQ->setText( QString::number( data.tq ) );
    ui->progressBar->setValue( data.variance );
    ui->checkBox->setChecked( mSimulator ? !mSimulator->isRunning() : false );
}
