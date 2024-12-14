#pragma once
#include <complex>
#include <vector>

namespace std
{
#define PI 3.1415926535
	vector<complex<double>> dft(vector<std::complex<double>>& vec)
	{
		int N = vec.size();
		vector<complex<double>> temp_vec;
		temp_vec.assign(vec.begin(), vec.end());
		complex<double> temp;

		for (int m = 0; m < N; ++m)
		{
			temp = 0;
			for (int n = 0; n < N; ++n)
			{
				temp += vec[n] * polar(1.0, -1 * 2 * PI * n * m / N);//由欧拉公式可得
			}
			temp_vec[m] = temp;
		}
		return temp_vec;
	}

	vector<complex<double>> idft(vector<std::complex<double>>& vec)
	{
		int N = vec.size();
		vector<complex<double>> temp_vec;
		temp_vec.assign(vec.begin(), vec.end());
		complex<double> temp;

		for (int m = 0; m < N; ++m)
		{
			temp = 0;
			for (int n = 0; n < N; ++n)
			{
				temp += vec[n] * polar(1.0, 1 * 2 * PI * n * m / N);//由欧拉公式可得
			}
			temp_vec[m] = temp / (double)N;
		}
		return temp_vec;
	}
}