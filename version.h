#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char mp3dec_DATE[] = "20";
	static const char mp3dec_MONTH[] = "03";
	static const char mp3dec_YEAR[] = "2012";
	static const char mp3dec_UBUNTU_VERSION_STYLE[] = "12.03";
	
	//Software Status
	static const char mp3dec_STATUS[] = "Alpha";
	static const char mp3dec_STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long mp3dec_MAJOR = 0;
	static const long mp3dec_MINOR = 4;
	static const long mp3dec_BUILD = 401;
	static const long mp3dec_REVISION = 2238;
	
	//Miscellaneous Version Types
	static const long mp3dec_BUILDS_COUNT = 635;
	#define mp3dec_RC_FILEVERSION 0,4,401,2238
	#define mp3dec_RC_FILEVERSION_STRING "0, 4, 401, 2238\0"
	static const char mp3dec_FULLVERSION_STRING[] = "0.4.401.2238";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long mp3dec_BUILD_HISTORY = 1;
	

#endif //VERSION_H
