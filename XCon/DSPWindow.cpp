#include "DSPWindow.h"
#include "FlirecViewer.h"
#include "FlameUI.h"
#include "winuser.h"
#include "windows.h"
#include "FlameUIBasics.h"
#include "Chart.h"
#include "fft.h"
#include <string>
#include "Sharp.h"
#include "OFDMModulator.h"

using namespace FlameUI;
using namespace D2D1;

Frame* dspw = nullptr;
struct WAVHeader
{
	char riff[4] = { 'R','I','F','F' };
	INT32 chunkSize;
	char format[4] = { 'W','A','V','E' };
	char type[4] = { 'f','m','t',' ' };
	INT32 subChunkSize = 16;
	INT16 audioFormat = 1;
	INT16 channels = 1;
	INT32 sampleRate = 44100;
	INT32 byteRate = 44100 * 1 * 16 / 8;
	INT16 blockAlign = 16 * 1 / 8;
	INT16 bitsPerSampal = 16;
	char data[4] = { 'd','a','t','a' };
	INT32 dataSize;
};
vector<short> Symbolize(vector<char> data)
{
	vector<short> symbs;
	symbs.resize(data.size() * 2);
	for (int i = 0; i < data.size(); i++)
	{
		symbs[i * 2] = data[i] & 0xF;
		symbs[i * 2 + 1] = (data[i] >> 4) & 0xF;
	}
	return symbs;
}
vector<double> GenerateWave(vector<char>& data, double carrierFreq = 2048, double sampleRate = 44100, double symbPierod = 4, float subCarrierDiv = 128, int subCarrierCount = 8)
{
	vector<double> wave;
	vector<short> symbs = Symbolize(data);
	int curSymb = 0;
	double time = 0;
	double dTime = 1 / sampleRate;
	double ltime = 0;
	ltime = time;
	while (curSymb < symbs.size())
	{
		double value = 0;
		for (int n = 0; n < subCarrierCount; n++)
		{
			double s = sin(time * (carrierFreq + subCarrierDiv * n) * 2 * PI);
			double c = cos(time * (carrierFreq + subCarrierDiv * n) * 2 * PI);
			short symbol;
			if (curSymb + n < symbs.size())
				symbol = symbs[curSymb + n];
			else
				symbol = symbs[symbs.size() - 1];

			int sa = symbol & 0x3;
			int ca = (symbol >> 2) & 0x3;
			value += (s * sa * 0.25 + c * ca * 0.25 / sqrt(2)) / subCarrierCount;
		}
		time += dTime;
		if (time - ltime >= symbPierod / carrierFreq)
		{
			ltime = time;
			curSymb += subCarrierCount;
		}
		wave.push_back(value);
	}
	return wave;
}
void ShowDSPWindow()
{
	if (dspw)
	{
		FlashWindow(dspw->GetNative(), true);
		return;
	}
	dspw = new Frame({ 900,850 });
	dspw->Title(L"DSP");
	dspw->AddEventListener([](Message, WPARAM, LPARAM) {
		dspw = nullptr;
		}, FE_DESTROY);

	Panel* pnl;
	ScrollView* sv;
	sv = new ScrollView(dspw);
	sv->Coord(COORD_FILL, COORD_FILL);
	sv->Position({ 5, 32 });
	sv->Size({ 5,55 });
	sv->Content()->Layouter(new LinearPlacer(DIRECTION_VERTICAL, 8));
	pnl = sv->Content();
	pnl->Coord(COORD_FILL, COORD_POSITIVE);
	pnl->SizeMode(SIZE_MODE_NONE, SIZE_MODE_CHILDREN);
	pnl->Size({ 15,100 });

	Chart* td = new Chart(pnl);
	td->Coord(COORD_FILL, COORD_POSITIVE);
	td->Size({ 0, 350 });
	td->JoinSeries(L"Time Domain Raw", ColorF(ColorF::Red));
	td->JoinSeries(L"Freqshifted", ColorF(ColorF::Yellow, 0.7f));
	td->JoinSeries(L"Freqshift reverse", ColorF(ColorF::Green, 0.6f));
	td->beginIndex = 0;
	td->maxHistory = 1024;

	Chart* fd = new Chart(pnl);
	fd->Coord(COORD_FILL, COORD_POSITIVE);
	fd->Size({ 0, 350 });
	fd->JoinSeries(L"Frequency Invshifted", ColorF(ColorF::Green));
	fd->JoinSeries(L"Frequency Raw", ColorF(ColorF::Red));
	fd->JoinSeries(L"Frequency Shifted", ColorF(ColorF::Yellow));
	fd->beginIndex = 0;
	fd->maxHistory = 512;

	string sdata = "abcdefghijklmnopqrstuvwxyzstd::vector<std::complex<double>> tdsig;\Chart* fd = new Chart(pnl);\
		fd->Coord(COORD_FILL, COORD_POSITIVE);\
		fd->Size({ 0, 350 });\
		fd->JoinSeries(L\"Phase\", ColorF(ColorF::Green));\
		fd->JoinSeries(L\"Frequency\", ColorF(ColorF::Red));\
		fd->beginIndex = 0;\
		fd->maxHistory = 512; \
vector<char> data;\
data.assign(sdata.begin(), sdata.end());\
	auto wav = GenerateWave16QAM(data);\
	char* wavfil = (char*)malloc(sizeof(WAVHeader) + wav.size() * sizeof(short));\
	WAVHeader wavh;\
	wavh.chunkSize = sizeof(WAVHeader) + wav.size() * sizeof(short) - 8;\
	wavh.dataSize = wav.size() * sizeof(short);\
	memcpy(wavfil, &wavh, sizeof(wavh));\
	for (auto i = 0; i < wav.size(); i++)\
	{\
		short v = 32768 * wav[i] - 1;\
		*((short*)((wavfil + sizeof(wavh))) + i) = v;\
	}\
	Sharp::FileStream fs(\"./modulated.wav\", Sharp::FileMode::CreateNew, Sharp::FileAccess::ReadWrite);\
	fs.Write(wavfil, wavh.chunkSize + 8);\
	fs.Flush();\
	fs.Close();\
	srand((unsigned int)&tdsig);\
	double s[] = { 0.25,0.75,0,0.5,0.75,1,0.5,0,1,0.25,1 };\
	double c[] = { 0.5,0.5,0.25,0.5,1,1,0.75,0.25,0.5,0.75,1 }; ";
	vector<char> data;
	data.assign(sdata.begin(), sdata.end());
	OFDMModulator ofdmm(1024, 128, 8, 44100, 2);
	auto llf = ofdmm.GetOutputLength(data.size());
	short* vs = new short[ofdmm.GetOutputLength(data.size())];
	auto rrlen = ofdmm.ProcessDAC(&data[0], data.size(), vs);
	char* wavfil = (char*)malloc(sizeof(WAVHeader) + rrlen * sizeof(short));
	WAVHeader wavh;
	wavh.chunkSize = sizeof(WAVHeader) + rrlen * sizeof(short) - 8;
	wavh.dataSize = rrlen * sizeof(short);
	Sharp::FileStream fs("./modulated.wav", Sharp::FileMode::CreateNew, Sharp::FileAccess::ReadWrite);
	fs.Write((char*)&wavh, sizeof(wavh));
	fs.Write((char*)vs, wavh.chunkSize);
	fs.Flush();
	fs.Close();

	std::vector<std::complex<double>> tdsig;
	std::vector<std::complex<double>> tdsigfs;
	std::vector<std::complex<double>> fdsig;
	std::vector<std::complex<double>> fdsigfs;
	std::vector<std::complex<double>> tdsigfsrev;
	tdsig.resize(1024);
	tdsigfs.resize(1024);

	srand((unsigned int)&tdsig);
	double s[] = { 0.25,0.75,0,0.5,0.75,1,0.5,0,1,0.25,1 };
	double c[] = { 0.5,0.5,0.25,0.5,1,1,0.75,0.25,0.5,0.75,1 };
	for (int i = 0; i < 1024; i++)
	{
		tdsig[i] = ((rand() % 1000) / 1000.0 - 0.5) * 0;//noise
		tdsig[i] += sin(0.1 * i) *s[i / 100];
		tdsig[i] += cos(0.1 * i) *c[i / 100];
		tdsigfs[i] = (tdsig[i] + 0.) * sin(i * 2);
		//tdsig[i] *=  sin(i*2);
	}
	fdsig = dft(tdsig);
	fdsigfs = dft(tdsigfs);
	for (auto& i : tdsig)
	{
		td->JoinValue(0, (float)i.real(), false);
	}
	for (auto& i : tdsigfs)
	{
		td->JoinValue(1, (float)i.real(), false);
	}

	for (int n = 0; n < 1024; n++)
	{
		if (n > 326 && 1023 - n > 326)
			fdsigfs[n] = { 0,0 };
		else
			fdsigfs[n] *= {2, 2};
	}
	tdsigfsrev = idft(fdsigfs);
	int di = 0;
	for (auto& i : tdsigfsrev)
	{
		td->JoinValue(2, (float)i.real() , false);
		di++;
	}
	fdsigfs = dft(tdsigfsrev);
	for (int n = 0; n < 512; n++)
	{
		auto& i = fdsig[n];
		auto& ifs = fdsigfs[n];
		fd->JoinValue(1, (float)sqrt(i.real() * i.real() + i.imag() * i.imag()) / 512, false);
		//fd->JoinValue(0, (float)atan2(i.imag(), i.real()) / PI, false);
		fd->JoinValue(2, (float)sqrt(ifs.real() * ifs.real() + ifs.imag() * ifs.imag()) / 512, false);
	}
	td->GenerateCurve(0);
	//td->GenerateCurve(1);
	td->GenerateCurve(2);
	fd->GenerateCurve(0);
	fd->GenerateCurve(1);
	fd->GenerateCurve(2);
	dspw->Show();

}