/******************************************
Copyright (C) 2009-2020 Authors of CryptoMiniSat, see AUTHORS file

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
***********************************************/

#ifndef DATASYNC_SERVER_H
#define DATASYNC_SERVER_H

#include "cms_vector.h"
#include "mpi.h"

#include "solvertypes.h"
#include "shareddata.h"

namespace CMSat {

class DataSyncServer {
    public:
        DataSyncServer();
        bool actAsServer();
    private:
        bool syncFromMPI();
        void addOneBinToOthers(const Lit lit1, const Lit lit2);
        void sendDataToAll();
        void forwardNeedToInterrupt();

        cms_vector<uint32_t> syncMPIFinish;
        cms_vector<cms_vector<Lit> > bins;
        cms_vector<lbool> value;

        bool ok;

        uint32_t *sendData;

        cms_vector<MPI_Request> sendRequests;
        cms_vector<bool> sendRequestsFinished;

        cms_vector<bool> alreadyInterrupted;
        cms_vector<bool> alreadySentInterrupt;
        cms_vector<MPI_Request> interruptRequests;
        int numAlreadyInterrupted;

        int mpiSize;
        uint32_t nVars;
        uint32_t recvBinData;
        uint32_t sentBinData;
        uint32_t numGotPacket;
        uint32_t lastSendNumGotPacket;
};

}

#endif //DATASYNC_SERVER_H
