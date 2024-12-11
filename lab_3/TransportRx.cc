#ifndef TRANSPORT_RX
#define TRANSPORT_RX

#include <omnetpp.h>
#include "Constants.h"

using namespace omnetpp;

class TransportRx: public cSimpleModule {
private:
    cQueue buffer;
    cPacket *endServiceEvent;
    cPacket *sendFeedbackEvent;
    // Parámetros
    int bufferSize;
    double recoverySpeed;
    // Para uso interno
    int windowSize;
    int optimalWindowSize;
    int receivedCount;
    bool parity;
public:
    TransportRx();
    virtual ~TransportRx();
    cOutVector bufferSizeVector;
    cOutVector packetDropVector;
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    virtual void sendFeedback();
};

Define_Module(TransportRx);

TransportRx::TransportRx() {
    endServiceEvent = nullptr;
    sendFeedbackEvent = nullptr;
}

TransportRx::~TransportRx() {
    cancelAndDelete(endServiceEvent);
    cancelAndDelete(sendFeedbackEvent);
}

void TransportRx::initialize() {
    buffer.setName("buffer");
    bufferSize = par("bufferSize");

    endServiceEvent = new cPacket("endService");
    sendFeedbackEvent = new cPacket("sendFeedback");
    optimalWindowSize = (int) bufferSize/2;
    windowSize = optimalWindowSize;
    receivedCount = 0;
    recoverySpeed = par("recoverySpeed");
    parity = true;
    packetDropVector.setName("packetDropVector");
    bufferSizeVector.setName("bufferSizeVector");
    scheduleAt(simTime(), sendFeedbackEvent);
}

void TransportRx::finish() {
}

void TransportRx::handleMessage(cMessage *msg) {
    if(msg == endServiceEvent){
        if (!buffer.isEmpty()) {
            // Enviar el data packet hacia el sink
            cPacket *data_pkt = (cPacket*) buffer.pop();
            send(data_pkt, "toOut$o"); // Camino al sink
            simtime_t serviceTime = data_pkt->getDuration();
            simtime_t departureTime = simTime() + serviceTime;

            // Preparar el envío de los paquetes que hay en el resto de la queue
            if(!endServiceEvent->isScheduled()) {
                scheduleAt(departureTime, endServiceEvent);
            }
        }
    }
    else if (msg == sendFeedbackEvent){
        sendFeedback();
    }
    else if (msg->getKind() == DATA_KIND) {
        cPacket *pkt = (cPacket*) msg;
        // Si el buffer no está lleno, encolar el paquete
        if (buffer.getLength() < bufferSize) {
            buffer.insert(pkt);
            simtime_t serviceTime = pkt->getDuration();
            simtime_t departureTime = simTime()+serviceTime;
            receivedCount++;
            if(!endServiceEvent->isScheduled()) {
                scheduleAt(departureTime, endServiceEvent);
            }
        }
        // Si el buffer está lleno, descartar el paquete y notificar que todo mal (En parada y espera nunca pasa)
        else {
            this->bubble("Packet dropped");
            packetDropVector.record(1);
            delete pkt;
        }

        bufferSizeVector.record(buffer.getLength());
    }
    else if (msg->getKind() == BURSTENDED_KIND && (msg->par("parity").boolValue() == parity)) {
        parity = !parity;
        // Si esto pasa, la próxima ráfaga tiene un tercio el tamaño de la anterior
        if (receivedCount < windowSize || windowSize==0) {
            windowSize = (int) windowSize/3 + 1;

            if (windowSize > bufferSize-buffer.getLength()) {
                windowSize = (bufferSize-buffer.getLength())/3 + 1;
            }
            if (!sendFeedbackEvent->isScheduled()) {
                scheduleAt(simTime(), sendFeedbackEvent);
            }
        } else if (receivedCount == windowSize) {
            optimalWindowSize = (int) (bufferSize-buffer.getLength())/2;
            // El windowSize debería lentamente acercarse al ideal, si es que se había reducido la ventana por congestión
            windowSize = (int) ((1-recoverySpeed) * (std::min(windowSize, optimalWindowSize)) + recoverySpeed * optimalWindowSize);
            if (!sendFeedbackEvent->isScheduled()) {
                scheduleAt(simTime(), sendFeedbackEvent);
            }
        }
        receivedCount = 0;
        delete msg;
    } else {
        delete msg;
    }
}

void TransportRx::sendFeedback() {
    // Enviar un ACK
    cPacket *ack_pkt = new cPacket("ACK");
    ack_pkt->addPar("freeSpace").setLongValue(windowSize);
    ack_pkt->addPar("parity").setBoolValue(parity);
    ack_pkt->setKind(FEEDBACK_KIND);
    ack_pkt->setByteLength(CONTROLBYTE_SIZE);
    send(ack_pkt, "toApp"); // Camino al sink
}

#endif /* TRANSPORT_RX */
