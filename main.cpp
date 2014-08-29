#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <locale>
#include <algorithm>

using namespace std;

typedef pair<wstring, int> PAIR;
int cmp(const PAIR &x, const PAIR &y);
bool isWord(wstring s);
double getEntropy(map<wstring, int> neighMap);
void getPhrases(string path, int freqLimit, double closeLimit, double freeLimit);

int main()
{
	// set the global locale as chinese UTF-8
	locale loc("zh_CN.UTF-8");
	locale::global(loc);

	getPhrases("红楼梦-UTF-8.txt", 20, 45.0, 3.0);

	return 0;
}

int cmp(const PAIR &x, const PAIR &y) 
{
	return x.second > y.second;
}

// judge the input string is a word or not
bool isWord(wstring s)
{
	if (s.find_first_of(L"，？：！；《》。、\"'.（）-―") != string::npos)
		return false;

	return true;
}

// get the entropy of a given map
double getEntropy(map<wstring, int> neighMap)
{
	double total = 0.0;
	double entropy = 0.0;
	vector<double> probVec;
	for (map<wstring, int>::const_iterator it = neighMap.begin(); it != neighMap.end(); ++it)
		total += it->second;
	for (map<wstring, int>::const_iterator it = neighMap.begin(); it != neighMap.end(); ++it)
		probVec.push_back(it->second / total);
	for (int i = 0; i < probVec.size(); i++)
		entropy += probVec[i] * log(probVec[i]) / log(2);
	return -1 * entropy;
}

// get the phrases of a given file
void getPhrases(string path, int freqLimit, double closeLimit, double freeLimit)
{
	map<wstring, int> phraseMap;

	wifstream file(path.c_str());
	if (!file.is_open()) {
		cerr << "Error: can't open the input file!" << endl;
		return;
	}

	wstring line;
	map<wstring, int> freqMap;
	map<int, double> wordNum;
	wordNum[1] = 0.0;
	wordNum[2] = 0.0;
	wordNum[3] = 0.0;
	wordNum[4] = 0.0;
	while (getline(file, line)) {
		if (line.size() == 0)
			continue;
		for (int i = 0; i < line.size(); i++) {
			wordNum[1] = wordNum[1] + 1;
        	        if (freqMap.find(line.substr(i, 1)) != freqMap.end())
        	                freqMap[line.substr(i, 1)] += 1;
        	        else
        	                freqMap[line.substr(i, 1)] = 1;
        	}
		for (int i = 0; i < line.size() - 1; i++) {
			wordNum[2] = wordNum[2] + 1;
			if (freqMap.find(line.substr(i, 2)) != freqMap.end())
				freqMap[line.substr(i, 2)] += 1;
			else
				freqMap[line.substr(i, 2)] = 1;
		}
		for (int i = 0; i < line.size() - 2; i++) {
			wordNum[3] = wordNum[3] + 1;
			if (freqMap.find(line.substr(i, 3)) != freqMap.end())
				freqMap[line.substr(i, 3)] += 1;
			else
				freqMap[line.substr(i, 3)] = 1;
		}
		for (int i = 0; i < line.size() - 3; i++) {
			wordNum[4] = wordNum[4] + 1;
			if (freqMap.find(line.substr(i, 4)) != freqMap.end())
        	                freqMap[line.substr(i, 4)] += 1;
        	        else
        	                freqMap[line.substr(i, 4)] = 1;
		}
	}
	file.close();
	// filter with word frequency
	for (map<wstring, int>::const_iterator it = freqMap.begin(); it != freqMap.end(); ++it) {
		if (it->first.size() > 1 && isWord(it->first) && it->second >= freqLimit) {
			phraseMap[it->first] = it->second;
		}	
	} 
	// filter with word close feature
	for (map<wstring, int>::iterator it = phraseMap.begin(); it != phraseMap.end();) {
		int len = it->first.size();
		double freMax = 0.0;
		for (int i = 1; i < len; i++) {
			double freCur = freqMap[it->first.substr(0, i)] / wordNum[i] * freqMap[it->first.substr(i)] / wordNum[len - i];
			if (freCur > freMax)
				freMax = freCur;
		}
		double freTotal = freqMap[it->first] / wordNum[len];
		double close = freTotal / freMax;
		if (close < closeLimit)
			phraseMap.erase(it++);
		else
			it++;
	}
	// filter with word free feature
	for (map<wstring, int>::iterator it = phraseMap.begin(); it != phraseMap.end();) {
		map<wstring, int> leftMap;
		map<wstring, int> rightMap;
		//file.seekg(0, file.beg);
		wifstream file(path.c_str());
        	if (!file.is_open()) {
                	cerr << "Error: can't open the input file!" << endl;
                	return;
        	}
		wstring line;
		while (getline(file, line)) {
			if (line.size() == 0 || line.find(it->first) == string::npos)
				continue;
			string::size_type found = line.find(it->first);
			if (found != 0) {
				if(leftMap.find(line.substr(found - 1, 1)) != leftMap.end())
					leftMap[line.substr(found - 1, 1)] += 1;
				else
					leftMap[line.substr(found - 1, 1)] = 1;
			}
			if (found + it->first.size() < line.size()) {
				if(rightMap.find(line.substr(found + it->first.size(), 1)) != rightMap.end())
                                        rightMap[line.substr(found + it->first.size(), 1)] += 1;
                                else
                                        rightMap[line.substr(found + it->first.size(), 1)] = 1;
			}
			while ((found = line.find(it->first, found + 1)) != string::npos) {
				if (found != 0) {
                                	if(leftMap.find(line.substr(found - 1, 1)) != leftMap.end())
                                        	leftMap[line.substr(found - 1, 1)] += 1;
                                	else
                                        	leftMap[line.substr(found - 1, 1)] = 1;
                        	}
                        	if (found + it->first.size() < line.size()) {
                                	if(rightMap.find(line.substr(found + it->first.size(), 1)) != rightMap.end())
                                        	rightMap[line.substr(found + it->first.size(), 1)] += 1;
                                	else
                                        	rightMap[line.substr(found + it->first.size(), 1)] = 1;
                        	}

			}
		}
		file.close();
		double entropy = min(getEntropy(leftMap), getEntropy(rightMap));
		if (entropy < freeLimit)
			phraseMap.erase(it++);
		else
			it++;
	}
	// sort the phrase with frequency
	vector<PAIR> phrases;
	for (map<wstring, int>::const_iterator it = phraseMap.begin(); it != phraseMap.end(); ++it) {
		phrases.push_back(make_pair(it->first, it->second));
	}
	sort(phrases.begin(), phrases.end(), cmp);
	for (int i = 0; i < phrases.size(); i++)
		wcout << phrases[i].first << " ";
	wcout << endl;
}
