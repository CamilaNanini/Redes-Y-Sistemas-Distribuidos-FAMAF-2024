#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>
#include "constants.h"

/*IDEA: Mando en primer momento un paquete Hello de cada nodo para reconocer la red en la que me voy a mover, a partir de la 
cantidad de saltos que de para llegar al destino y la cantidad de saltos que hago para volver al nodo origen; calculo que ruta me 
conviene (sentido horario 0 o sentido anti-horario 1).
*/

using namespace omnetpp;

class Net: public cSimpleModule {
private:
    int link_to_use;
public:
    Net();
    virtual ~Net();
    cOutVector hopsVector; //Para contar los saltos en general en TODA la simulacion
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Net);

#endif /* NET */

Net::Net() {
}

Net::~Net() {
}

void Net::initialize() {
    hopsVector.setName("hopsVector");
    link_to_use = 0; //En qué sentido voy a mandar a los paquetes

    //Todos los nodos inician con este paquete mandandolo a los demas 
    Packet *hello_pkt = new Packet("Hello_Packet",this->getParentModule()->getIndex());
    hello_pkt->setKind(HELLO_KIND);
    hello_pkt->setByteLength(BYTESIZE);
    //Ver packet.msg, donde se redefine la estructura de cpacket en Packet
    //No lo descubrí se lo pregunté a titoGPT pq vi el archivo y no entendía
    hello_pkt->setSource(this->getParentModule()->getIndex()); //Implicitamente se le está dando una numeración, me deja saber en que paquete estoy 
    hello_pkt->setDestination(par("destination")); //A dónde tengo que mandar el menasaje
    hello_pkt->setHopCount(0); //Cuentos todos los saltos que hace el paquete hello
    hello_pkt->setHopsToDestination(0); //Cuento los saltos solo hacia el destino
    send(hello_pkt, "toLnk$o", LINK);
}

void Net::finish() {
}

void Net::handleMessage(cMessage *msg) {
    // All msg (events) on net are packets
    Packet *pkt = (Packet *) msg;
    int kind_to_comparete = pkt->getKind();
    if (kind_to_comparete == HELLO_KIND){
        //Si donde estoy parada es el mismo lugar de donde vino el hello -> hago cambios
        if (pkt->getSource() == this->getParentModule()->getIndex()){
            int length_of_net = pkt->getHopCount();
            if (pkt->getHopsToDestination() <= (length_of_net / 2)){
                link_to_use = LINK;
            }else {
                link_to_use = OTHER_LINK;
            }
            delete (pkt);
        }else{
            //Si estoy en donde es el destino, detengo una de las cuentas de salto
            if (pkt->getDestination() == this->getParentModule()->getIndex()){
                pkt->setHopsToDestination(pkt->getHopCount());
            }
            //Si tengo que seguir en la red sido sumando saltos
            pkt->setHopCount(pkt->getHopCount() + 1);
            send(pkt, "toLnk$o", LINK);
        }
    }else if (kind_to_comparete == DATA_KIND){ //Si no es hello entonces lo manejo como una data
        if (pkt->getDestination() == this->getParentModule()->getIndex()) {
            send(pkt, "toApp$o");
        }
        else {
            //Debo solo contar acá???
            hopsVector.record(1); //Mando un mensaje que va a saltar para otro nodo 
            send(pkt, "toLnk$o", link_to_use); //Mando por la ruta que dije
        }
    }    
}
