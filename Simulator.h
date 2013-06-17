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

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QThread>
#include <QTimer>
#include <map>
#include "Generator.h"
#include "Event.h"

typedef std::multimap<unsigned long, Event> EventMap;

class Simulator : public QThread
{
    Q_OBJECT
public:
    struct SimulationData
    {
        SimulationData();
        unsigned long simulationTime, nextEventTime;
        int numServiceUnits;
        float N, T, NQ, TQ;
        float varianceN, varianceT, varianceNQ, varianceTQ;
        float standardDerivationN, standardDerivationT;
        float standardDerivationNQ, standardDerivationTQ;
        float minimalSD;
        unsigned long Nsum, Tsum, NQsum, TQsum;
        unsigned long NsumSQ, TsumSQ, NQsumSQ, TQsumSQ;
        unsigned long Nnum, Tnum, NQnum, TQnum;
        int curN, curT, curNQ, curTQ;
        bool enableMeasureEvents;
        unsigned int measureEventDistance;
    };

    explicit Simulator( unsigned int incomingRate, unsigned int serviceDuration,
                        unsigned int serviceUnits, QObject *parent = 0 );
    void run();

    bool isRunning();
    void quit();
    void configureMeasureEvents( bool enabled, unsigned int distance );
    void setPrecision( float precision );

signals:
    void finished();
    void updateValues( const Simulator::SimulationData &data );

private:
    void updateStatistics();

    Generator mIncomingRateGenerator, mServiceDurationGenerator;
    bool mRunning, mFirstRun;

    QTimer mTimer;

    SimulationData mData;

    EventMap mEvents;

private slots:
    void emitUpdateSignal();

};

#endif // SIMULATOR_H
