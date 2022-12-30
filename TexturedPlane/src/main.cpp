/***********************************************************************************************************************************
COPYRIGHT AND PERMISSION NOTICE
Penn Software The Penn Haptic Texture Toolkit
Copyright (C) 2013 The Trustees of the University of Pennsylvania
All rights reserved.

See copyright and permission notice at the end of this file.

Report bugs to Heather Culbertson (hculb@seas.upenn.edu, +1 215-573-6748) or Katherine J. Kuchenbecker (kuchenbe@seas.upenn.edu, +1 215-573-2786)

This code is based on the original TexturePad haptic rendering system designed by Joseph M. Romano.
************************************************************************************************************************************/

/*******************************************************************************

Module Name:

TexturedSphere.cpp

*******************************************************************************/

#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <time.h>
//#include "shared.h"
//#include "sharedInit.h"
#include "AccSynthHashMatrix.h"
#include "autoGenHashMatrix.h"
#include <HD/hd.h>
#include <HDU/hduError.h>
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>
//file control - debugging
#include <fstream>
#include <cstdlib>

#if defined(WIN32) || defined(linux)
# include <GL/glut.h>
#elif defined(__APPLE__)
# include <GLUT/glut.h>
#endif

#if defined(linux)
# include <sys/time.h>
#endif

#include "Constants.h"
#include "ContactModel.h"

/*******************************************************************************
Globals, declarations
*******************************************************************************/

/* Center of fixed sphere, center of haptic workspace */
hduVector3Dd gCenter;

/* Current button state */
int btn[3] = { 0, 0, 0 };

/* Current mouse position */
int mouse_x, mouse_y;

/* Dynamics simulation */
ContactModel* gContactModelHS;

HHD ghHD = HD_INVALID_HANDLE;
HDSchedulerHandle gSchedulerCallback = HD_INVALID_HANDLE;

void displayFunction(void);
void handleMenu(int);
void handleMouse(int, int, int, int);
void initGlut(int argc, char* argv[]);
void initGraphics(const HDdouble LLB[3], const HDdouble TRF[3]);
void displayVisitorSphere(GLUquadricObj* quadObj, const double position[3]);
void displayFixedSphere(GLUquadricObj* quadObj, const double position[3]);
void doGraphicsState();
void loadTexture();
void loadHashMatrix();
GLuint loadBMP_custom(const char * imagepath);
void printtext(int x, int y, std::string String, bool sizeL);
void processNormalKeys(unsigned char key, int x, int y);

hduVector3Dd priorPosition(0, 0, 0);
hduVector3Dd priorVelocity(0, 0, 0);
hduVector3Dd priorpriorVelocity(0, 0, 0);

AccSynthHashMatrix mymatrix;
boost::mt19937 rng;

// Precise timing variables
char clockResult[100];
LARGE_INTEGER ticksPerSecond;
LARGE_INTEGER thisTime;
LARGE_INTEGER lastTime;
LARGE_INTEGER deltaTime;
double deltaTimeS;
LARGE_INTEGER trialTime;
double trialTimeS;

//pAudio variables
std::vector <float> outputHist;
std::vector <float> excitationHist;

// debug variables
bool debug_flag = false;
int loopCount = 0;
int debugLoopCount = 0;

/*******************************************************************************
Get data modified by haptics thread, copy them into data that can be
used by graphics thread/
*******************************************************************************/
struct Synchronizer
{
    HHD m_hHD;
    
    // Will be assigned to global pointer modified by haptics thread
    ContactModel *pContactModelHS;
    
    // pClientData will be coped and assigned into these, thread safe
    hduVector3Dd visitorPosition;
    hduVector3Dd forceOnVisitor;
};

/*******************************************************************************
Callback used by the graphics main loop. It copies thread-safely data
that is modified constantly by the haptics thread.
*******************************************************************************/
HDCallbackCode HDCALLBACK GetStateCB(void *pUserData)
{
    Synchronizer *pSynchronizer = static_cast<Synchronizer*>(pUserData);
    
    // pSynchronizer->pContactModelHS points to same data structure passed
    // in the haptics thread callback. So it is updated every servoloop tick
    // and is not thread safe to access directly.
    
    pSynchronizer->visitorPosition =
    pSynchronizer->pContactModelHS->GetCurrentContactPoint();
    
    pSynchronizer->forceOnVisitor =
    pSynchronizer->pContactModelHS->GetCurrentForceOnVisitor();
    
    return HD_CALLBACK_DONE;
}

/*******************************************************************************
Haptic device record
*******************************************************************************/
struct DeviceDisplayState
{
    HHD m_hHD;
    double position[3];
    double force[3];
};

/*******************************************************************************
Query haptic device state
*******************************************************************************/
HDCallbackCode HDCALLBACK DeviceStateCallback(void *pUserData)
{
    DeviceDisplayState *pDisplayState =
    static_cast<DeviceDisplayState *>(pUserData);
    
    HHD hHD = hdGetCurrentDevice();
    hdBeginFrame(hHD);
    hdGetDoublev(HD_CURRENT_POSITION, pDisplayState->position);
    hdGetDoublev(HD_CURRENT_FORCE, pDisplayState->force);
    hdEndFrame(hHD);
    
    return HD_CALLBACK_DONE;
}

/*******************************************************************************
Graphics main loop
*******************************************************************************/
void displayFunction(void)
{
    glPushMatrix();
    doGraphicsState();
    
    // Get a snapshot of the haptics thread data  - thread safe
    Synchronizer synchronizer;
    
    // Since the haptics loop may be modifying the synchronizer object
    // that it is pointing to, the graphics loop cannot use it directly.
    // A synchronous callback is thread safe to access the data.
    synchronizer.pContactModelHS = gContactModelHS;
    
    hdScheduleSynchronous(GetStateCB, &synchronizer, HD_DEFAULT_SCHEDULER_PRIORITY);
    
    GLUquadricObj* quadObj = gluNewQuadric();
    
    // sort objects to draw the one behind first
    if(synchronizer.visitorPosition[2] < gCenter[2])
    {
        displayVisitorSphere(quadObj, synchronizer.visitorPosition);
        displayFixedSphere(quadObj, gCenter);
    }
    else
    {
        displayFixedSphere(quadObj, gCenter);
        displayVisitorSphere(quadObj, synchronizer.visitorPosition);
    }
    
    gluDeleteQuadric(quadObj);
    glEnable(GL_LIGHTING);
    
    // Displays text to screen
    printtext(10,750,texArray[textNum],true);  // prints texture name
    if(dispFric)
    	printtext(10,725,"Friction ON!",false); // display of friction is on
    else
    	printtext(10,725,"Friction OFF!",false); // display of friction is off
    if(dispTex)
    	printtext(10,700,"Texture ON!",false); // display of texture is on
    else
    	printtext(10,700,"Texture OFF!",false); // display of texture is off
    
    // prints name of texture group	
    if(texGroupNum==0)
    	printtext(10,10,"Paper",true);
    else if(texGroupNum==1)
    	printtext(10,10,"Plastic",true);
    else if(texGroupNum==2)
    	printtext(10,10,"Fabric",true);
    else if(texGroupNum==3)
    	printtext(10,10,"Tile",true);
    else if(texGroupNum==4)
    	printtext(10,10,"Carpet",true);
    else if(texGroupNum==5)
   	printtext(10,10,"Foam",true);
    else if(texGroupNum==6)
   	printtext(10,10,"Metal",true);
    else if(texGroupNum==7)
   	printtext(10,10,"Stone",true);
    else if(texGroupNum==8)
   	printtext(10,10,"Carbon Fiber",true);
    else if(texGroupNum==9)
   	printtext(10,10,"Wood",true);
    
    glPopMatrix();
    glutSwapBuffers();
}

/******************************************************************************
Popup menu handler
******************************************************************************/
void handleMenu(int ID)
{
    switch(ID) {
        case 0:
        exit(0);
        break;
    }
}

/******************************************************************************
Mouse events handler
******************************************************************************/
void handleMouse(int b, int s, int x, int y)
{
    if (s == GLUT_DOWN) {
        btn[b] = 1;
        } else {
        btn[b] = 0;
    }
    
    mouse_x = x;
    mouse_y = glutGet(GLUT_WINDOW_HEIGHT) - y;
}

/******************************************************************************
Called by the GLUT framework
******************************************************************************/
void handleIdle(void)
{
    glutPostRedisplay();
    if(!hdWaitForCompletion(gSchedulerCallback, HD_WAIT_CHECK_STATUS))
    {
        printf("The main scheduler callback has exited\n");
        printf("Press any key to quit.\n");
        getchar();
        exit(-1);
    }
}
/******************************************************************************
Calculating Vibrations from texture files
******************************************************************************/
static double vibrations()
{
    double output = 0.0;
    double excitation = 0.0;
    double rgen_mean=0.;
    boost::mt19937 generator;
    
    //Double buffered, if buffer 1:
    if(SynthesisFlag_Buffer1) {
        //generate Gaussian random number with power equal to interpolation model variance
        boost::normal_distribution<> nd(rgen_mean, sqrt(filtVariance_buf1));
        boost::variate_generator<boost::mt19937&,
        boost::normal_distribution<> > var_nor(rng, nd);
        excitation = var_nor();
        output = 0.0;
        
        //if the size of output history is less than the number of AR coefficients, append zeros
        if(outputHist.size()<(unsigned int) MAX_COEFF) {
            int subt = MAX_COEFF - outputHist.size();
            for(int j = 0; j < subt ; j++) {
                outputHist.push_back(0.0);
            }
        }
        //if the size of excitation history is less than the number of MA coefficients, append zeros
        if(excitationHist.size()<(unsigned int) MAX_MACOEFF) {
            int subt = MAX_MACOEFF - excitationHist.size();
            for(int j = 0; j < subt ; j++) {
                excitationHist.push_back(0.0);
            }
        }
        
        //apply AR coefficients to history of output values
        for(int i = 0; i < coeffNum; i++) {
            output += outputHist.at(i) * (-filtCoeff_buf1[i]);
        }
        //if applicable, also apply MA coefficients to history of excitation values
        if(isARMA){
            output += excitation*filtGain_buf1;
            for(int i = 0; i < MAcoeffNum; i++) {
                output += excitationHist.at(i) * (filtMACoeff_buf1[i])*filtGain_buf1;
            }
            
            } else{
            output += excitation;
        }

        //if the size of output history is greater than the number of AR coefficients, make the extra values zero so we're not storing junk
        if(outputHist.size()>(unsigned int) coeffNum) {
            for(unsigned int kk = coeffNum; kk < outputHist.size(); kk++)
            outputHist.at(kk) = 0.0;
        }
        //if the size of excitation history is greater than the number of MA coefficients, make the extra values zero so we're not storing junk
        if(excitationHist.size()>(unsigned int) MAcoeffNum) {
            for(unsigned int kk = MAcoeffNum; kk < excitationHist.size(); kk++)
            excitationHist.at(kk) = 0.0;
        }
        
        } else {//if buffer 2
        //generate Gaussian random number with power equal to interpolation model variance
        boost::normal_distribution<> nd(rgen_mean, sqrt(filtVariance_buf2));
        boost::variate_generator<boost::mt19937&,
        boost::normal_distribution<> > var_nor(rng, nd);
        excitation = var_nor();
        output = 0.0;
        
        //if the size of output history is less than the number of AR coefficients, append zeros
        if(outputHist.size()<(unsigned int) MAX_COEFF) {
            int subt = MAX_COEFF - outputHist.size();
            for(int j = 0; j < subt ; j++) {
                outputHist.push_back(0.0);
            }
        }
        //if the size of excitation history is less than the number of MA coefficients, append zeros
        if(excitationHist.size()<(unsigned int) MAX_MACOEFF) {
            int subt = MAX_MACOEFF - excitationHist.size();
            for(int j = 0; j < subt ; j++) {
                excitationHist.push_back(0.0);
            }
        }
        
        //apply AR coefficients to history of output values
        for(int i = 0; i < coeffNum; i++) {
            output += outputHist.at(i) * (-filtCoeff_buf2[i]);
        }
        //if applicable, also apply MA coefficients to history of excitation values
        if(isARMA){
            output += excitation*filtGain_buf2;
            for(int i = 0; i < MAcoeffNum; i++) {
                output += excitationHist.at(i) * (filtMACoeff_buf2[i])*filtGain_buf2;
            }
            
            } else{
            output += excitation;
        }

        //if the size of output history is greater than the number of AR coefficients, make the extra values zero so we're not storing junk
        if(outputHist.size()>(unsigned int) coeffNum) {
            for(unsigned int kk = coeffNum; kk < outputHist.size(); kk++) {
                outputHist.at(kk) = 0.0;
            }
        }
        //if the size of excitation history is greater than the number of MA coefficients, make the extra values zero so we're not storing junk
        if(excitationHist.size()>(unsigned int) MAcoeffNum) {
            for(unsigned int kk = MAcoeffNum; kk < excitationHist.size(); kk++)
            excitationHist.at(kk) = 0.0;
        }
    }
    
    // remove the last element of our output vector
    outputHist.pop_back();
    excitationHist.pop_back();
    // push our new ouput value onto the front of our vector stack
    outputHist.insert(outputHist.begin(),output);
    excitationHist.insert(excitationHist.begin(),excitation);
    
    return output; //this is the output vibration value (in m/s^2)
}

/******************************************************************************
Main callback that runs dynamic simulation with haptics loop
******************************************************************************/
HDCallbackCode HDCALLBACK ContactCB(void *data)
{
    double output;
    
    float slopeFric = 0.004; // 0.004
    float vThresh; // threshold velocity for friction calculation
    vThresh = mu_k/slopeFric;
    
    static const hduVector3Dd direction(0, 1, 0); //direction of the vibration - now y-axis
    static HDdouble timer = 0; // timer, as defined by OH
    
    HDdouble instRate; //instantaneous rate of update of device
    
    hduVector3Dd forceTex; //the calculated texture force
    hduVector3Dd forceFric; //the friction force
    double forceNorm; //magnitude of the normal force
    hduVector3Dd currPosition; //Current position vector
    hduVector3Dd currVelocity; //current velocity vector in mm/s
    double distFromCenter;
    hduVector3Dd normalVelocity; //normal vector of the velocity
    hduVector3Dd normalVec; //vector normal to the sphere
    hduVector3Dd tanVelocity;//tangential velocity vector
    
    hduVector3Dd tanVec; //vector tangent to direction of travel
    hduVector3Dd textureVec; //direction to display texture force (binormal to normalVec and tanVec)
    ContactModel *pContactModelHS = static_cast<ContactModel*>(data);
    
    
    HHD hHD = hdGetCurrentDevice();
    hdGetDoublev(HD_INSTANTANEOUS_UPDATE_RATE, &instRate);
    hdGetDoublev(HD_CURRENT_POSITION, currPosition);
    
    timer += 1.0 / instRate;
    
    hdBeginFrame(hHD); //haptic frame begins here <~~~~~~~~~~~~

	// Cache the time of the previous haptic function call.
	lastTime = thisTime;
	
	// Find out what time it is now.  This information facilitates accurate velocity calculation.
	QueryPerformanceCounter(&thisTime);

	// Calculate time since last call in clock cycles and then convert to seconds.
	deltaTime.QuadPart = (thisTime.QuadPart - lastTime.QuadPart);
	deltaTimeS = (float) deltaTime.LowPart / (float) ticksPerSecond.QuadPart;
    
    //Second order lowpass filter parameters
    float lambda = 125.0; //cutoff frequency of lowpass filter in rad/s
	float T = deltaTimeS; //current sampling time
    float w0 = lambda*lambda*T*T/((1.0+T*lambda)*(1.0+T*lambda)); //weight for z^0 term
    float w1 = 2.0/(1.0+lambda*T); //weight for z^-1 term
    float w2 = -1.0/((1.0+T*lambda)*(1.0+T*lambda)); //weight for z^-2 term
       
    currVelocity = w0*((currPosition - priorPosition) / T) + w1*priorVelocity + w2*priorpriorVelocity; //second order low-pass filter at 20 Hz
    
    priorpriorVelocity = priorVelocity; //store new values for filter in next loop
    priorVelocity = currVelocity;

    normalVec = (currPosition - gCenter)/((currPosition - gCenter).magnitude()); //unit vector normal to surface
    
    hduVector3Dd newEffectorPosition;
    hdGetDoublev(HD_CURRENT_POSITION,newEffectorPosition);
    
    // Given the new master position, run the dynamics simulation and
    // update the slave position.
    pContactModelHS->UpdateEffectorPosition(newEffectorPosition);
    
    hduVector3Dd forceVec;
    hduVector3Dd forceM;
    hdGetDoublev(HD_CURRENT_FORCE,forceM);
    
    distFromCenter = (currPosition - gCenter).magnitude();
    
    forceVec = pContactModelHS->GetCurrentForceOnVisitor(); //get current force being applied to user
    if (distFromCenter <= (FIXED_SPHERE_RADIUS + VISITOR_SPHERE_RADIUS)) //if touching sphere
    {
        {
            normalVelocity = normalVec.dotProduct(currVelocity) * normalVec; //velocity in normal direction
            forceNorm = normalVec.dotProduct(forceVec); //normal force being applied
            tanVelocity = currVelocity - normalVelocity; //velocity in tangential direction
            mymatrix.HashAndInterp2(textNum,tanVelocity.magnitude(),forceNorm); //interpolate between models using the current normal force and tangential velocity
            tanVec = tanVelocity/tanVelocity.magnitude(); //unit vector tangent to direction of motion     
            textureVec = tanVec.crossProduct(normalVec); //unit vector for display of texture force
            output = vibrations(); //calculate next output texture vibration (in m/s^2)
            forceTex = textureVec * output * 0.05; //scale ouput value by effective mass of Omni handle and user's hand

            if (tanVelocity.magnitude()<vThresh) //if below velocity threshold, use viscous friction
            {
                forceFric = -mu_k*forceNorm*slopeFric*tanVelocity;
                
            } else //else Coulomb friction, mu*Fn
            {
                forceFric = -mu_k*forceNorm*tanVec;
            }
            //std::cout << forceNorm << "," << forceFric.magnitude() << "," << output << "," << tanVelocity.magnitude() << std::endl;
            forceVec = forceNorm*normalVec + forceTex + forceFric; //output force is resultant sum of normal, texture, and friction forces
            if(dispFric)
            {
            	if(dispTex)
            		forceVec = forceNorm*normalVec + forceTex + forceFric; //output force is resultant sum of normal, texture, and friction forces	
    		else
    			forceVec = forceNorm*normalVec + forceFric; //output force is resultant sum of normal and friction forces (no texture)
            }
            else
            {
            	if(dispTex)
            		forceVec = forceNorm*normalVec + forceTex; //output force is resultant sum of normal and texture forces (no friction)
    		else
    			forceVec = forceNorm*normalVec; //output force is resultant sum of normal force only (no friction or texture)
            }
        }
    }
    priorPosition = currPosition; //store new values for filter in next loop
    hdSetDoublev(HD_CURRENT_FORCE, forceVec);
    
    hdEndFrame(hHD); //haptic frame ends here <~~~~~~~~~~~~~~~
    
    HDErrorInfo error;
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        hduPrintError(stderr, &error, "Error during scheduler callback");
        
        if (hduIsSchedulerError(&error))
        {
            return HD_CALLBACK_DONE;
        }
    }
    return HD_CALLBACK_CONTINUE;
    
}

/******************************************************************************
Schedules the contact simluation callback.
******************************************************************************/
void DefineForceField()
{
    DeviceDisplayState initialMasterState;
    hdScheduleSynchronous(DeviceStateCallback, &initialMasterState,
    HD_MAX_SCHEDULER_PRIORITY);
    
    // Initialize the slave position, relative to master
    hduVector3Dd visitorLocation(2*VISITOR_SPHERE_RADIUS + FIXED_SPHERE_RADIUS,
    2*VISITOR_SPHERE_RADIUS + FIXED_SPHERE_RADIUS, 0.0);
    
    gContactModelHS = new ContactModel(FIXED_SPHERE_RADIUS,
    gCenter,
    VISITOR_SPHERE_RADIUS,
    visitorLocation);
    
    gSchedulerCallback = hdScheduleAsynchronous(ContactCB,
    gContactModelHS,
    HD_DEFAULT_SCHEDULER_PRIORITY);
}

/******************************************************************************
The handler gets called when the process is exiting.
******************************************************************************/
void exitHandler()
{
    hdStopScheduler();
    
    if (ghHD != HD_INVALID_HANDLE)
    {
        hdDisableDevice(ghHD);
        ghHD = HD_INVALID_HANDLE;
    }
}

 void loadHashMatrix() {
  
}

/******************************************************************************
 Method to handle keyboard input.
******************************************************************************/
void processNormalKeys(unsigned char key, int x, int y) {
	DeviceDisplayState state;
	
	GLuint Texture;

	switch (key) {
	
		// close the simulation
		case 27: 			//27 is the escape key
		case 'q':
		case 'Q':
			exit(0);		//close the simulation
			break;
		// + key to scroll up through textures
		case 43:
			if(textNum<NUM_TEX-1)
				textNum = textNum+1;
			
			// display correct texture group name	
			if(textNum>94)
				texGroupNum = 9;
			else if(textNum>92)
				texGroupNum = 8;
			else if(textNum>85)
				texGroupNum = 7;
			else if(textNum>79)
				texGroupNum = 6;
			else if(textNum>73)
				texGroupNum = 5;
			else if(textNum>68)
				texGroupNum = 4;
			else if(textNum>61)
				texGroupNum = 3;
			else if(textNum>31)
				texGroupNum = 2;
			else if(textNum>21)
				texGroupNum = 1;
			else
				texGroupNum = 0;
			Texture = loadBMP_custom(imArray[textNum]); // load the texture image

    			glutDisplayFunc(displayFunction); // change display of texture name
			break;
		// - key to scroll down through textures
		case 45:
			if(textNum>0)
				textNum = textNum-1;
			
			// display correct texture group name
			if(textNum>94)
				texGroupNum = 9;
			else if(textNum>92)
				texGroupNum = 8;
			else if(textNum>85)
				texGroupNum = 7;
			else if(textNum>79)
				texGroupNum = 6;
			else if(textNum>73)
				texGroupNum = 5;
			else if(textNum>68)
				texGroupNum = 4;
			else if(textNum>61)
				texGroupNum = 3;
			else if(textNum>31)
				texGroupNum = 2;
			else if(textNum>21)
				texGroupNum = 1;
			else
				texGroupNum = 0;
			Texture = loadBMP_custom(imArray[textNum]); // load the texture image
				
			glutDisplayFunc(displayFunction); // change display of texture name
			break;
		// toggle friction
		case 'f':
		case 'F':
			if(dispFric)
				dispFric=false;
			else
				dispFric=true;
		
			glutDisplayFunc(displayFunction); // display current state of friction
			break;
		// toggle texture force
		case 't':
		case 'T':
			if(dispTex)
				dispTex=false;
			else
				dispTex=true;
		
			glutDisplayFunc(displayFunction); // display current state of texture force
			break;
		// display menu in terminal window
		case 'm':
		case 'M':
			printf("\n\n Key Command List \n -----------------------------------------\n");
		        printf("    + --> Scroll Forward Through Textures\n");
		        printf("    - --> Scroll Backward Through Textures\n");
		        printf("    F/f --> Toggle Friction ON/OFF\n");
		        printf("    T/t --> Toggle Texture ON/OFF\n");
		        printf("\n    Jump to Texture Group\n");
		        printf("    ----------------------\n");
		        printf("    0 --> Paper\n");
		        printf("    1 --> Plastic\n");
		        printf("    2 --> Fabric\n");
		        printf("    3 --> Tile\n");
		        printf("    4 --> Carpet\n");
		        printf("    5 --> Foam\n");
		        printf("    6 --> Metal\n");
		        printf("    7 --> Stone\n");
		        printf("    8 --> Carbon Fiber\n");
		        printf("    9 --> Wood\n");
			break;
		// jump to Paper texture group
		case '0':
			texGroupNum = 0;
			textNum = 0;
		
			Texture = loadBMP_custom(imArray[textNum]); // load texture image
			glutDisplayFunc(displayFunction); // display current texture group
			break;
		// jump to Plastic texture group
		case '1':
			texGroupNum = 1;
			textNum = 22;
		
			Texture = loadBMP_custom(imArray[textNum]); // load texture image
			glutDisplayFunc(displayFunction); // display current texture group
			break;
		// jump to Fabric texture group
		case '2':
			texGroupNum = 2;
			textNum = 32;
		
			Texture = loadBMP_custom(imArray[textNum]); // load texture image
			glutDisplayFunc(displayFunction); // display current texture group
			break;
		// jump to Tile texture group
		case '3':
			texGroupNum = 3;
			textNum = 62;
			
			Texture = loadBMP_custom(imArray[textNum]); // load texture image
			glutDisplayFunc(displayFunction); // display current texture group
			break;
		// jump to Carpet texture group
		case '4':
			texGroupNum = 4;
			textNum = 69;
		
			Texture = loadBMP_custom(imArray[textNum]); // load texture image
			glutDisplayFunc(displayFunction); // display current texture group
			break;
		// jump to Foam texture group
		case '5':
			texGroupNum = 5;
			textNum = 74;
		
			Texture = loadBMP_custom(imArray[textNum]); // load texture image
			glutDisplayFunc(displayFunction); // display current texture group
			break;
		// jump to Metal texture group
		case '6':
			texGroupNum = 6;
			textNum = 80;
		
			Texture = loadBMP_custom(imArray[textNum]); // load texture image
			glutDisplayFunc(displayFunction); // display current texture group
			break;
		// jump to Stone texture group
		case '7':
			texGroupNum = 7;
			textNum = 86;
		
			Texture = loadBMP_custom(imArray[textNum]); // load texture image
			glutDisplayFunc(displayFunction); // display current texture group
			break;
		// jump to Carbon Fiber texture group
		case '8':
			texGroupNum = 8;
			textNum = 93;
		
			Texture = loadBMP_custom(imArray[textNum]); // load texture image
			glutDisplayFunc(displayFunction); // display current texture group
			break;
		// jump to Wood texture group
		case '9':
			texGroupNum = 9;
			textNum = 95;
		
			Texture = loadBMP_custom(imArray[textNum]); // load texture image
			glutDisplayFunc(displayFunction); // display current texture group
			break;
		// key pressed doesn't do anything
		default:
			printf("You pressed the %c key, for which there is no dedicated action.\n", key);
			break;
	}
}

/******************************************************************************
Main Function
******************************************************************************/
int main(int argc, char* argv[])
{
	// Get the high resolution counter's accuracy.
	QueryPerformanceFrequency(&ticksPerSecond);
	// Find out what time it is now.
	QueryPerformanceCounter(&thisTime);
    HDErrorInfo error;
    printf("Starting Application\n");
    atexit(exitHandler);
    
    // Initialize device before any actions on it are to be performed
    ghHD = hdInitDevice(HD_DEFAULT_DEVICE);
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        hduPrintError(stderr, &error, "Failed to initialize haptic device");
        fprintf(stderr, "\nPress any key to quit.\n");
        getchar();
        return -1;
    }
    
    printf("Found device %s\n",hdGetString(HD_DEVICE_MODEL_TYPE));
    
    hdEnable(HD_FORCE_OUTPUT);
    hdEnable(HD_MAX_FORCE_CLAMPING);
    
    hdStartScheduler();
    
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        hduPrintError(stderr, &error, "Failed to start scheduler");
        fprintf(stderr, "\nPress any key to quit.\n");
        getchar();
        return -1;
    }

    // Load texture file
    //menuInterface();
    AccSynthHashMatrix thematrix = generateHashMatrix();
    mymatrix = thematrix;
    printf("Texture matrix created successfully!\n");

    //QHGLUT* DisplayObject = initGlut(argc, argv);
    initGlut(argc, argv);
    
    HDdouble maxWorkspace[6];
    hdGetDoublev(HD_MAX_WORKSPACE_DIMENSIONS, maxWorkspace);
    
    //Low, Left, Back point of device workspace.
    hduVector3Dd LLB(maxWorkspace[0], maxWorkspace[1], maxWorkspace[2]);
    //Top, Right, Front point of device workspace.
    hduVector3Dd TRF(maxWorkspace[3], maxWorkspace[4], maxWorkspace[5]);
    initGraphics(LLB, TRF);
    
    gCenter = (LLB + TRF)/2.0;
    
    DefineForceField();
    
    // print menu to terminal window after initialization
    printf("\n\n Key Command List \n -----------------------------------------\n");
    printf("    + --> Scroll Forward Through Textures\n");
    printf("    - --> Scroll Backward Through Textures\n");
    printf("    F/f --> Toggle Friction ON/OFF\n");
    printf("    T/t --> Toggle Texture ON/OFF\n");
    printf("\n    Jump to Texture Group\n");
    printf("    ----------------------\n");
    printf("    0 --> Paper\n");
    printf("    1 --> Plastic\n");
    printf("    2 --> Fabric\n");
    printf("    3 --> Tile\n");
    printf("    4 --> Carpet\n");
    printf("    5 --> Foam\n");
    printf("    6 --> Metal\n");
    printf("    7 --> Stone\n");
    printf("    8 --> Carbon Fiber\n");
    printf("    9 --> Wood\n");

    // Enter Glut main loop
    glutMainLoop();
    
    // HDAPI cleanup managed by exitHandler
    printf("Done\n");
    return 0;
}


//********* E * O * F *********//

/***********************************************************************************************************************************
COPYRIGHT AND PERMISSION NOTICE
Penn Software The Penn Haptic Texture Toolkit
Copyright (C) 2013 The Trustees of the University of Pennsylvania
All rights reserved.

The Trustees of the University of Pennsylvania (“Penn”) and Heather Culbertson, Juan Jose Lopez Delgado, and Katherine J. Kuchenbecker, the developer (“Developer”) of Penn Software The Penn Haptic Texture Toolkit (“Software”) give recipient (“Recipient”) and Recipient’s Institution (“Institution”) permission to use, copy, and modify the software in source and binary forms, with or without modification for non-profit research purposes only provided that the following conditions are met:

1)	All copies of Software in binary form and/or source code, related documentation and/or other materials provided with the Software must reproduce and retain the above copyright notice, this list of conditions and the following disclaimer.

2)	Recipient shall have the right to create modifications of the Software (“Modifications”) for their internal research and academic purposes only. 

3)	All copies of Modifications in binary form and/or source code and related documentation must reproduce and retain the above copyright notice, this list of conditions and the following disclaimer.

4)	Recipient and Institution shall not distribute Software or Modifications to any third parties without the prior written approval of Penn.

5)	Recipient will provide the Developer with feedback on the use of the Software and Modifications, if any, in their research.  The Developers and Penn are permitted to use any information Recipient provides in making changes to the Software. All feedback, bug reports and technical questions shall be sent to: 

Heather Culbertson, hculb@seas.upenn.edu, +1 215-573-6748
Katherine J. Kuchenbecker, kuchenbe@seas.upenn.edu, +1 215-573-2786

6)	Recipient acknowledges that the Developers, Penn and its licensees may develop modifications to Software that may be substantially similar to Recipient’s modifications of Software, and that the Developers, Penn and its licensees shall not be constrained in any way by Recipient in Penn’s or its licensees’ use or management of such modifications. Recipient acknowledges the right of the Developers and Penn to prepare and publish modifications to Software that may be substantially similar or functionally equivalent to your modifications and improvements, and if Recipient or Institution obtains patent protection for any modification or improvement to Software, Recipient and Institution agree not to allege or enjoin infringement of their patent by the Developers, Penn or any of Penn’s licensees obtaining modifications or improvements to Software from the Penn or the Developers.

7)	Recipient and Developer will acknowledge in their respective publications the contributions made to each other’s research involving or based on the Software. The current citations for Software are:

Heather Culbertson, Juan Jose Lopez Delgado, and Katherine J. Kuchenbecker. One Hundred Data-Driven Haptic Texture Models and Open-Source Methods for Rendering on 3D Objects. In Proc. IEEE Haptics Symposium, February 2014.

8)	Any party desiring a license to use the Software and/or Modifications for commercial purposes shall contact The Center for Technology Transfer at Penn at 215-898-9591.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS, CONTRIBUTORS, AND THE TRUSTEES OF THE UNIVERSITY OF PENNSYLVANIA "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER, CONTRIBUTORS OR THE TRUSTEES OF THE UNIVERSITY OF PENNSYLVANIA BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

************************************************************************************************************************************/