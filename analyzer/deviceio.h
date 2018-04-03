/*
(C) Copyright 2015 Jeremy Burton

This file is part of Sark-100-antenna-analyzer.

Sark-100-antenna-analyzer is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Sark-100-antenna-analyzer is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DEVICEIO_H
#define DEVICEIO_H

#include <QString>
#include "scandata.h"

#define FMIN 1000000
#define FMAX 700000000

class DeviceIO
{
public:
    DeviceIO();
    ~DeviceIO();

    bool IsUp();
    void Cmd_Scan(long fstart, long fend, long fstep, EventReceiver *erx);
    void Cmd_Single(long freq, Sample &sample);
    void Cmd_Off();

protected:
    int devfd;

private:
};

#endif // DEVICEIO_H
