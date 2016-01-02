#pragma once
#include "stdafx.h"
#include "BaseApuChannel.h"

class ApuLengthCounter : public BaseApuChannel<15>
{
private:
	const vector<uint8_t> _lcLookupTable = { { 10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14, 12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30 } };
	bool _enabled = false;
	bool _newHaltValue;
	static bool _needToRun;

protected:
	bool _lengthCounterHalt;
	uint8_t _lengthCounter;
	uint8_t _lengthCounterReloadValue;
	uint8_t _lengthCounterPreviousValue;

	void InitializeLengthCounter(bool haltFlag)
	{
		SetRunFlag();
		_newHaltValue = haltFlag;
	}

	void LoadLengthCounter(uint8_t value)
	{
		if(_enabled) {
			_lengthCounterReloadValue = _lcLookupTable[value];
			_lengthCounterPreviousValue = _lengthCounter;
			SetRunFlag();
		}
	}

	void SetRunFlag()
	{
		ApuLengthCounter::_needToRun = true;
	}

public:
	ApuLengthCounter(AudioChannel channel, Blip_Buffer* buffer) : BaseApuChannel(channel, buffer)
	{
	}
	
	static bool NeedToRun()
	{
		return ApuLengthCounter::_needToRun;
	}

	virtual void Reset(bool softReset)
	{
		BaseApuChannel::Reset(softReset);

		if(softReset) {
			_enabled = false;
			if(GetChannel() != AudioChannel::Triangle) {
				//"At reset, length counters should be enabled, triangle unaffected"
				_lengthCounterHalt = false;
				_lengthCounter = 0;
				_newHaltValue = false;
			}
		} else {
			_enabled = false;
			_lengthCounterHalt = false;
			_lengthCounter = 0;
			_newHaltValue = false;
		}

		ApuLengthCounter::_needToRun = false;
	}

	virtual void StreamState(bool saving)
	{
		BaseApuChannel::StreamState(saving);

		Stream<bool>(_enabled);
		Stream<bool>(_lengthCounterHalt);
		Stream<bool>(_newHaltValue);
		Stream<uint8_t>(_lengthCounter);
	}

	bool GetStatus()
	{
		return _lengthCounter > 0;
	}

	virtual void Run(uint32_t targetCycle)
	{
		ApuLengthCounter::_needToRun = false;
		_lengthCounterHalt = _newHaltValue;
		BaseApuChannel::Run(targetCycle);
	}

	void ReloadCounter()
	{
		if(_lengthCounterReloadValue) {
			if(_lengthCounter == _lengthCounterPreviousValue) {
				_lengthCounter = _lengthCounterReloadValue;
			}
			_lengthCounterReloadValue = 0;
		}
	}

	void TickLengthCounter()
	{
		if(_lengthCounter > 0 && !_lengthCounterHalt) {
			_lengthCounter--;
		}
	}

	void SetEnabled(bool enabled)
	{
		if(!enabled) {
			_lengthCounter = 0;
		}
		_enabled = enabled;
	}
};
