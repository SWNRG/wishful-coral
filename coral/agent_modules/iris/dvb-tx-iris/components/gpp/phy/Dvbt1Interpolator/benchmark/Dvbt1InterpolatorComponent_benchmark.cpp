/**
 * \file components/gpp/phy/Dvbt1Interpolator/benchmark/Dvbt1InterpolatorComponent_benchmark.cpp
 * \version 0.1
 *
 * \section COPYRIGHT
 *
 * Copyright 2012-2016 The Iris Project Developers. See the
 * COPYRIGHT file at the top-level directory of this distribution
 * and at http://www.softwareradiosystems.com/iris/copyright.html.
 *
 * \section LICENSE
 *
 * This file is part of the Iris Project.
 *
 * Iris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Iris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * A copy of the GNU Lesser General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 * \section DESCRIPTION
 *
 * Main benchmark file for Dvbt1Interpolator component.
 */

#include "../Dvbt1InterpolatorComponent.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>
#include "utility/DataBufferTrivial.h"
#include "utility/RawFileUtility.h"

using namespace std;
using namespace iris;
using namespace iris::phy;
namespace bp = boost::posix_time;

int main(int argc, char* argv[])
{
  double outarr[] = {100e6/10,100e6/9,100e6/8,100e6/7};
  typedef std::complex<float>   Cplx;

  for(int oi = 0; oi < sizeof(outarr) / sizeof(outarr[0]); oi++)
  {
    Dvbt1InterpolatorComponent mod("test");
    mod.setValue("outsamplerate", outarr[oi]);
    mod.registerPorts();

    map<string, int> iTypes,oTypes;
    iTypes["input1"] = TypeInfo< Cplx >::identifier;
    mod.calculateOutputTypes(iTypes,oTypes);

    DataBufferTrivial< Cplx > in;
    DataBufferTrivial< Cplx > out;

    DataSet< Cplx >* iSet = NULL;
    int num = ((8192+8192/4)/6817)*((3*(8*204*(10000000/204))/4)/6);
    in.getWriteData(iSet, num);
    boost::random::mt19937 rng;
    boost::random::normal_distribution<> gauss(0,1);
    for(int i=0;i<num;i++)
        iSet->data[i] = Cplx(gauss(rng), gauss(rng)); // random complex data
    in.releaseWriteData(iSet);

    mod.setBuffers(&in,&out);
    mod.initialize();

    bp::ptime t1(bp::microsec_clock::local_time());
    mod.process();
    bp::ptime t2(bp::microsec_clock::local_time());
    bp::time_duration time = t2-t1;

    float sampsPerSec = ((num)/(1.0e-9 * time.total_nanoseconds()));
    cout << outarr[oi] << ": Rate = " << sampsPerSec*1.0e-6 << " Msamp/s" << endl;

    float maxRate = (64.0e6/7); // table 4 pag. 26
    cout << "Maximum sample rate in ETSI EN 300 744 is "
      << maxRate*1.0e-6 << " Msamp/s" << endl;

    if(sampsPerSec > 1.2 * maxRate)
      cout << "    >>>> " << sampsPerSec/maxRate << "x: GOOD! <<<<" << endl;
    else
      cout << "    >>>> " << sampsPerSec/maxRate << "x: OOPS! <<<<" << endl;
  }
}

