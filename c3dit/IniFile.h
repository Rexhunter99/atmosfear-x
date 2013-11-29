
#ifndef INIFILE_H
#define INIFILE_H

#include <Windows.h>
#include <string>
#include <vector>
using namespace std;


class IniKey
{
public:
	string	mName;
	string	mValue;

	IniKey();
	IniKey( string Name, string Value );
	~IniKey();
};

class IniGroup
{
public:
	string			mName;
	vector<IniKey>	mKeys;

	IniGroup();
	IniGroup( string Name );
	~IniGroup();
	string FindKey( string Name );
};


class IniFile
{
private:
	bool				mUpdated;
	vector<IniGroup>	mGroups;
	string				mFileName;

public:

	IniFile();
	~IniFile();

	bool	Load( string FileName );
	bool	Save( string FileName );

	int		ReadKeyInt( string Group, string Key, int Default );
	float	ReadKeyFloat( string Group, string Key, float Default );
	string	ReadKeyString( string Group, string Key, string Default );

	void	WriteKeyInt( string Group, string Key, int Value );
	void	WriteKeyFloat( string Group, string Key, float Value );
	void	WriteKeyString( string Group, string Key, string Value );
};

#endif // INIFILE_H