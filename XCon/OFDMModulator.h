#pragma once
enum class ModulateMode
{
	ASK,
	PSK,
	FSK,
	APSK,
	FPSK,
	QAM16,
	QAM64,
	QAM256,
	QAM1024
};
class OFDMModulator
{
private:
	double mainFreq;
	double subFreqDiv;
	int subFreqCount;
	ModulateMode mode;
	int sampleRate;
	double symbolPeriod;

	double time;
	double lastSymbolTime;

public:
	OFDMModulator(double mfreq = 1024, double subFreqDiv = 128, int subFreqCount = 1, int sampleRate = 44100, double symbolPeriod = 2, ModulateMode mode = ModulateMode::QAM16);
	int GetOutputLength(int dataSize);
	int Process(char* data, int len, double* out);
	int ProcessDAC(char* data, int len, short* out);
};

