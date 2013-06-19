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

    mTimer.setInterval( 100 );
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
    unsigned int numServiceUnits = ui->serviceUnitsCount->value();
    unsigned int incomingDistance = ui->incomingRate->text().toInt();
    unsigned int serviceDistance = ui->serviceRate->text().toInt();

    bool enableMeasureEvents = ui->enableMeasureEvents->isChecked();
    unsigned int measureEventDistance = ui->measureEventDistance->text().toInt();

    float precision = std::pow( 10.f, -( ui->precision->text().toInt() ) );

    if( incomingDistance <= 0 || serviceDistance <= 0 )
    {
        QMessageBox *msg = new QMessageBox( this );
        msg->setText( tr( "Invalid values entered!" ) );
        msg->show();
        return;
    }

    ui->startSimulationButton->setText( tr( "Stop Simulation" ) );

    if( !mSimulator )
    {
        mSimulator.reset( new Simulator( incomingDistance, serviceDistance, numServiceUnits, this ) );
        connect( mSimulator.data(), SIGNAL( finished() ), this,  SLOT( on_Simulator_finished() ) );
        connect( mSimulator.data(), SIGNAL( updateValues(Simulator::SimulationData) ),
                 this, SLOT( on_Simulator_updateValues(Simulator::SimulationData) ) );
        connect( &mTimer, SIGNAL( timeout() ), mSimulator.data(), SLOT( emitUpdateSignal() ) );
        mSimulator->configureMeasureEvents( enableMeasureEvents, measureEventDistance );
        mSimulator->setPrecision( precision );
        mSimulator->start();
        mTimer.start();
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
    ui->startSimulationButton->setText( tr( "Start Simulation" ) );

    ui->standardDerivationN->setValue( 100 );
    ui->standardDerivationT->setValue( 100 );
    ui->standardDerivationNQ->setValue( 100 );
    ui->standardDerivationTQ->setValue( 100 );
}

void MainWindow::on_Simulator_updateValues( const Simulator::SimulationData &data )
{
    ui->simTime->setText( QString::number( data.simulationTime ) );
    ui->valueN->setText( QString::number( data.N ) );
    ui->valueT->setText( QString::number( data.T ) );
    ui->valueNQ->setText( QString::number( data.NQ ) );
    ui->valueTQ->setText( QString::number( data.TQ ) );
    float f = 1.f / data.minimalSD;
    ui->standardDerivationN->setValue(
                std::max( 0.f, 101.f - data.standardDerivationN * f ) );
    ui->standardDerivationT->setValue(
                std::max( 0.f, 101.f - data.standardDerivationT * f ) );
    ui->standardDerivationNQ->setValue(
                std::max( 0.f, 101.f - data.standardDerivationNQ * f ) );
    ui->standardDerivationTQ->setValue(
                std::max( 0.f, 101.f - data.standardDerivationTQ * f ) );
    ui->standardDerivationN->setFormat(
                QString::number( data.standardDerivationN ) );
    ui->standardDerivationT->setFormat(
                QString::number( data.standardDerivationT ) );
    ui->standardDerivationNQ->setFormat(
                QString::number( data.standardDerivationNQ ) );
    ui->standardDerivationTQ->setFormat(
                QString::number( data.standardDerivationTQ ) );
    ui->checkBox->setChecked( mSimulator ? !mSimulator->isRunning() : false );
}
