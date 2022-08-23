#ifndef ACTIVESPEAKERMULTIPLEXER_H
#define ACTIVESPEAKERMULTIPLEXER_H
#include "config.h"
#include "rtp/RTPIncomingMediaStream.h"
#include "rtp/RTPStreamTransponder.h"
#include "TimeService.h"


class ActiveSpeakerMultiplexer :
	public RTPIncomingMediaStream::Listener
{
private:
	struct Source
	{
		uint32_t id;
		RTPIncomingMediaStream* incoming = nullptr;
		uint64_t score = 0;
		uint64_t ts = 0;
		std::vector<RTPPacket::shared> packets;

		Source(uint32_t id, RTPIncomingMediaStream* incoming) :
			id(id),
			incoming(incoming)
		{
		}

		bool operator<(const Source& b)
		{
			return score < b.score;
		}
	};

	struct Destination
	{
		uint32_t id;
		RTPStreamTransponder* transponder = nullptr;
		uint64_t ts = 0;
		uint64_t sourceId = 0;

		Destination(uint32_t id, RTPStreamTransponder* transponder) :
			id(id),
			transponder(transponder)
		{
		}
	};
public: 
	class Listener
	{
	public:
		virtual ~Listener() = default;
		virtual void onActiveSpeakerChanged(uint32_t speakerId, uint32_t multiplexdId) = 0;
		virtual void onActiveSpeakerRemoved(uint32_t multiplexdId) = 0;
	};
public:
	ActiveSpeakerMultiplexer(TimeService& timeService, Listener* listener);
	virtual ~ActiveSpeakerMultiplexer();

	void AddIncomingSourceGroup(RTPIncomingMediaStream* incoming, uint32_t id);
	void RemoveIncomingSourceGroup(RTPIncomingMediaStream* incoming);

	void AddRTPStreamTransponder(RTPStreamTransponder* transpoder, uint32_t id);
	void RemoveRTPStreamTransponder(RTPStreamTransponder* transpoder);
	
	virtual void onRTP(RTPIncomingMediaStream* stream, const RTPPacket::shared& packet) override;
	virtual void onRTP(RTPIncomingMediaStream* stream, const std::vector<RTPPacket::shared>& packets) override;
	virtual void onBye(RTPIncomingMediaStream* stream) override;
	virtual void onEnded(RTPIncomingMediaStream* stream) override;

	void SetMaxAccumulatedScore(uint64_t maxAcummulatedScore)	{ this->maxAcummulatedScore = maxAcummulatedScore;	}
	void SetNoiseGatingThreshold(uint8_t noiseGatingThreshold)	{ this->noiseGatingThreshold = noiseGatingThreshold;	}
	void SetMinActivationScore(uint32_t minActivationScore)		{ this->minActivationScore = minActivationScore;	}

	void Stop();
private:
	void Process(uint64_t now);
private:
	TimeService& timeService;
	Timer::shared timer;
	Listener* listener;

	uint64_t last = 0;
	uint64_t maxAcummulatedScore = 2500;
	uint8_t noiseGatingThreshold = 127;
	uint64_t minActivationScore = 0;

	std::map<RTPIncomingMediaStream*, Source> sources;
	std::map<RTPStreamTransponder*, Destination> destinations;
};

#endif /* ACTIVESPEAKERMULTIPLEXER_H */