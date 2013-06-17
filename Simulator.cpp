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
                      unsigned int serviceUnits, QObject *parent )
    : QThread( parent ),
      mRunning( true ),
      mFirstRun( true )
{
    mIncomingRateGenerator.setValue( incomingRate );
    mServiceDurationGenerator.setValue( serviceDuration );

    mData.numServiceUnits = serviceUnits;

    mTimer.setInterval( 100 );
    connect( &mTimer, SIGNAL( timeout() ), this, SLOT( emitUpdateSignal() ) );
    mTimer.start( 100 );
}

void Simulator::run()
{
    unsigned long nextIncomingTime, nextFinishedTime;

    //Initialize simulation: generate EET_INCOMING event and first EET_MEASURE event
    nextIncomingTime = mIncomingRateGenerator.generate();
    mEvents.insert( Event::makeEventPair( Event::EET_INCOMING_EVENT,
                                          nextIncomingTime ) );

    if( mData.enableMeasureEvents )
    {
        mEvents.insert( Event::makeEventPair( Event::EET_MEASURE_EVENT,
                                              mData.measureEventDistance ) );
    }

    while( mRunning )
    {
        //Get next Event's time
        mData.simulationTime = mEvents.begin()->first;

        for( auto it : mEvents )
        {
            //Handle only events at current timestamp
            if( it.first != mData.simulationTime )
            {
                break;
            }

            switch( it.second.getType() )
            {
            case Event::EET_INCOMING_EVENT:
                //Generate new incoming event and duration event for current
                //incoming event
                nextIncomingTime = mData.simulationTime
                        + mIncomingRateGenerator.generate();
                nextFinishedTime = mData.simulationTime
                        + mServiceDurationGenerator.generate();
                mEvents.insert( Event::makeEventPair( Event::EET_INCOMING_EVENT,
                                                      nextIncomingTime ) );
                mEvents.insert( Event::makeEventPair( Event::EET_FINISHED_EVENT,
                                                      nextFinishedTime ) );

                //Increment current service unit usage
                mData.curn++;

                updateStatistics();
                break;

            case Event::EET_FINISHED_EVENT:
                //decrement current service unit usage
                mData.curn--;

                updateStatistics();
                break;

            case Event::EET_MEASURE_EVENT:
                //Schedule new event
                mEvents.insert( Event::makeEventPair(
                                    Event::EET_MEASURE_EVENT,
                                    mData.simulationTime
                                    + mData.measureEventDistance ) );

                updateStatistics();
                break;

            default:
                break;
            }
        }

        mEvents.erase( mData.simulationTime );
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

void Simulator::configureMeasureEvents( bool enabled, unsigned int distance )
{
    mData.enableMeasureEvents = enabled;
    mData.measureEventDistance = distance;
}

void Simulator::emitUpdateSignal()
{
    emit updateValues( mData );
}

void Simulator::updateStatistics()
{
    //Update n
    mData.nnum++;
    mData.nsum += mData.curn;
    mData.n = (float)mData.nsum / (float)mData.nnum;
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
      curn( 0 ), curt( 0 ), curnq( 0 ), curtq( 0 ),
      enableMeasureEvents( true ),
      measureEventDistance( 100 )
{
}
