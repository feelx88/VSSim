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

#include "Event.h"

Event::Event( E_EVENT_TYPE type, size_t startTime, size_t creationTime )
    : mType( type ),
      mStartTime( startTime ),
      mCreationTime( creationTime )
{
}

Event::E_EVENT_TYPE Event::getType() const
{
    return mType;
}

void Event::setStartTime( size_t startTime )
{
    mStartTime = startTime;
}

size_t Event::getStartTime() const
{
    return mStartTime;
}

size_t Event::getCreationTime() const
{
    return mCreationTime;
}

std::pair<size_t, Event> Event::makeEventPair( Event::E_EVENT_TYPE type,
                                               size_t startTime,
                                               size_t creationTime )
{
    return std::make_pair( startTime, Event( type, startTime, creationTime ) );
}
