/*
	Errors.h

	* This module handle the errors that may occur during the execution of the sketch.
	* If the error is fatal, the device will be restarted in 3 seconds, in order to try to recover from the error.
*/

#ifndef Errors_H_
#define Errors_H_

/**
 * Enumerate the errors
 * 
 * none: No error
 * noInternet: No internet connection
 * noDatabaseConnection: No database connection
 * noNTPdata: NTP Sync failed
 * bufferFull: Buffer full
 * IMUInitFailure: IMU initialization failure
 */
enum class ErrorType {
	None,
	NoInternet,
	NoDatabaseConnection,
	NoNTPdata,
	BufferFull,
	IMUInitFailure
};

/**
 * This class handle the errors that may occur during the execution of the sketch.
 * If the error is fatal, the device will be restarted in 3 seconds, in order to try to recover from the error.
 */
class Errors {
public:

	Errors();

	/** Show an error message on the serial port
	 * 
	 * @param error The error type
	 * @param fatal If true, the device will be restarted in 3 seconds
	*/
	void showError(ErrorType error, bool fatal = false);
};

// Declare the extern instance of the Errors class
extern Errors errorHandler;

#endif  // Errors_H_