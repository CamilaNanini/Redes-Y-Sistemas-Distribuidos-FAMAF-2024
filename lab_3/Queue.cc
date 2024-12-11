#ifndef QUEUE
#define QUEUE

#include <string.h>
#include <omnetpp.h>
#include "Constants.h"

using namespace omnetpp;

class Queue: public cSimpleModule {
private:
    cQueue buffer;
    cMessage *endServiceEvent;
    simtime_t serviceTime;
public:
    Queue();
    virtual ~Queue();
    cOutVector bufferSizeVector;
    cOutVector packetDropVector;
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Queue);

Queue::Queue() {
    endServiceEvent = nullptr;
}

Queue::~Queue() {
    cancelAndDelete(endServiceEvent);
}

void Queue::initialize() {
    buffer.setName("buffer");
    endServiceEvent = new cMessage("endServiceEvent");
    packetDropVector.setName("packetDropVector");
    bufferSizeVector.setName("bufferSizeVector");
}

void Queue::finish() {
}

void Queue::handleMessage(cMessage *msg) {

    // if msg is signaling an endServiceEvent
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty()) {
            // dequeue packet
            cPacket *pkt = (cPacket*) buffer.pop();
            send(pkt, "out");
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime, endServiceEvent);
        }
    } else { // if msg is a data packet
        cPacket * pkt = ( cPacket *) msg;
        if (buffer.getLength() >= par("bufferSize").intValue() ) { //El método longValue no aparece así que use el más similar
            delete pkt;
            this->bubble("Packet dropped");
            packetDropVector.record(1);
        }else{
            // enqueue the packet
            buffer.insert(pkt);
            bufferSizeVector.record(buffer.getLength());
            // if the server is idle
            if (!endServiceEvent->isScheduled()) {
                // start the service
                serviceTime = pkt->getDuration();
                scheduleAt(simTime() + serviceTime, endServiceEvent);
            }
        }
    }
}

#endif /* QUEUE */
