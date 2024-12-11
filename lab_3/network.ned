simple Generator
{
    parameters:
        volatile double generationInterval; // sec
        int packetByteSize; // Esto lo agregué en un momento de locura total <- esta locura hace que se pueda encontrar el parametro usando par() :))))
        @display("i=block/source");
    gates:
        output out;
}

simple Queue
{
    parameters:
       // volatile double serviceTime; // automatically setted by the package transmission duration
       int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        input in;
        output out;
}

simple Sink
{
    parameters:
        @display("i=block/sink");
    gates:
        input in;
}

simple TransportTx{
    parameters:
        int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        input toApp;
        inout toOut;
}

simple TransportRx{
    parameters:
        int bufferSize;
        // Velocidad a la que se irá volviendo al tamaño de ventana ideal si hubo congestión
        double recoverySpeed;
        @display("i=block/queue;q=buffer");
    gates:
        output toApp;
        inout toOut;
}

module NodeTx
{
	parameters:
	    @display("i=block/routing");
	gates:
	    output out;
        input in;
	submodules:
	    gen: Generator{
	    	parameters:
	    	    @display("p=75,50");
	    }
	    transport: TransportTx{
            parameters:
                @display("p=75,250");
        }
	 connections:
        //Enviar los paquetes generados hacia la cola de transporte
        gen.out --> transport.toApp;
        transport.toOut$o --> out;

        //Para poder controlar el flujo, con indicaciones recibidas desde TransportRx
        in --> transport.toOut$i;
}

module NodeRx
{
	parameters:
	    @display("i=block/routing");
	gates:
	    input in;
        output out;
	submodules:
	    sink: Sink {
	    	parameters:
	    	    @display("p=75,50");
	    }
	    transport: TransportRx{
            parameters:
                @display("p=75,250");
        }
	 connections:
        // Al obtener los paquetes de la salida de NodeTx, se los mando al consumidor
        in --> transport.toOut$i;
        
        //Caso 1 - Diseño 2
        // transport.toOut$o --> {datarate = 0.5Mbps; delay = 100us;} --> sink.in;
        
        //Caso 2 - Diseño 2
        transport.toOut$o --> {datarate = 1Mbps; delay = 100us;} --> sink.in;
        
        // Para enviarle a NodeTx la informacion de control
        transport.toApp --> out;
}

network Network
{
    @display("bgl=2");
    submodules:

        nodeRx: NodeRx {
            @display("p=100,100");
        }
        nodeTx: NodeTx {
            @display("p=100,300");
        }
        queueTowards: Queue {
            parameters:
            @display("p=50,200");
        }
        queueBackwards: Queue {
            parameters:
            @display("p=150,200");
        }
    connections:
        //Caso 1 - Diseño 2
        // nodeTx.out --> {datarate = 1Mbps; delay = 100us;} --> queueTowards.in;
        // queueTowards.out --> {datarate = 1Mbps; delay = 100us;} --> nodeRx.in;

        //Caso 2 - Diseño 2
        nodeTx.out --> {datarate = 1Mbps; delay = 100us;} --> queueTowards.in;
        queueTowards.out --> {datarate = 0.5Mbps; delay = 100us;} --> nodeRx.in;

        nodeRx.out -->  {datarate = 1Mbps; delay = 100us;} --> queueBackwards.in;
        queueBackwards.out --> {datarate = 1Mbps; delay = 100us;} --> nodeTx.in;
}
