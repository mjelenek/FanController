#ifndef PID_v1_h
#define PID_v1_h
#define LIBRARY_VERSION	1.2.1-modified

class PID
{


  public:

  //Constants used in some of the functions below
  #define AUTOMATIC	1
  #define MANUAL	0

  //commonly used functions **************************************************************************
    PID(double*, double*, double*,        // * constructor.  links the PID to the Input, Output, and 
        double, double, double);     //   Setpoint.  Initial tuning parameters are also set here
	
    void SetMode(int Mode);               // * sets PID to either Manual (0) or Auto (non-0)

    bool Compute(bool withIntegration);   // * performs the PID calculation.  it should be
                                          //   called every time loop() cycles. ON/OFF and
                                          //   calculation frequency can be set using SetMode
                                          //   SetSampleTime respectively

    void SetOutputLimits(byte, byte); // * clamps the output to a specific range. 0-255 by default, but
										                      //   it's likely the user will want to change this depending on
										                      //   the application
	


  //available but not commonly used functions ********************************************************
    void SetTunings(double, double,       // * While most users will set the tunings once in the 
                    double);         	    //   constructor, this function gives the user the option
                                          //   of changing tunings during runtime for Adaptive control
    void SetSampleTime(unsigned long);    // * sets the frequency, in Microseconds, with which 
                                          //   the PID calculation is performed.  default is 100
										  
										  
										  
  //Display functions ****************************************************************
	double GetKp();						  // These functions query the pid for interal values.
	double GetKi();						  //  they were created mainly for the pid front-end,
	double GetKd();						  // where it's important to know what is actually 
	int GetMode();						  //  inside the PID.

  private:
	void Initialize();

	double kp;                  // * (P)roportional Tuning Parameter
	double ki;                  // * (I)ntegral Tuning Parameter
	double kd;                  // * (D)erivative Tuning Parameter

	double *myInput;              // * Pointers to the Input, Output, and Setpoint variables
	double *myOutput;             //   This creates a hard link between the variables and the 
	double *mySetpoint;           //   PID, freeing the user from having to constantly tell us
                                  //   what these values are.  with pointers we'll just know.
			  
	unsigned long lastTime;
	double outputSum, lastInput;

	unsigned long SampleTime;
	byte outMin, outMax;
	bool inAuto;
};
#endif

