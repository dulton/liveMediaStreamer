/*
 *  Filter.hh - Filter base classes
 *  Copyright (C) 2014  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of liveMediaStreamer.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:  David Cassany <david.cassany@i2cat.net>,
 *            Marc Palau <marc.palau@i2cat.net>
 *            
 */

#ifndef _FILTER_HH
#define _FILTER_HH

#include <map>
#include <vector>
#include <queue>
#include <mutex>

#ifndef _FRAME_QUEUE_HH
#include "FrameQueue.hh"
#endif

#ifndef _IO_INTERFACE_HH
#include "IOInterface.hh"
#endif

#ifndef _WORKER_HH
#include "Worker.hh"
#endif

#include <iostream>

#include "Event.hh"
#define DEFAULT_ID 1
#define MAX_WRITERS 16
#define MAX_READERS 16

class BaseFilter : public Runnable {
    
public:
    bool connectOneToOne(BaseFilter *R);
    bool connectManyToOne(BaseFilter *R, int wId);
    bool connectOneToMany(BaseFilter *R, int rId);
    bool connectManyToMany(BaseFilter *R, int rId, int wId);
    //Only for testing! Should not exist
    bool connect(Reader *r);
    ///////////////////////////////////////
    bool disconnect(int wId, BaseFilter *R, int rId);
    FilterType getType() {return fType;};
    int generateReaderID();
    int generateWriterID();
    const int getMaxWriters() const {return maxWriters;};
    const int getMaxReaders() const {return maxReaders;};
    virtual void pushEvent(Event e);
    
protected:
    BaseFilter(int maxReaders_, int maxWriters_, bool force_ = false);
    //TODO: desctructor
    
    virtual FrameQueue *allocQueue(int wId) = 0;
    virtual bool processFrame(bool removeFrame = false) = 0;
    virtual Reader *setReader(int readerID, FrameQueue* queue);
    virtual void initializeEventMap() = 0;

    Reader* getReader(int id);
    bool demandOriginFrames();
    bool demandDestinationFrames();
    void addFrames();
    void removeFrames();
    void processEvent(); 
    bool newEvent();

    std::map<std::string, std::function<void(Jzon::Node* params)> > eventMap; 
    
protected:
    std::map<int, Reader*> readers;
    std::map<int, Writer*> writers;
    std::map<int, Frame*> oFrames;
    std::map<int, Frame*> dFrames;
    FilterType fType;
      
private:
    bool force;
    int maxWriters;
    int maxReaders;
    std::priority_queue<Event> eventQueue;
    std::mutex eventQueueMutex;
    
    std::map<int, bool> rUpdates;
};

class OneToOneFilter : public BaseFilter {
    
protected:
    OneToOneFilter(bool force_ = false);
    //TODO: desctructor
    virtual bool doProcessFrame(Frame *org, Frame *dst) = 0;
    
private:
    bool processFrame(bool removeFrame = false);
    using BaseFilter::demandOriginFrames;
    using BaseFilter::demandDestinationFrames;
    using BaseFilter::addFrames;
    using BaseFilter::removeFrames;
    //using BaseFilter::readers;
    using BaseFilter::writers;
    using BaseFilter::oFrames;
    using BaseFilter::dFrames;
};

class OneToManyFilter : public BaseFilter {
    
protected:
    OneToManyFilter(int writersNum = MAX_WRITERS, bool force_ = false);
    //TODO: desctructor
    virtual bool doProcessFrame(Frame *org, std::map<int, Frame *> dstFrames) = 0;
    
private:
    bool processFrame(bool removeFrame = false);
    using BaseFilter::demandOriginFrames;
    using BaseFilter::demandDestinationFrames;
    using BaseFilter::addFrames;
    using BaseFilter::removeFrames;
    using BaseFilter::readers;
    using BaseFilter::writers;
    using BaseFilter::oFrames;
    using BaseFilter::dFrames;
};

class HeadFilter : public BaseFilter {
public:
    //TODO:implement this function
    void pushEvent(Event e);

protected:
    HeadFilter(int writersNum = MAX_WRITERS);
    //TODO: desctructor
    int getNullWriterID();
    
private:
    //TODO: error message
    bool processFrame(bool removeFrame = false) {};
    using BaseFilter::demandOriginFrames;
    using BaseFilter::demandDestinationFrames;
    using BaseFilter::addFrames;
    using BaseFilter::removeFrames;
    using BaseFilter::readers;
    using BaseFilter::oFrames;
    using BaseFilter::dFrames;
};

class TailFilter : public BaseFilter {
public:
    void pushEvent(Event e);

protected:
    TailFilter(int readersNum = MAX_READERS);
    //TODO: desctructor
    
private:
    bool processFrame(bool removeFrame = false) {};
    FrameQueue *allocQueue(int wId) {return NULL;};
    using BaseFilter::demandOriginFrames;
    using BaseFilter::demandDestinationFrames;
    using BaseFilter::addFrames;
    using BaseFilter::removeFrames;
    using BaseFilter::oFrames;
    using BaseFilter::dFrames;
};

class ManyToOneFilter : public BaseFilter {
    
protected:
    ManyToOneFilter(int readersNum = MAX_READERS, bool force_ = false);
    //TODO: desctructor
    virtual bool doProcessFrame(std::map<int, Frame *> orgFrames, Frame *dst) = 0;

private:
    bool processFrame(bool removeFrame = false);
    using BaseFilter::demandOriginFrames;
    using BaseFilter::demandDestinationFrames;
    using BaseFilter::addFrames;
    using BaseFilter::removeFrames;
    using BaseFilter::writers;
    using BaseFilter::oFrames;
    using BaseFilter::dFrames;
};

#endif
