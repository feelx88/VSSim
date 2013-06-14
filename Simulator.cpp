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

Simulator::Simulator( unsigned int incomingRate, unsigned int serviceDuration,
                      QObject *parent )
    : QThread( parent ),
      mRunning( true )
{
    mIncomingRateGenerator.setValue( incomingRate );
    mServiceDurationGenerator.setValue( serviceDuration );

    mTimer.setInterval( 100 );
    connect( &mTimer, SIGNAL( timeout() ), this, SLOT( emitUpdateSignal() ) );
    mTimer.start( 100 );
}

void Simulator::run()
{
    while( mRunning )
    {
        mData.simulationTime++;
        //Dummy code for gui testing
        if( mIncomingRateGenerator.generate() > 1000 )
        {
            mRunning = false;
        }
        //End dummy code
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
      minimalSD( 0.000000001f )
{
}
