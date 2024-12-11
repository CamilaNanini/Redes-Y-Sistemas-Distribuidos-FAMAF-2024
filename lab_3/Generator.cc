#ifndef GENERATOR
#define GENERATOR

#include <string.h>
#include <omnetpp.h>
#include "Constants.h"

using namespace omnetpp;

class Generator : public cSimpleModule {
private:
    cPacket *sendMsgEvent;
    cStdDev transmissionStats;
public:
    Generator();
    virtual ~Generator();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};
Define_Module(Generator);

Generator::Generator() {
    sendMsgEvent = nullptr;

}

Generator::~Generator() {
    cancelAndDelete(sendMsgEvent);
}

void Generator::initialize() {
    transmissionStats.setName("TotalTransmissions");
    sendMsgEvent = new cPacket("sendEvent");
    sendMsgEvent->setByteLength(par("packetByteSize"));
    // schedule the first event at random time
    scheduleAt(par("generationInterval"), sendMsgEvent);
}

void Generator::finish() {
}

void Generator::handleMessage(cMessage *msg) {

    if (msg == sendMsgEvent) {
        // create new packet, and configure it as data packet
        cPacket *pkt = new cPacket("packet");
        pkt->setByteLength(par("packetByteSize"));
        pkt->setKind(DATA_KIND); 

        // send to the output
        send(pkt, "out");

        // compute the new departure time
        simtime_t departureTime = simTime() + par("generationInterval");
        // schedule the new packet generation
        scheduleAt(departureTime, sendMsgEvent); // Esto deberia cambiar en funcion de la instruccion de control de flujo
    }
    else { //Si el mensaje no es un evento de envío, es una instruccion de control de flujo
        delete msg; //Simplemente para que no se quede en la memoria hasta que lo implemente
    }
}

#endif /* GENERATOR */
