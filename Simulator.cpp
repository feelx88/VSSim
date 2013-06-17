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
                mEvents.insert( Event::makeEventPair( Event::EET_INCOMING_EVENT,
                                                      nextIncomingTime ) );

                //If there is a finite number of service units, check if they are
                //busy
                if( mData.numServiceUnits > 0 && mData.curN == mData.numServiceUnits )
                {
                    //Increment queue usage
                    mData.curNQ++;
                }
                else
                {
                    //As the request can now be serviced, add its finished event
                    nextFinishedTime = mData.simulationTime
                            + mServiceDurationGenerator.generate();
                    mEvents.insert( Event::makeEventPair( Event::EET_FINISHED_EVENT,
                                                          nextFinishedTime ) );

                    //Increment service unit ussage
                    mData.curN++;
                }

                mData.curT = nextFinishedTime;

                updateStatistics();
                break;

            case Event::EET_FINISHED_EVENT:

                //If there is a finite number of service units, check if there
                //are queud requests
                if( mData.numServiceUnits > 0 && mData.curNQ > 0 )
                {
                    //Decrement queue usage
                    mData.curNQ--;

                    //As the request can now be serviced, add its finished event
                    mEvents.insert( Event::makeEventPair( Event::EET_FINISHED_EVENT,
                                                          nextFinishedTime ) );
                }
                else
                {
                    //Decrement current service unit usage
                    mData.curN--;
                }

                updateStatistics();
                break;

            case Event::EET_MEASURE_EVENT:
                //Schedule new measure event
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

        //Check if stop criteria are met
        if( mData.standardDerivationN <= mData.minimalSD
                //&& mData.standardDerivationT <= mData.minimalSD
                && mData.standardDerivationNQ <= mData.minimalSD )
                //&& mData.standardDerivationTQ <= mData.minimalSD )
        {
            mRunning = false;
        }
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

void Simulator::setPrecision( float precision )
{
    mData.minimalSD = precision;
}

void Simulator::emitUpdateSignal()
{
    emit updateValues( mData );
}

void Simulator::updateStatistics()
{
    //Update N
    mData.Nnum++;
    mData.Nsum += mData.curN;
    mData.NsumSQ += std::pow( mData.curN, 2.f );
    mData.N = (float)mData.Nsum / (float)mData.Nnum;
    mData.varianceN = ( ( (float)mData.NsumSQ / (float)mData.Nnum )
                        - std::pow( mData.N, 2.f ) ) / (float)mData.Nnum;
    mData.standardDerivationN = std::sqrt( mData.varianceN );

    //Update T
    mData.Tnum++;
    mData.Tsum += mData.curT;
    mData.TsumSQ += std::pow( mData.curT, 2 );
    mData.T = (float)mData.Tsum / (float)mData.Tnum;

    //Uodate NQ
    mData.NQnum++;
    mData.NQsum += mData.curNQ;
    mData.NQsumSQ += std::pow( mData.curNQ, 2 );
    mData.NQ = (float)mData.NQsum / (float)mData.NQnum;
    mData.varianceNQ = ( ( (float)mData.NQsumSQ / (float)mData.NQnum )
                        - std::pow( mData.NQ, 2.f ) ) / (float)mData.NQnum;
    mData.standardDerivationNQ = std::sqrt( mData.varianceNQ );
}

Simulator::SimulationData::SimulationData()
    : simulationTime( 0 ),
      nextEventTime( 0 ),
      N( 0 ),
      T( 0 ),
      NQ( 0 ),
      TQ( 0 ),
      varianceN( std::numeric_limits<float>::max() ),
      varianceT( std::numeric_limits<float>::max() ),
      varianceNQ( std::numeric_limits<float>::max() ),
      varianceTQ( std::numeric_limits<float>::max() ),
      standardDerivationN( std::numeric_limits<float>::max() ),
      standardDerivationT( std::numeric_limits<float>::max() ),
      standardDerivationNQ( std::numeric_limits<float>::max() ),
      standardDerivationTQ( std::numeric_limits<float>::max() ),
      minimalSD( 1.e-3f ),
      Nsum( 0 ), Tsum( 0 ), NQsum( 0 ), TQsum( 0 ),
      Nnum( 0 ), Tnum( 0 ), NQnum( 0 ), TQnum( 0 ),
      curN( 0 ), curT( 0 ), curNQ( 0 ), curTQ( 0 ),
      enableMeasureEvents( true ),
      measureEventDistance( 100 )
{
}
