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
#include <iostream>

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

    //Calculate theoretical results
    if( numServiceUnits == 1
            && mSimulator )
    {

        float A = 1.f / (float)incomingDistance;
        float B = 1.f / (float)serviceDistance;

        float N = A / ( B - A );
        float T = 1.f / ( B - A );
        float NQ = ( A / ( B - A ) ) - ( A / B );
        float TQ = ( 1.f / ( B - A ) ) - ( 1.f / B );

        std::stringstream str;
        str << "Theoretical results: <br>";
        str << "N = " << N << "<br>";
        str << "T = " << T << "<br>";
        str << "N<sub>Q</sub> = " << NQ << "<br>";
        str << "T<sub>Q</sub> = " << TQ;

        QMessageBox *msg = new QMessageBox( this );
        msg->setModal( false );
        msg->setText( str.str().c_str() );
        msg->show();
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

    ui->nCheck->setChecked( true );
    ui->tCheck->setChecked( true );
    ui->nqCheck->setChecked( true );
    ui->tqCheck->setChecked( true );
}

void MainWindow::on_Simulator_updateValues( const Simulator::SimulationData &data )
{
    ui->simTime->setText( QString::number( data.simulationTime ) );

    ui->valueN->setText( QString::number( data.N.value ) );
    ui->valueT->setText( QString::number( data.T.value ) );
    ui->valueNQ->setText( QString::number( data.NQ.value ) );
    ui->valueTQ->setText( QString::number( data.TQ.value ) );

    float f = 1.f / data.minimalSD;
    ui->standardDerivationN->setValue(
                std::max( 0.f, 101.f - data.N.standardDerivation * f ) );
    ui->standardDerivationT->setValue(
                std::max( 0.f, 101.f - data.T.standardDerivation * f ) );
    ui->standardDerivationNQ->setValue(
                std::max( 0.f, 101.f - data.NQ.standardDerivation * f ) );
    ui->standardDerivationTQ->setValue(
                std::max( 0.f, 101.f - data.TQ.standardDerivation * f ) );
    ui->standardDerivationN->setFormat(
                QString::number( data.N.standardDerivation ) );
    ui->standardDerivationT->setFormat(
                QString::number( data.T.standardDerivation ) );
    ui->standardDerivationNQ->setFormat(
                QString::number( data.NQ.standardDerivation ) );
    ui->standardDerivationTQ->setFormat(
                QString::number( data.TQ.standardDerivation ) );

    ui->nCheck->setChecked( data.N.standardDerivation < data.minimalSD );
    ui->tCheck->setChecked( data.T.standardDerivation < data.minimalSD );
    ui->nqCheck->setChecked( data.NQ.standardDerivation < data.minimalSD );
    ui->tqCheck->setChecked( data.TQ.standardDerivation < data.minimalSD );

    ui->checkBox->setChecked( mSimulator ? !mSimulator->isRunning() : false );
}
