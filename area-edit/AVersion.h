#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "20";
	static const char MONTH[] = "06";
	static const char YEAR[] = "2011";
	static const double UBUNTU_VERSION_STYLE = 11.06;
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 2;
	static const long MINOR = 4;
	static const long BUILD = 144;
	static const long REVISION = 757;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 265;
	#define RC_FILEVERSION 2,4,144,757
	#define RC_FILEVERSION_STRING "2, 4, 144, 757\0"
	static const char FULLVERSION_STRING[] = "2.4.144.757";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 4;
	

}
#endif //VERSION_h
