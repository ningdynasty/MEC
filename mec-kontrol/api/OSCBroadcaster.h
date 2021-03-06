#pragma once

#include "KontrolModel.h"
#include "ChangeSource.h"

#include <memory>
#include <ip/UdpSocket.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <readerwriterqueue.h>
#include <condition_variable>

namespace Kontrol {


class OSCBroadcaster : public KontrolCallback {
public:
    static const unsigned int OUTPUT_BUFFER_SIZE = 1024;

    OSCBroadcaster(Kontrol::ChangeSource src, unsigned keepAlive, bool master);
    ~OSCBroadcaster();
    bool connect(const std::string &host, unsigned port);
    void stop() override;

    void sendPing(unsigned port);

    // KontrolCallback
    void rack(ChangeSource, const Rack &) override;
    void module(ChangeSource, const Rack &rack, const Module &) override;
    void page(ChangeSource src, const Rack &rack, const Module &module, const Page &p) override;
    void param(ChangeSource src, const Rack &rack, const Module &module, const Parameter &) override;
    void changed(ChangeSource src, const Rack &rack, const Module &module, const Parameter &p) override;
    void resource(ChangeSource, const Rack &, const std::string &, const std::string &) override;
    void deleteRack(ChangeSource, const Rack &) override;
    void activeModule(ChangeSource source, const Rack &rack, const Module &module) override;
    void ping(ChangeSource src, const std::string &host, unsigned port, unsigned keepAlive) override;
    void assignMidiCC(ChangeSource, const Rack &, const Module &, const Parameter &, unsigned midiCC) override;
    void unassignMidiCC(ChangeSource, const Rack &, const Module &, const Parameter &, unsigned midiCC) override;
    void savePreset(ChangeSource, const Rack &, std::string preset) override;
    void loadPreset(ChangeSource, const Rack &, std::string preset) override;
    void saveSettings(ChangeSource, const Rack &) override;
    void loadModule(ChangeSource, const Rack &, const EntityId &, const std::string &) override;

    void assignModulation(ChangeSource, const Rack &, const Module &, const Parameter &, unsigned bus) override;
    void unassignModulation(ChangeSource, const Rack &, const Module &, const Parameter &, unsigned bus) override;

    void midiLearn(ChangeSource src, bool b) override;
    void modulationLearn(ChangeSource src, bool b) override;

    void publishStart(ChangeSource, unsigned numRacks) override;
    void publishRackFinished(ChangeSource, const Rack &) override;

    bool isThisHost(const std::string &host, unsigned port) { return host == host_ && port == port_; }

    bool isActive();
    void writePoll();

    std::string host() { return host_; }

    unsigned port() { return port_; }


protected:
    void send(const char *data, unsigned size);
    bool broadcastChange(ChangeSource src);

private:
    void flush();

    struct OscMsg {
        static const int MAX_N_OSC_MSGS = 128;
        static const int MAX_OSC_MESSAGE_SIZE = 512;
        int size_;
        char buffer_[MAX_OSC_MESSAGE_SIZE];
    };

    std::string host_;
    unsigned int port_;
    std::shared_ptr<UdpTransmitSocket> socket_;
    char buffer_[OUTPUT_BUFFER_SIZE];
#ifdef __COBALT__
    struct timespec lastPing_;
#else
    std::chrono::steady_clock::time_point lastPing_;
#endif
    unsigned keepAliveTime_;

    moodycamel::BlockingReaderWriterQueue<OscMsg> messageQueue_;
    bool master_;

    bool running_;
    std::thread writer_thread_;

    ChangeSource changeSource_;
};

} //namespace
