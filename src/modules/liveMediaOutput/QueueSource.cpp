#include "QueueSource.hh"
#include "SinkManager.hh"

QueueSource* QueueSource::createNew(UsageEnvironment& env, Reader *reader, int readerId) {
  return new QueueSource(env, reader, readerId);
}


QueueSource::QueueSource(UsageEnvironment& env, Reader *reader, int readerId)
  : FramedSource(env), fReader(reader), fReaderId(readerId) {
}

void QueueSource::doGetNextFrame() 
{
    checkStatus();

    if ((frame = fReader->getFrame()) == NULL) {
        nextTask() = envir().taskScheduler().scheduleDelayedTask(POLL_TIME,
            (TaskFunc*)QueueSource::staticDoGetNextFrame, this);
        return;
    }
    
    fPresentationTime = frame->getPresentationTime();
    if (fMaxSize < frame->getLength()){
        fFrameSize = fMaxSize;
        fNumTruncatedBytes = frame->getLength() - fMaxSize;
    } else {
        fNumTruncatedBytes = 0; 
        fFrameSize = frame->getLength();
    }
    
    memcpy(fTo, frame->getDataBuf(), fFrameSize);
    fReader->removeFrame();
    
    afterGetting(this);
}

void QueueSource::doStopGettingFrames() {
    return;
}

void QueueSource::staticDoGetNextFrame(FramedSource* source) {
    source->doGetNextFrame();
}

void QueueSource::checkStatus()
{
    if (fReader->isConnected()) {
        return;
    }

    SinkManager* transmitter = SinkManager::getInstance();

    transmitter->deleteReader(fReaderId);

    std::string sessionID = transmitter->getSessionIdFromReaderId(fReaderId);

    if (!sessionID.empty()) {
        transmitter->removeSession(sessionID);
    }
}
