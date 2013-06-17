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

Event::Event( E_EVENT_TYPE type, unsigned long startTime )
    : mType( type ),
      mStartTime( startTime )
{
}

Event::E_EVENT_TYPE Event::getType() const
{
    return mType;
}

unsigned long Event::getStartTime() const
{
    return mStartTime;
}

std::pair<unsigned long, Event> Event::makeEventPair(Event::E_EVENT_TYPE type, unsigned long startTime)
{
    return std::make_pair( startTime, Event( type, startTime ) );
}
