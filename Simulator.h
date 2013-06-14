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

typedef std::multimap<long, Event> EventMap;

class Simulator : public QThread
{
    Q_OBJECT
public:
    struct SimulationData
    {
        SimulationData();
        unsigned long simulationTime, nextEventTime;
        int n, t, nq, tq;
        float variance, standardDerivation, minimalSD;
    };

    explicit Simulator( unsigned int incomingRate, unsigned int serviceDuration,
                        QObject *parent = 0 );
    void run();

    bool isRunning();
    void quit();

signals:
    void finished();
    void updateValues( Simulator::SimulationData data );

private:
    Generator mIncomingRateGenerator, mServiceDurationGenerator;
    bool mRunning;

    QTimer mTimer;

    SimulationData mData;

    EventMap mEvents;

private slots:
    void emitUpdateSignal();

};

#endif // SIMULATOR_H
