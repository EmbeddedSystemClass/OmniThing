#include <iostream>
#include <frozen.h>
#include <unistd.h>

#include "OmniThing.h"
#include "OmniUtil.h"
#include "ContactSensor.h"
#include "Switch.h"

#include "NetworkReceiverHttpLib.h"
#include "DigitalInputPinStub.h"

int main(int argc, char* argv[])
{
    using namespace omni;

    OmniThing& omnithing = OmniThing::getInstance();

    const char* ip = "192.168.2.101";
    NetworkReceiverHttpLib receiver(ip, 1337);

    omnithing.setNetworkReceiver(&receiver);

    DigitalInputPinStub stub(22, false);
    ContactSensor contact(stub, false);
    omnithing.addDevice(&contact);

    omnithing.init();

    while(true)
    {
        sleepMillis(10);

        omnithing.run();
    }

    return 0;
}

