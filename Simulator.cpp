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
      mFirstRun( true ),
      mDeleteEventAtZero( false )
{
    mIncomingRateGenerator.setValue( incomingRate );
    mServiceDurationGenerator.setValue( serviceDuration );

    mData.numServiceUnits = serviceUnits;
}

Simulator::~Simulator()
{
}

void Simulator::run()
{
    size_t nextIncomingTime, nextFinishedTime;

    //Initialize simulation: generate EET_INCOMING event and first EET_MEASURE event
    nextIncomingTime = mIncomingRateGenerator.generate();
    mEvents.insert( Event::makeEventPair( Event::EET_INCOMING_EVENT,
                                          nextIncomingTime, 0 ) );

    if( mData.enableMeasureEvents )
    {
        mEvents.insert( Event::makeEventPair( Event::EET_MEASURE_EVENT,
                                              mData.measureEventDistance, 0 ) );
    }

    while( mRunning )
    {
        //Get next Event's time
        auto begin = mEvents.begin();
        while( begin->first == 0 )
        {
            ++begin;
        }
        mData.simulationTime = begin->first;

        //Iterate over all events (they are sorted beacuse of std::multimap)

        for( auto pair : mEvents )
        {
            //Handle only events at current timestamp
            if( pair.first == 0 ) //START_SERVICE events
            {
                continue;
            }
            else if( pair.first != mData.simulationTime ) //later events
            {
                break;
            }

            switch( pair.second.getType() )
            {
            case Event::EET_INCOMING_EVENT:
            {
                //Generate new incoming event and duration event for current
                //incoming event
                nextIncomingTime = mData.simulationTime
                        + mIncomingRateGenerator.generate();
                mEvents.insert( Event::makeEventPair( Event::EET_INCOMING_EVENT,
                                                      nextIncomingTime,
                                                      mData.simulationTime ) );

                //If there is a finite number of service units, check if they are
                //busy
                if( mData.numServiceUnits > 0 && mData.curN == mData.numServiceUnits )
                {
                    //Increment queue usage
                    mData.curNQ++;

                    //Uodate NQ
                    mData.NQnum++;
                    mData.NQsum += mData.curNQ;
                    mData.NQsumSQ += mData.curNQ * mData.curNQ;
                    mData.NQ = (float)mData.NQsum / (float)mData.NQnum;
                    mData.varianceNQ = ( ( (float)mData.NQsumSQ / (float)mData.NQnum )
                                        - mData.NQ * mData.NQ ) / (float)mData.NQnum;
                    mData.standardDerivationNQ = std::sqrt( mData.varianceNQ );

                    //Create START_SERVICE event to save the creation time
                    //Use start time of 0 to indicate unknown starting time
                    mEvents.insert( Event::makeEventPair( Event::EET_START_SERVICE_EVENT,
                                                          0, mData.simulationTime ) );
                }
                else
                {
                    //Increment service unit ussage
                    mData.curN++;

                    //Update N
                    mData.Nnum++;
                    mData.Nsum += mData.curN;
                    mData.NsumSQ += mData.curN * mData.curN;
                    mData.N = (float)mData.Nsum / (float)mData.Nnum;
                    mData.varianceN = ( ( (float)mData.NsumSQ / (float)mData.Nnum )
                                        - mData.N * mData.N ) / (float)mData.Nnum;
                    mData.standardDerivationN = std::sqrt( mData.varianceN );

                    //As the request can be directly serviced, add its finished event
                    nextFinishedTime = mData.simulationTime
                            + mServiceDurationGenerator.generate();
                    mEvents.insert( Event::makeEventPair( Event::EET_FINISHED_EVENT,
                                                          nextFinishedTime,
                                                          mData.simulationTime ) );
                }

                //Update quantity statistics
                //updateQuantityStatistics();

                break;
            }

            case Event::EET_FINISHED_EVENT:
            {
                //Decrement current service unit usage
                mData.curN--;

                mData.curT = mData.simulationTime - pair.second.getCreationTime();

                //Update T
                mData.Tnum++;
                mData.Tsum += mData.curT;
                mData.TsumSQ += mData.curT * mData.curT;
                mData.T = (float)mData.Tsum / (float)mData.Tnum;
                mData.varianceT = ( ( (float)mData.TsumSQ / (float)mData.Tnum )
                                    - mData.T * mData.T ) / (float)mData.Tnum;
                mData.standardDerivationT = std::sqrt( mData.varianceT );

                //Check for queued request created at time 0
                auto it2 = mEvents.find( 0 );
                if( it2 != mEvents.end() && it2->second.getType() == Event::EET_START_SERVICE_EVENT )
                {
                    //Decrement queue usage
                    mData.curNQ--;
                    mData.curN++;

                    //Uodate NQ
                    mData.NQnum++;
                    mData.NQsum += mData.curNQ;
                    mData.NQsumSQ += mData.curNQ * mData.curNQ;
                    mData.NQ = (float)mData.NQsum / (float)mData.NQnum;
                    mData.varianceNQ = ( ( (float)mData.NQsumSQ / (float)mData.NQnum )
                                        - mData.NQ * mData.NQ ) / (float)mData.NQnum;
                    mData.standardDerivationNQ = std::sqrt( mData.varianceNQ );

                    mData.curTQ = mData.simulationTime - it2->second.getCreationTime();

                    //Update TQ
                    mData.TQnum++;
                    mData.TQsum += mData.curTQ;
                    mData.TQsumSQ += mData.curTQ * mData.curTQ;
                    mData.TQ = (float)mData.TQsum / (float)mData.TQnum;
                    mData.varianceTQ = ( ( (float)mData.TQsumSQ / (float)mData.TQnum )
                                        - mData.TQ * mData.TQ ) / (float)mData.TQnum;
                    mData.standardDerivationTQ = std::sqrt( mData.varianceTQ );

                    //As the request can now be serviced, add its finished event
                    nextFinishedTime = mData.simulationTime
                            + mServiceDurationGenerator.generate();
                    mEvents.insert( Event::makeEventPair( Event::EET_FINISHED_EVENT,
                                                          nextFinishedTime,
                                                          mData.simulationTime ) );

                    //Delete Event
                    mEvents.erase( it2 );
                }

                //Update N
                mData.Nnum++;
                mData.Nsum += mData.curN;
                mData.NsumSQ += mData.curN * mData.curN;
                mData.N = (float)mData.Nsum / (float)mData.Nnum;
                mData.varianceN = ( ( (float)mData.NsumSQ / (float)mData.Nnum )
                                    - mData.N * mData.N ) / (float)mData.Nnum;
                mData.standardDerivationN = std::sqrt( mData.varianceN );


                //Update duration statistics
                //updateDurationStatistics();

                //Update quantity statistics
                //updateQuantityStatistics();

                break;
            }

            case Event::EET_START_SERVICE_EVENT:
            {
                //THIS SHOULD NEVER HAPPEN! (start time of these events is 0...)
                break;
            }

            case Event::EET_MEASURE_EVENT:
            {
                //Schedule new measure event
                mEvents.insert( Event::makeEventPair(
                                    Event::EET_MEASURE_EVENT,
                                    mData.simulationTime
                                    + mData.measureEventDistance,
                                    mData.simulationTime ) );

                break;
            }

            default:
                break;
            }
        }

        //Delete current events cause they are no longer needed
        mEvents.erase( mData.simulationTime );

        //Check if stop criteria are met
        if( mData.standardDerivationN <= mData.minimalSD
                && mData.standardDerivationT <= mData.minimalSD )
        {
            //Only check queue parameters if there is need for a queue
            if( mData.numServiceUnits == 0
                    || ( mData.standardDerivationNQ <= mData.minimalSD
                    && mData.standardDerivationTQ <= mData.minimalSD ) )
            {
                mRunning = false;
            }
        }
    }

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

void Simulator::updateQuantityStatistics()
{
    //Update N
    mData.Nnum++;
    mData.Nsum += mData.curN;
    mData.NsumSQ += mData.curN * mData.curN;
    mData.N = (float)mData.Nsum / (float)mData.Nnum;
    mData.varianceN = ( ( (float)mData.NsumSQ / (float)mData.Nnum )
                        - mData.N * mData.N ) / (float)mData.Nnum;
    mData.standardDerivationN = std::sqrt( mData.varianceN );

    //Uodate NQ
    if( mData.numServiceUnits > 0 )
    {
        mData.NQnum++;
        mData.NQsum += mData.curNQ;
        mData.NQsumSQ += mData.curNQ * mData.curNQ;
        mData.NQ = (float)mData.NQsum / (float)mData.NQnum;
        mData.varianceNQ = ( ( (float)mData.NQsumSQ / (float)mData.NQnum )
                            - mData.NQ * mData.NQ ) / (float)mData.NQnum;
        mData.standardDerivationNQ = std::sqrt( mData.varianceNQ );
    }
    else
    {
        mData.NQ = 0;
        mData.standardDerivationNQ = 0.f;
    }
}

void Simulator::updateDurationStatistics()
{
    //Update T
    mData.Tnum++;
    mData.Tsum += mData.curT;
    mData.TsumSQ += mData.curT * mData.curT;
    mData.T = (float)mData.Tsum / (float)mData.Tnum;
    mData.varianceT = ( ( (float)mData.TsumSQ / (float)mData.Tnum )
                        - mData.T * mData.T ) / (float)mData.Tnum;
    mData.standardDerivationT = std::sqrt( mData.varianceT );

    //Update TQ
    if( mData.numServiceUnits > 0 )
    {
        mData.TQnum++;
        mData.TQsum += mData.curTQ;
        mData.TQsumSQ += mData.curTQ * mData.curTQ;
        mData.TQ = (float)mData.TQsum / (float)mData.TQnum;
        mData.varianceTQ = ( ( (float)mData.TQsumSQ / (float)mData.TQnum )
                            - mData.TQ * mData.TQ ) / (float)mData.TQnum;
        mData.standardDerivationTQ = std::sqrt( mData.varianceTQ );
    }
    else
    {
        mData.TQ = 0;
        mData.standardDerivationTQ = 0.f;
    }
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
      standardDerivationNQ( 0.f ),
      standardDerivationTQ( 0.f ),
      minimalSD( 1.e-3f ),
      Nsum( 0 ), Tsum( 0 ), NQsum( 0 ), TQsum( 0 ),
      NsumSQ( 0 ), TsumSQ( 0 ), NQsumSQ( 0 ), TQsumSQ( 0 ),
      Nnum( 0 ), Tnum( 0 ), NQnum( 0 ), TQnum( 0 ),
      curN( 0 ), curT( 0 ), curNQ( 0 ), curTQ( 0 ),
      enableMeasureEvents( true ),
      measureEventDistance( 100 )
{
}
