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

#include "Simulator.h"
#include <limits>
#include <math.h>

Simulator::Simulator( unsigned int incomingRate, unsigned int serviceDuration,
                      QObject *parent )
    : QThread( parent ),
      mRunning( true ),
      mFirstRun( true )
{
    mIncomingRateGenerator.setValue( incomingRate );
    mServiceDurationGenerator.setValue( serviceDuration );

    mTimer.setInterval( 100 );
    connect( &mTimer, SIGNAL( timeout() ), this, SLOT( emitUpdateSignal() ) );
    mTimer.start( 100 );
}

void Simulator::run()
{
    unsigned long nextIncomingTime, nextFinishedTime;

    //Initialize simulation: generate EET_INCOMING event
    nextIncomingTime = mIncomingRateGenerator.generate();
    mEvents.insert( std::make_pair( nextIncomingTime,
                                    Event( Event::EET_INCOMING_EVENT, nextIncomingTime ) ) );
    mEvents.insert( std::make_pair( 100, Event( Event::EET_MEASURE_EVENT, 100 ) ) );
    mData.nextEventTime = nextIncomingTime;

    while( mRunning )
    {
        mData.simulationTime = mData.nextEventTime;

        for( auto it : mEvents )
        {
            if( it.first != mData.simulationTime )
            {
                break;
            }

            switch( it.second.getType() )
            {
            case Event::EET_INCOMING_EVENT:
                nextIncomingTime = mData.simulationTime + mIncomingRateGenerator.generate();
                nextFinishedTime = mData.simulationTime + mServiceDurationGenerator.generate();
                mEvents.insert( std::make_pair( nextIncomingTime,
                                                Event( Event::EET_INCOMING_EVENT, nextIncomingTime ) ) );
                mEvents.insert( std::make_pair( nextFinishedTime,
                                                Event( Event::EET_FINISHED_EVENT, nextFinishedTime ) ) );

                mData.curn++;
                break;

            case Event::EET_FINISHED_EVENT:
                mData.curn--;
                break;

            case Event::EET_MEASURE_EVENT:
                mData.nnum++;
                mData.nsum += mData.curn;
                mData.n = (float)mData.nsum / (float)mData.nnum;

                mEvents.insert( std::make_pair( mData.simulationTime + 100,
                                                Event( Event::EET_MEASURE_EVENT, mData.simulationTime + 100 ) ) );
                break;

            default:
                break;
            }
        }

        mEvents.erase( mData.simulationTime );

        mData.nextEventTime = mEvents.begin()->first;
        mData.t = mData.curn;
    }

    mTimer.stop();
    emitUpdateSignal();
    emit finished();
}

bool Simulator::isRunning()
{
    return mRunning;
}

void Simulator::quit()
{
    mRunning = false;
}

void Simulator::emitUpdateSignal()
{
    emit updateValues( mData );
}

Simulator::SimulationData::SimulationData()
    : simulationTime( 0 ),
      nextEventTime( 0 ),
      n( 0 ),
      t( 0 ),
      nq( 0 ),
      tq( 0 ),
      variance( std::numeric_limits<float>::max() ),
      standardDerivation( std::numeric_limits<float>::max() ),
      minimalSD( 0.000000001f ),
      nsum( 0 ), tsum( 0 ), nqsum( 0 ), tqsum( 0 ),
      nnum( 0 ), tnum( 0 ), nqnum( 0 ), tqnum( 0 ),
      curn( 0 ), curt( 0 ), curnq( 0 ), curtq( 0 )
{
}
