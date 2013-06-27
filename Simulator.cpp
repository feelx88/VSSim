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
        while( begin->second.getType() == Event::EET_START_SERVICE_EVENT )
        {
            ++begin;
        }
        mData.simulationTime = begin->first;

        //Iterate over all events (they are sorted beacuse of std::multimap)

        for( auto pair : mEvents )
        {
            //Handle only events at current timestamp
            if( pair.second.getType() == Event::EET_START_SERVICE_EVENT )
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

                //Increment service unit ussage
                mData.N.cur++;

                //Update N
                calculateStatistics( mData.N );

                //Reset times
                mData.T.cur = 0;
                mData.TQ.cur = 0;

                //If there is a finite number of service units, check if they are
                //busy
                if( mData.numServiceUnits > 0 && mData.N.cur == mData.numServiceUnits )
                {
                    //Increment queue usage
                    mData.NQ.cur++;

                    //Uodate NQ
                    calculateStatistics( mData.NQ );

                    //Create START_SERVICE event to save the creation time
                    //Use start time of 0 to indicate unknown starting time
                    mEvents.insert( Event::makeEventPair( Event::EET_START_SERVICE_EVENT,
                                                          0, mData.simulationTime ) );
                }
                else
                {
                    //As the request can be directly serviced, add its finished event
                    nextFinishedTime = mData.simulationTime
                            + mServiceDurationGenerator.generate();
                    mEvents.insert( Event::makeEventPair( Event::EET_FINISHED_EVENT,
                                                          nextFinishedTime,
                                                          mData.simulationTime ) );
                }

                break;
            }

            case Event::EET_FINISHED_EVENT:
            {
                //Decrement current service unit usage
                mData.N.cur--;

                //Update N
                calculateStatistics( mData.N );

                mData.T.cur = mData.simulationTime - pair.second.getCreationTime();

                //Update T
                calculateStatistics( mData.T );

                //Check for queued request created at time 0
                auto it = mEvents.find( 0 );
                if( it != mEvents.end()
                        && it->second.getType() == Event::EET_START_SERVICE_EVENT )
                {
                    //Decrement queue usage
                    mData.NQ.cur--;

                    //Uodate NQ
                    calculateStatistics( mData.NQ );

                    mData.TQ.cur = mData.simulationTime - it->second.getCreationTime();

                    //Update TQ
                    calculateStatistics( mData.TQ );

                    //As the request can now be serviced, add its finished event
                    nextFinishedTime = mData.simulationTime
                            + mServiceDurationGenerator.generate();
                    mEvents.insert( Event::makeEventPair( Event::EET_FINISHED_EVENT,
                                                          nextFinishedTime,
                                                          it->second.getCreationTime() ) );

                    //Delete Event
                    mEvents.erase( it );
                }

                break;
            }

            case Event::EET_START_SERVICE_EVENT:
            {
                //THIS SHOULD NEVER HAPPEN! (start time of these events is 0...)
                break;
            }

            case Event::EET_MEASURE_EVENT:
            {
                calculateStatistics( mData.N );
                calculateStatistics( mData.T );
                calculateStatistics( mData.NQ );
                calculateStatistics( mData.TQ );


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
        if( mData.N.value > 0.f
                && mData.N.standardDerivation <= mData.minimalSD
                && mData.T.standardDerivation <= mData.minimalSD )
        {
            //Only check queue parameters if there is need for a queue
            if( mData.numServiceUnits == 0
                    || ( mData.NQ.standardDerivation <= mData.minimalSD
                    && mData.TQ.standardDerivation <= mData.minimalSD ) )
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

void Simulator::calculateStatistics( Simulator::Var &var )
{
    var.num++;
    var.sum += var.cur;
    var.sumSQ += var.cur * var.cur;
    var.value = (float)var.sum / (float)var.num;
    var.variance = ( ( (float)var.sumSQ / (float)var.num )
                     - ( var.value * var.value ) );
    var.standardDerivation = std::sqrt( std::abs( var.variance ) ) / (float)var.num;
}

Simulator::SimulationData::SimulationData()
    : simulationTime( 0 ),
      nextEventTime( 0 ),
      minimalSD( 1.e-3f ),
      enableMeasureEvents( true ),
      measureEventDistance( 100 )
{
}


Simulator::Var::Var()
    : value( 0.f ),
      variance( std::numeric_limits<float>::max() ),
      standardDerivation( std::numeric_limits<float>::max() ),
      num( 0 ),
      sum( 0 ),
      sumSQ( 0 ),
      cur( 0 )
{
}
