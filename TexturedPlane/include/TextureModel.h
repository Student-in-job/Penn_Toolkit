#pragma once

#include "AccSynthHashMatrix.h"
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>
#include <vector>
#include "helpers.h"
#include "pugixml.hpp"

class TextureModel
{
private:
	AccSynthHashMatrix matrix;
	boost::mt19937 rng;
	
	// pAudio variables
	std::vector <float> outputHist;
	std::vector <float> excitationHist;
	std::string baseDirectory;

	// Private Methods
	AccSynthHashMatrix generateHashMatrix(std::string directory);

public:
	// Constructors
	TextureModel(std::string directory);

	// Public members
	double GetVibrations();
	char* GetModelName(int model);
	void setTextureModel(int model);
	int getTextureModel();
	std::string GetModelImage(int model);
	std::string GetCurrentModelImage();
	void Interpolate(double force, double velocity);
};