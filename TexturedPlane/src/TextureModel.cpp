#include "TextureModel.h"
#include "shared.h"
#include "sharedInit.h"

using namespace std;

TextureModel::TextureModel(std::string directory)
{
	textNum = -1;
	AccSynthHashMatrix thematrix = this->generateHashMatrix(directory);
    this->matrix = thematrix;
}

AccSynthHashMatrix TextureModel::generateHashMatrix(std::string directory)
{
	AccSynthHashMatrix tempmatrix(NUM_TEX);

	float allSpeeds[200];    //allocate memory for arrays to store lists of speeds and forces
	float allForces[200];
	int DT1[200];            //allocate memory for arrays to store lists of Delaunay triangulation vertices
	int DT2[200];
	int DT3[200];
	float modLSF[MAX_COEFF]; //allocate memory for arrays to store AR and MA LSFs
	float modMALSF[MAX_MACOEFF];

	int numMod;
	int numTri;
	float maxSpeed;
	float maxForce;
	float mu;

	int numCoeff;
	int numMACoeff;
	float variance;
	float gain;
	float modSpeed;
	float modForce;

	string imFilename;

	string::size_type pos1 = directory.find_last_of("\\/");
	string exeFilepath = directory.substr(0, pos1);
	string::size_type pos2 = exeFilepath.find_last_of("\\/");
	
	string pplatformPath = exeFilepath.substr(0, pos2);
	string::size_type pos3 = pplatformPath.find_last_of("\\/");

	this->baseDirectory = pplatformPath.substr(0, pos3);

	//Find filepath of XML model files
	string baseFilename = this->baseDirectory + "\\build\\XML\\Models_";
	string myFilename;

	/********* Fill array with known texture names ***********/
	// divide textures by group

	//Paper
	strcpy(texArray[0], "Book");
	strcpy(texArray[1], "Bubble Envelope");
	strcpy(texArray[2], "Cardboard");
	strcpy(texArray[3], "Coffee Filter");
	strcpy(texArray[4], "Dot Paper");
	strcpy(texArray[5], "Folder");
	strcpy(texArray[6], "Gift Box");
	strcpy(texArray[7], "Glitter Paper");
	strcpy(texArray[8], "Greeting Card");
	strcpy(texArray[9], "Masking Tape");
	strcpy(texArray[10], "Paper Bag");
	strcpy(texArray[11], "Paper Plate 1");
	strcpy(texArray[12], "Paper Plate 2");
	strcpy(texArray[13], "Playing Card");
	strcpy(texArray[14], "Resume Paper");
	strcpy(texArray[15], "Sandpaper 100");
	strcpy(texArray[16], "Sandpaper 220");
	strcpy(texArray[17], "Sandpaper 320");
	strcpy(texArray[18], "Sandpaper Aluminum Oxide");
	strcpy(texArray[19], "Textured Paper");
	strcpy(texArray[20], "Tissue Paper");
	strcpy(texArray[21], "Wax Paper");

	//Plastic
	strcpy(texArray[22], "ABS Plastic");
	strcpy(texArray[23], "Binder");
	strcpy(texArray[24], "Candle");
	strcpy(texArray[25], "File Portfolio");
	strcpy(texArray[26], "Frosted Acrylic");
	strcpy(texArray[27], "Nitrile Glove");
	strcpy(texArray[28], "Plastic Mesh 1");
	strcpy(texArray[29], "Plastic Mesh 2");
	strcpy(texArray[30], "Tarp");
	strcpy(texArray[31], "Wavy Acrylic");

	//Fabric
	strcpy(texArray[32], "Athletic Shirt");
	strcpy(texArray[33], "Blanket");
	strcpy(texArray[34], "CD Sleeve");
	strcpy(texArray[35], "Canvas 1");
	strcpy(texArray[36], "Canvas 2");
	strcpy(texArray[37], "Canvas 3");
	strcpy(texArray[38], "Cotton");
	strcpy(texArray[39], "Denim");
	strcpy(texArray[40], "Felt");
	strcpy(texArray[41], "Flannel");
	strcpy(texArray[42], "Fleece");
	strcpy(texArray[43], "Leather 1 Back");
	strcpy(texArray[44], "Leather 1 Front");
	strcpy(texArray[45], "Leather 2 Back");
	strcpy(texArray[46], "Leather 2 Front");
	strcpy(texArray[47], "Microfiber Cloth");
	strcpy(texArray[48], "Nylon Bag");
	strcpy(texArray[49], "Nylon Mesh");
	strcpy(texArray[50], "Pleather");
	strcpy(texArray[51], "Portfolio Cover");
	strcpy(texArray[52], "Silk 1");
	strcpy(texArray[53], "Silk 2");
	strcpy(texArray[54], "Textured Cloth");
	strcpy(texArray[55], "Towel");
	strcpy(texArray[56], "Velcro Hooks");
	strcpy(texArray[57], "Velcro Loops");
	strcpy(texArray[58], "Velvet");
	strcpy(texArray[59], "Vinyl 1");
	strcpy(texArray[60], "Vinyl 2");
	strcpy(texArray[61], "Whiteboard Eraser");

	//Tile
	strcpy(texArray[62], "Floortile 1");
	strcpy(texArray[63], "Floortile 2");
	strcpy(texArray[64], "Floortile 3");
	strcpy(texArray[65], "Floortile 4");
	strcpy(texArray[66], "Floortile 5");
	strcpy(texArray[67], "Floortile 6");
	strcpy(texArray[68], "Floortile 7");

	//Carpet
	strcpy(texArray[69], "Artificial Grass");
	strcpy(texArray[70], "Carpet 1");
	strcpy(texArray[71], "Carpet 2");
	strcpy(texArray[72], "Carpet 3");
	strcpy(texArray[73], "Carpet 4");

	//Foam
	strcpy(texArray[74], "EPDM Foam");
	strcpy(texArray[75], "Pink Foam");
	strcpy(texArray[76], "Polyethylene Foam");
	strcpy(texArray[77], "Scouring Pad");
	strcpy(texArray[78], "Styrofoam");
	strcpy(texArray[79], "Textured Rubber");

	//Metal
	strcpy(texArray[80], "Aluminum Foil");
	strcpy(texArray[81], "Aluminum");
	strcpy(texArray[82], "Metal Mesh");
	strcpy(texArray[83], "Metal Shelving");
	strcpy(texArray[84], "Textured Metal");
	strcpy(texArray[85], "Whiteboard");

	//Stone
	strcpy(texArray[86], "Brick 1");
	strcpy(texArray[87], "Brick 2");
	strcpy(texArray[88], "Ceramic");
	strcpy(texArray[89], "Painted Brick");
	strcpy(texArray[90], "Stone Tile 1");
	strcpy(texArray[91], "Stone Tile 2");
	strcpy(texArray[92], "Terra Cotta");

	//Carbon Fiber
	strcpy(texArray[93], "Carbon Fiber");
	strcpy(texArray[94], "Resin Carbon Fiber");

	//Wood
	strcpy(texArray[95], "Cork");
	strcpy(texArray[96], "MDF");
	strcpy(texArray[97], "Painted Wood");
	strcpy(texArray[98], "Stained Wood");
	strcpy(texArray[99], "Wood");

	// Loop through all textures
	for (int numSurf = 0; numSurf < NUM_TEX; numSurf++)
	{
		pugi::xml_document doc;

		myFilename = baseFilename + texArray[numSurf] + ".xml"; // get full filename of model file

		pugi::xml_parse_result result = doc.load_file(myFilename.c_str()); // load model file

		pugi::xml_node modelSet = doc.child("modelSet");

		mu = atof(modelSet.child_value("mu")); //friction coefficient

		stringstream ss;
		ss << modelSet.child_value("renderPicture"); //picture for display in rendering
		imFilename = ss.str(); //convert image name to string		
		strcpy(imArray[numSurf], imFilename.c_str());

		numMod = atoi(modelSet.child_value("numMod")); //number of models

		numTri = atoi(modelSet.child_value("numTri")); //number of triangles in Delaunay triangulation

		maxSpeed = atof(modelSet.child_value("maxSpeed")); //maximum modeled speed

		maxForce = atof(modelSet.child_value("maxForce")); //maximum modeled force

		//list of all model speeds
		int count = 0;
		for (pugi::xml_node speedList = modelSet.child("speedList").child("value"); speedList; speedList = speedList.next_sibling("value"))
		{
			allSpeeds[count] = atof(speedList.child_value());
			count++;
		}

		//list of all model forces
		count = 0;
		for (pugi::xml_node forceList = modelSet.child("forceList").child("value"); forceList; forceList = forceList.next_sibling("value"))
		{
			allForces[count] = atof(forceList.child_value());
			count++;
		}

		//list of all triangles in Delaunay triangulation
		count = 0;
		for (pugi::xml_node tri = modelSet.child("tri"); tri; tri = tri.next_sibling("tri"))
		{
			pugi::xml_node triChild = tri.child("value");
			DT1[count] = atoi(triChild.child_value());
			DT2[count] = atoi(triChild.next_sibling("value").child_value());
			DT3[count] = atoi(triChild.next_sibling("value").next_sibling("value").child_value());
			count++;
		}


		numCoeff = atoi(modelSet.child_value("numARCoeff")); //number of AR coefficients

		numMACoeff = atoi(modelSet.child_value("numMACoeff")); //number of MA coefficients
		if (numMACoeff == 0)
			isARMA = false;
		else
			isARMA = true;

		//create a hash table for this surface
		tempmatrix.AddTable(numSurf, numMod, allSpeeds, allForces);

		//for each model in the file
		for (pugi::xml_node model = modelSet.child("model"); model; model = model.next_sibling("model"))
		{
			//read all AR LSFs
			count = 0;
			pugi::xml_node ARlsf = model.child("ARlsf");
			for (pugi::xml_node ARval = ARlsf.child("value"); ARval; ARval = ARval.next_sibling("value"))
			{
				modLSF[count] = atof(ARval.child_value());
				count++;
			}

			//read all MA LSFs (if needed)
			if (isARMA)
			{
				count = 0;
				pugi::xml_node MAlsf = model.child("MAlsf");
				for (pugi::xml_node MAval = MAlsf.child("value"); MAval; MAval = MAval.next_sibling("value"))
				{
					modMALSF[count] = atof(MAval.child_value());
					count++;
				}
				gain = atof(model.child("gain").child_value());
			}

			variance = atof(model.child("var").child_value()); //model variance
			modSpeed = atof(model.child("speedMod").child_value()); //model speed
			modForce = atof(model.child("forceMod").child_value()); //model force

			//create a hash entry for each model
			if (isARMA) {
				AccSynthHashEntry HashEntry(numSurf, modForce, modSpeed, modLSF, modMALSF, variance, gain, numCoeff, numMACoeff, numTri, numMod, DT1, DT2, DT3, maxSpeed, maxForce, mu);
				tempmatrix.AddEntry(HashEntry, numMod, allSpeeds, allForces);
			}
			else {
				AccSynthHashEntry HashEntry(numSurf, modForce, modSpeed, modLSF, variance, numCoeff, numTri, numMod, DT1, DT2, DT3, maxSpeed, maxForce, mu);
				tempmatrix.AddEntry(HashEntry, numMod, allSpeeds, allForces);
			}
		}

	} // end loop through all textures

	return tempmatrix;
}

/******************************************************************************
Calculating Vibrations from texture files
******************************************************************************/
double TextureModel::GetVibrations()
{
    double output = 0.0;
    double excitation = 0.0;
    double rgen_mean = 0.;
    boost::mt19937 generator;

    //Double buffered, if buffer 1:
    if (SynthesisFlag_Buffer1) {
        //generate Gaussian random number with power equal to interpolation model variance
        boost::normal_distribution<> nd(rgen_mean, sqrt(filtVariance_buf1));
        boost::variate_generator<boost::mt19937&,
            boost::normal_distribution<> > var_nor(rng, nd);
        excitation = var_nor();
        output = 0.0;

        //if the size of output history is less than the number of AR coefficients, append zeros
        if (outputHist.size() < (unsigned int)MAX_COEFF) {
            int subt = MAX_COEFF - outputHist.size();
            for (int j = 0; j < subt; j++) {
                outputHist.push_back(0.0);
            }
        }
        //if the size of excitation history is less than the number of MA coefficients, append zeros
        if (excitationHist.size() < (unsigned int)MAX_MACOEFF) {
            int subt = MAX_MACOEFF - excitationHist.size();
            for (int j = 0; j < subt; j++) {
                excitationHist.push_back(0.0);
            }
        }

        //apply AR coefficients to history of output values
        for (int i = 0; i < coeffNum; i++) {
            output += outputHist.at(i) * (-filtCoeff_buf1[i]);
        }
        //if applicable, also apply MA coefficients to history of excitation values
        if (isARMA) {
            output += excitation * filtGain_buf1;
            for (int i = 0; i < MAcoeffNum; i++) {
                output += excitationHist.at(i) * (filtMACoeff_buf1[i]) * filtGain_buf1;
            }

        }
        else {
            output += excitation;
        }

        //if the size of output history is greater than the number of AR coefficients, make the extra values zero so we're not storing junk
        if (outputHist.size() > (unsigned int) coeffNum) {
            for (unsigned int kk = coeffNum; kk < outputHist.size(); kk++)
                outputHist.at(kk) = 0.0;
        }
        //if the size of excitation history is greater than the number of MA coefficients, make the extra values zero so we're not storing junk
        if (excitationHist.size() > (unsigned int) MAcoeffNum) {
            for (unsigned int kk = MAcoeffNum; kk < excitationHist.size(); kk++)
                excitationHist.at(kk) = 0.0;
        }

    }
    else {//if buffer 2
 //generate Gaussian random number with power equal to interpolation model variance
        boost::normal_distribution<> nd(rgen_mean, sqrt(filtVariance_buf2));
        boost::variate_generator<boost::mt19937&,
            boost::normal_distribution<> > var_nor(rng, nd);
        excitation = var_nor();
        output = 0.0;

        //if the size of output history is less than the number of AR coefficients, append zeros
        if (outputHist.size() < (unsigned int)MAX_COEFF) {
            int subt = MAX_COEFF - outputHist.size();
            for (int j = 0; j < subt; j++) {
                outputHist.push_back(0.0);
            }
        }
        //if the size of excitation history is less than the number of MA coefficients, append zeros
        if (excitationHist.size() < (unsigned int)MAX_MACOEFF) {
            int subt = MAX_MACOEFF - excitationHist.size();
            for (int j = 0; j < subt; j++) {
                excitationHist.push_back(0.0);
            }
        }

        //apply AR coefficients to history of output values
        for (int i = 0; i < coeffNum; i++) {
            output += outputHist.at(i) * (-filtCoeff_buf2[i]);
        }
        //if applicable, also apply MA coefficients to history of excitation values
        if (isARMA) {
            output += excitation * filtGain_buf2;
            for (int i = 0; i < MAcoeffNum; i++) {
                output += excitationHist.at(i) * (filtMACoeff_buf2[i]) * filtGain_buf2;
            }

        }
        else {
            output += excitation;
        }

        //if the size of output history is greater than the number of AR coefficients, make the extra values zero so we're not storing junk
        if (outputHist.size() > (unsigned int) coeffNum) {
            for (unsigned int kk = coeffNum; kk < outputHist.size(); kk++) {
                outputHist.at(kk) = 0.0;
            }
        }
        //if the size of excitation history is greater than the number of MA coefficients, make the extra values zero so we're not storing junk
        if (excitationHist.size() > (unsigned int) MAcoeffNum) {
            for (unsigned int kk = MAcoeffNum; kk < excitationHist.size(); kk++)
                excitationHist.at(kk) = 0.0;
        }
    }

    // remove the last element of our output vector
    outputHist.pop_back();
    excitationHist.pop_back();
    // push our new ouput value onto the front of our vector stack
    outputHist.insert(outputHist.begin(), output);
    excitationHist.insert(excitationHist.begin(), excitation);

    return output; //this is the output vibration value (in m/s^2)
}

char* TextureModel::GetModelName(int model)
{
	return texArray[model];
}

void TextureModel::setTextureModel(int model)
{
	textNum = model;
}

int TextureModel::getTextureModel()
{
	return textNum;
}

string TextureModel::GetModelImage(int model)
{
	return baseDirectory + ToString(imArray[model]);
}

string TextureModel::GetCurrentModelImage()
{
	return baseDirectory + ToString(imArray[textNum]);
}

void TextureModel::Interpolate(double force, double velocity)
{
	this->matrix.HashAndInterp2(textNum, velocity, force);
}