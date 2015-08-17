#ifndef INCLUDE_SCOPEVIS_H
#define INCLUDE_SCOPEVIS_H

#include <boost/circular_buffer.hpp>
#include "dsp/samplesink.h"
#include "util/export.h"
#include "util/message.h"

class GLScope;
class MessageQueue;

class SDRANGELOVE_API ScopeVis : public SampleSink {
public:
	enum TriggerChannel {
		TriggerFreeRun,
		TriggerChannelI,
		TriggerChannelQ,
		TriggerMagLin,
		TriggerMagDb,
		TriggerPhase
	};

	static const uint m_traceChunkSize;

	ScopeVis(GLScope* glScope = NULL);
	virtual ~ScopeVis();

	void configure(MessageQueue* msgQueue, 
        TriggerChannel triggerChannel, 
        Real triggerLevel, 
        bool triggerPositiveEdge, 
        bool triggerBothEdges,
        uint triggerPre,
        uint triggerDelay, 
        uint traceSize);
	void setOneShot(bool oneShot);

	virtual void feed(SampleVector::const_iterator begin, SampleVector::const_iterator end, bool positiveOnly);
	virtual void start();
	virtual void stop();
	virtual bool handleMessage(const Message& message);

	void setSampleRate(int sampleRate);
	int getSampleRate() const { return m_sampleRate; }
	SampleVector::const_iterator getTriggerPoint() const { return m_triggerPoint; }

private:
	class MsgConfigureScopeVis : public Message {
		MESSAGE_CLASS_DECLARATION

	public:
		int getTriggerChannel() const { return m_triggerChannel; }
		Real getTriggerLevel() const { return m_triggerLevel; }
		Real getTriggerPositiveEdge() const { return m_triggerPositiveEdge; }
		Real getTriggerBothEdges() const { return m_triggerBothEdges; }
        uint getTriggerPre() const { return m_triggerPre; }
        uint getTriggerDelay() const { return m_triggerDelay; }
        uint getTraceSize() const { return m_traceSize; }

		static MsgConfigureScopeVis* create(int triggerChannel, 
            Real triggerLevel, 
            bool triggerPositiveEdge, 
            bool triggerBothEdges,
            uint triggerPre,
            uint triggerDelay, 
            uint traceSize)
		{
			return new MsgConfigureScopeVis(triggerChannel,
					triggerLevel,
					triggerPositiveEdge,
					triggerBothEdges,
					triggerPre,
					triggerDelay,
					traceSize);
		}

	private:
		int m_triggerChannel;
		Real m_triggerLevel;
		bool m_triggerPositiveEdge;
		bool m_triggerBothEdges;
        uint m_triggerPre;
        uint m_triggerDelay;
        uint m_traceSize;

		MsgConfigureScopeVis(int triggerChannel, 
                Real triggerLevel, 
                bool triggerPositiveEdge, 
                bool triggerBothEdges,
                uint triggerPre,
                uint triggerDelay,
                uint traceSize) :
			Message(),
			m_triggerChannel(triggerChannel),
			m_triggerLevel(triggerLevel),
			m_triggerPositiveEdge(triggerPositiveEdge),
			m_triggerBothEdges(triggerBothEdges),
            m_triggerPre(triggerPre),
            m_triggerDelay(triggerDelay),
            m_traceSize(traceSize)
		{ }
	};

    /**
     *  TriggerState:
     * 
     *            send a                       Trigger condition                                               +--------------------+
     *          dummy trace                    - Immediate                              m_triggerOneShot       |                    |
     *  Config -------------> Untriggered ----------------------------------> Triggered ----------------> WaitForReset              |
     *                         ^ ^ |                                             ^  |                          |  ^                 |
     *                         | | |           - Delayed         Delay expired   |  |                          |  | setOneShot(true)|
     *                         | | +---------------------> Delay ----------------+  |                          |  +-----------------+
     *                         | |                                !m_triggerOneShot |                          |
     *                         | +--------------------------------------------------+        setOneShot(false) |
     *                         +-------------------------------------------------------------------------------+
     */
	enum TriggerState {
		Untriggered,  //!< Search for trigger
        Config,       //!< New configuration has just been received
		Triggered,    //!< Trigger was kicked off
		WaitForReset, //!< Wait for release from GUI
        Delay         //!< Trigger delay engaged
	};

	GLScope* m_glScope;
	std::vector<Complex> m_trace;                //!< Raw trace to be used by GLScope
	boost::circular_buffer<Complex> m_traceback; //!< FIFO for samples prior to triggering point to support pre-trigger (when in triggered mode)
    uint m_tracebackCount;                       //!< Count of samples stored into trace memory since triggering is active up to trace memory size
	uint m_fill;
	TriggerState m_triggerState;
	TriggerChannel m_triggerChannel;
	Real m_triggerLevel;
	bool m_triggerPositiveEdge;
	bool m_triggerBothEdges;
	bool m_prevTrigger;
    uint m_triggerPre; //!< Pre-trigger delay in number of samples
	bool m_triggerOneShot;
	bool m_armed;
    uint m_triggerDelay;      //!< Trigger delay in number of trace sizes
    uint m_triggerDelayCount; //!< trace sizes delay counter
	int m_sampleRate;
	SampleVector::const_iterator m_triggerPoint;

	bool triggerCondition(SampleVector::const_iterator& it);
};

#endif // INCLUDE_SCOPEVIS_H
