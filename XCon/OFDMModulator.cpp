#include "OFDMModulator.h"
#include <vector>
#define PI 3.1415926535
using namespace std;
vector<short> Symbolize(char* data, int len)
{
	vector<short> symbs;
	symbs.resize(len * 2);
	for (int i = 0; i < len; i++)
	{
		symbs[i * 2] = data[i] & 0xF;
		symbs[i * 2 + 1] = (data[i] >> 4) & 0xF;
	}
	return symbs;
}
int OFDMModulator::GetOutputLength(int dataSize)
{
	return symbolPeriod / (mainFreq)*sampleRate * dataSize * 2 * 1.2;
}
OFDMModulator::OFDMModulator(double mfreq, double subFreqDiv, int subFreqCount, int sampleRate, double symbolPeriod, ModulateMode mode)
{
	this->mainFreq = mfreq;
	this->subFreqCount = subFreqCount;
	this->subFreqDiv = subFreqDiv;
	this->sampleRate = sampleRate;
	this->symbolPeriod = symbolPeriod;
	this->mode = mode;
	lastSymbolTime = 0;
	time = 0;
}
int OFDMModulator::Process(char* data, int len, double* out)
{
	auto symbols = Symbolize(data, len);
	int curSymb = 0;
	double dT = 1.0 / sampleRate;
	int sampleCount = 0;
	while (curSymb < symbols.size())
	{
		double value = 0;
		for (int n = 0; n < subFreqCount; n++)
		{
			double s = sin(time * (mainFreq + subFreqDiv * n) * 2 * PI);
			double c = cos(time * (mainFreq + subFreqDiv * n) * 2 * PI);
			short symbol;
			if (curSymb + n < symbols.size())
				symbol = symbols[curSymb + n];
			else
				symbol = symbols[symbols.size() - 1];

			int sa = symbol & 0x3;
			int ca = (symbol >> 2) & 0x3;
			value += (s * sa * 0.25 + c * ca * 0.25 / sqrt(2)) / subFreqCount;
		}
		if (time - lastSymbolTime >= symbolPeriod / mainFreq)
		{
			lastSymbolTime = time;
			curSymb += subFreqCount;
		}
		time += dT;
		out[sampleCount] = value;
		sampleCount++;
	}
	return sampleCount;
}
int OFDMModulator::ProcessDAC(char* data, int len, short* out)
{
	int vlen = GetOutputLength(len);
	double* vals = new double[vlen];
	int rlen = Process(data, len, vals);
	for (int i = 0; i < vlen; i++)
	{
		out[i] = 32767 * vals[i];
	}
	delete[] vals;
	return rlen;
}
