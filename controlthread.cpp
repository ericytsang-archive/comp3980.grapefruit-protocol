#include "controlthread.h"
#include "receivethread.h"

DWORD WINAPI fnControl(LPVOID args)
{
    // parse thread parameters
    ControlArgs* controlArgs = (ControlArgs*) args;
    OutputDebugString("Control thread started\n");

    // create transmit and receive structures
    TransmitArgs transmitArgs;
    ReceiveArgs receiveArgs;

    // initialize transmit structure
    transmitArgs.bRequestStop    = false;
    transmitArgs.bStopped        = true;
    transmitArgs.bActive         = false;
    transmitArgs.bReset          = false;
    transmitArgs.bSYN1           = true;
    transmitArgs.pReceive        = &receiveArgs;
    transmitArgs.pTransmitBuffer = controlArgs->pTransmitBuffer;
    transmitArgs.hCommPort    = controlArgs->hCommPort;

    // initialize receive thread structures
    receiveArgs.bRequestStop = false;
    receiveArgs.bStopped     = true;
    receiveArgs.bActive      = false;
    receiveArgs.bRVI         = false;
    receiveArgs.bSYN1        = false;
    receiveArgs.pTransmit    = &transmitArgs;
    receiveArgs.hCommPort    = controlArgs->hCommPort;

    // enter main control loop
    while (true)
    {
        // exit look if we're supposed to stop
        if (controlArgs->bRequestStop) {
            OutputDebugString("Control thread stopping\n");
            receiveArgs.bRequestStop = true;
            transmitArgs.bRequestStop = true;

            if (receiveArgs.bStopped /*&& transmitArgs.bStopped*/)
            {
                OutputDebugString("Control thread stopped\n");
                controlArgs->bStopped = true;
                return 0;
            }
        }

        // restart any threads that are stopped if the state of both threads are
        // not active
        if (!receiveArgs.bActive && !transmitArgs.bActive)
        {
            if (receiveArgs.bStopped)
            {
                receiveArgs.bRequestStop = false;
                receiveArgs.bStopped = false;
                DWORD threadId;
                CreateThread(NULL, 0, fnReceiveThreadIdle, &receiveArgs, 0,
                        &threadId);
            }
            if (transmitArgs.bStopped)
            {
                transmitArgs.bRequestStop = false;
                transmitArgs.bStopped = false;
            }
        }

        // sleep so we don't burn out the core
        Sleep(CONTROL_THREAD_SLEEP_INTERVAL);
    }
}

/*
#include <iostream>
int main(void) {
    ReceiveArgs a;
    a.bRequestStop = false;
    a.bStopped = false;
    a.bActive = false;
    std::cout << a.bActive << std::endl;
    return 0;
}
*/
