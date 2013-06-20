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

#ifndef EVENT_H
#define EVENT_H

#include <utility>
#include <cstddef>

class Event
{
public:
    enum E_EVENT_TYPE
    {
        EET_INCOMING_EVENT = 0,
        EET_FINISHED_EVENT,
        EET_START_SERVICE_EVENT,
        EET_MEASURE_EVENT
    };

    Event( E_EVENT_TYPE type, size_t startTime, size_t creationTime );

    E_EVENT_TYPE getType() const;
    void setStartTime( size_t startTime );
    size_t getStartTime() const;
    size_t getCreationTime() const;

    static std::pair<size_t, Event> makeEventPair( E_EVENT_TYPE type,
                                                          size_t startTime,
                                                          size_t creationTime );

private:
    E_EVENT_TYPE mType;
    size_t mStartTime, mCreationTime;
};

#endif // EVENT_H
