#ifndef TRANSPORT_TX
#define TRANSPORT_TX

#include <omnetpp.h>
#include "Constants.h"

using namespace omnetpp;

class TransportTx: public cSimpleModule {
private:
    cQueue buffer;
    int windowSize;
    int toSend;
    int bufferSize;
    bool parity;
    cMessage *prepareBurstEvent;
    cMessage *sendPacketEvent;
    cMessage *burstEndedEvent;
public:
    TransportTx();
    virtual ~TransportTx();
    cOutVector bufferSizeVector;
    cOutVector packetDropVector;
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    virtual void prepareBurst();
    virtual void sendPacket();
};

Define_Module(TransportTx);

TransportTx::TransportTx() {
    prepareBurstEvent = nullptr;
    sendPacketEvent = nullptr;
    burstEndedEvent = nullptr;
}

TransportTx::~TransportTx() {
    cancelAndDelete(sendPacketEvent);
    cancelAndDelete(prepareBurstEvent);
    cancelAndDelete(burstEndedEvent);
}

void TransportTx::initialize() {
    buffer.setName("buffer");

    bufferSize = par("bufferSize");

    sendPacketEvent = new cMessage("sendPacket");
    prepareBurstEvent = new cMessage("prepareBurst");
    burstEndedEvent = new cMessage("burstEnded");

    parity = true;
    windowSize = 0;
    toSend = 0;
    bufferSizeVector.setName("bufferSizeVector");
    packetDropVector.setName("packetDropVector");
}

void TransportTx::finish() {
}

void TransportTx::handleMessage(cMessage *msg) {
    if (msg == sendPacketEvent) {
        sendPacket();
    }
    else if(msg == prepareBurstEvent){
        prepareBurst();
    }
    else if (msg == burstEndedEvent) {
        cPacket *info_pkt = new cPacket("burstEnded");
        info_pkt->setByteLength(CONTROLBYTE_SIZE);
        info_pkt->setKind(BURSTENDED_KIND);
        info_pkt->addPar("parity").setBoolValue(parity);
        send(info_pkt, "toOut$o");
        simtime_t serviceTime = info_pkt->getDuration();
        scheduleAt(simTime() + 1, burstEndedEvent);
    }
    // El mensaje podria ser un ACK entrante, o un DATA PACKET saliente
    else if (msg->getKind() == FEEDBACK_KIND) {
        // Si se recibe ACK ver si pudo enviar el proximo packet
        simtime_t departureTime = simTime();
        windowSize = msg->par("freeSpace").longValue();
        parity = msg->par("parity").boolValue();
        delete msg;

        // Si llegó un feedback, comenzar de 0 los envíos
        if (burstEndedEvent->isScheduled()) {
            cancelEvent(burstEndedEvent);
        }

        if (prepareBurstEvent->isScheduled()) {
            cancelEvent(prepareBurstEvent);
        }

        if (sendPacketEvent->isScheduled()) {
            cancelEvent(sendPacketEvent);
        }

        toSend = 0;
        prepareBurst();
    }
    else if (msg->getKind() == DATA_KIND) {
        // Si el buffer no está lleno, encolar el paquete
        if (buffer.getLength() < bufferSize) {
            buffer.insert(msg);
        }
        // Si el buffer está lleno, descartar el paquete
        else {
            this->bubble("Packet dropped");
            packetDropVector.record(1);
            delete msg;
        }
        bufferSizeVector.record(buffer.getLength());
    }
}

void TransportTx::sendPacket(){
    if (toSend > 0) {
        cPacket *pkt = (cPacket*) buffer.pop();
        send(pkt, "toOut$o");
        toSend--;
        simtime_t serviceTime = pkt->getDuration();
        scheduleAt(simTime() + serviceTime, sendPacketEvent);
    }
    else if (toSend == 0) {
        // Terminó la ráfaga. No hay problema que se schedulee burstEnded para tiempo simTime, porque sabemos que no se envió ningún paquete en este tiempo
        scheduleAt(simTime(), burstEndedEvent);
    }
}

void TransportTx::prepareBurst() {
    if (buffer.getLength() >= windowSize && toSend == 0) {
        // Preparar la ventana con los proximos windowSize packets
        toSend = windowSize;
        // Para obtener información del canal
        cChannel *channel = gate("toOut$o")->getTransmissionChannel();
        // Si el canal estaba ocupado, se comenzará la ráfaga cuando la antigua transmisión termine
        if (channel->isBusy()) {
            scheduleAt(gate("toOut$o")->getTransmissionChannel()->getTransmissionFinishTime(), sendPacketEvent);
        } 
        else {
            scheduleAt(simTime(), sendPacketEvent);
        }

    }
    else { // Esperar que se generen windowSize packet
        simtime_t waitTime = 0.1;
        scheduleAt(simTime() + waitTime, prepareBurstEvent);
    }

}

#endif /* TRANSPORT_RX */
