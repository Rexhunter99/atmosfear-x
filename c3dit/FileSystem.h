
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

/*
	UNUSED, FOR FUTURE VERSIONS OF TOOL
*/

#include <string>
using namespace std;


class FileBinary
{

public:

	enum EFileType {
		FILE_READ,
		FILE_WRITE,
	};

	FileBinary();
	~FileBinary();

	bool OpenFile( string, EFileType);

	bool	ReadBool();
	__int32	ReadInt();
	__int64	ReadInt64();
	float	ReadFloat();
	double	ReadDouble();
}


#endif