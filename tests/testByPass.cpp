#include "../src/modules/liveMediaInput/SourceManager.hh"
#include "../src/modules/liveMediaOutput/SinkManager.hh"
#include "../src/AudioFrame.hh"
#include "../src/Controller.hh"
#include "../src/Callbacks.hh"
#include "../src/Utils.hh"

#include <iostream>
#include <csignal>
#include <vector>
#include <liveMedia/liveMedia.hh>
#include <string>

#define V_MEDIUM "video"
#define PROTOCOL "RTP"
#define PAYLOAD 96
#define V_CODEC "VP8"
#define BANDWITH 5000
#define V_CLIENT_PORT 6004
#define V_TIME_STMP_FREQ 90000


void signalHandler( int signum )
{
    utils::infoMsg("Interruption signal received");
    
    PipelineManager *pipe = Controller::getInstance()->pipelineManager();
    pipe->stopWorkers();
    
    utils::infoMsg("Workers Stopped");
}

int main(int argc, char** argv) 
{   
    std::string sessionId;
    std::string sdp;
    std::vector<int> readers;
    Session* session;

    PipelineManager *pipe = Controller::getInstance()->pipelineManager();
    SourceManager *receiver = pipe->getReceiver();
    SinkManager *transmitter = pipe->getTransmitter();
    
    //This will connect every input directly to the transmitter
    receiver->setCallback(callbacks::connectToTransmitter);
    
    pipe->startWorkers();
  
    signal(SIGINT, signalHandler); 
    
    for (int i = 1; i <= argc-1; ++i) {
        sessionId = utils::randomIdGenerator(ID_LENGTH);
        session = Session::createNewByURL(*(receiver->envir()), argv[0], argv[i], sessionId);
        receiver->addSession(session);
        session->initiateSession();
    }
    
    sessionId = utils::randomIdGenerator(ID_LENGTH);
    
    sdp = SourceManager::makeSessionSDP(sessionId, "this is a test");
    
    sdp += SourceManager::makeSubsessionSDP(V_MEDIUM, PROTOCOL, PAYLOAD, V_CODEC, 
                                       BANDWITH, V_TIME_STMP_FREQ, V_CLIENT_PORT);
    
    session = Session::createNew(*(receiver->envir()), sdp, sessionId);
    
    receiver->addSession(session);

    session->initiateSession();
    
    sleep(2);
    
    for (auto it : pipe->getPaths()){
        readers.push_back(it.second->getDstReaderID());
    
        sessionId = utils::randomIdGenerator(ID_LENGTH);
        if (! transmitter->addSession(sessionId, readers)){
            return 1;
        }
        readers.pop_back();
        transmitter->publishSession(sessionId);
    }

    while(pipe->getWorker(pipe->getReceiverID())->isRunning() && 
        pipe->getWorker(pipe->getTransmitterID())->isRunning()) {
        sleep(1);
    }

    return 0;
}