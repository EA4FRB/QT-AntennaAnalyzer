/*
(C) Copyright 2015 Jeremy Burton

This file is part of Sark-100-antenna-analyzer.

Sark-100-antenna-analyzer is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Sark-100-antenna-analyzerr is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <complex>
#include <errno.h>

#include "scandata.h"
#include "deviceio.h"
#include "sark_client.h"

extern ScanData scandata;

DeviceIO::DeviceIO()
{
    int rc = Sark_Connect();
    if (rc < 0)
    {
        devfd = -1;
        printf("Cannot connect to SARK-110\n");
        return;
    }
    printf("SARK-110 Connected\n");
    devfd = 0;
}

DeviceIO::~DeviceIO()
{
    devfd = -1;
    Sark_Close();
}


bool DeviceIO::IsUp()
{
   return (devfd != -1);
}

void DeviceIO::Cmd_Scan(long fstart, long fend, long fstep, EventReceiver *erx)
{
    Sample sample;

    scandata.points.resize(0);
    int step = 0;
    int nsteps = (fend-fstart)/fstep;
    for (long freq = fstart; freq < fend; freq+=fstep, step++)
    {
        float fR, fX, fS21Re, fS21Im;
        int rc = Sark_Meas_Rx(freq, true, 1, &fR, &fX, &fS21Re, &fS21Im);
        if (rc < 0)
        {
            devfd = -1;
            Sark_Close();
            break;
        }
        std::complex<double> cxZ(fR, fX);
        std::complex<double> cxRho = (cxZ - 50.0) / (cxZ + 50.0);
        if (std::abs(cxRho) > 0.980197824)
            sample.swr = 99.999;
        else
            sample.swr = (1.0 + std::abs(cxRho)) / (1.0 - std::abs(cxRho));
        sample.R = fR;
        sample.X = fX;
        sample.Z = std::abs(cxZ);
        sample.freq = freq;
        scandata.points.push_back(sample);
        erx->RaiseEvent(EventReceiver::progress_event, 100 * step / nsteps);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    scandata.UpdateStats();
}

void DeviceIO::Cmd_Off()
{
    float fR, fX, fS21Re, fS21Im;
    int rc = Sark_Meas_Rx(0, true, 1, &fR, &fX, &fS21Re, &fS21Im);
    if (rc < 0)
    {
        devfd = -1;
        Sark_Close();
    }
}

void DeviceIO::Cmd_Single(long freq, Sample &sample)
{
    float fR, fX, fS21Re, fS21Im;
    int rc = Sark_Meas_Rx(freq, true, 1, &fR, &fX, &fS21Re, &fS21Im);
    if (rc < 0)
    {
        devfd = -1;
        Sark_Close();
    }
    std::complex<double> cxZ(fR, fX);
    std::complex<double> cxRho = (cxZ - 50.0) / (cxZ + 50.0);
    if (std::abs(cxRho) > 0.980197824)
        sample.swr = 99.999;
    else
        sample.swr = (1.0 + std::abs(cxRho)) / (1.0 - std::abs(cxRho));
    sample.R = fR;
    sample.X = fX;
    sample.Z = std::abs(cxZ);
    sample.freq = freq;
    scandata.points.push_back(sample);
}
