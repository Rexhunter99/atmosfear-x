
#include "IniFile.h"

IniFile::IniFile()
{

}

IniFile::~IniFile()
{
	this->mGroups.clear();
}

IniGroup::IniGroup()
{

}

IniGroup::IniGroup( string Name )
{
	this->mName = Name;
}

IniGroup::~IniGroup()
{
	this->mKeys.clear();
}

IniKey::IniKey()
{

}

IniKey::IniKey( string Name, string Value )
{
	this->mName = Name;
	this->mValue = Value;
}

IniKey::~IniKey()
{

}


bool IniFile::Load( string FileName )
{
	FILE* hFile = 0;
	errno_t eFile = 0;

	mUpdated = false;
	
	this->mFileName = FileName;

	fopen_s( &hFile, FileName.c_str(), "rb" );

	if ( hFile == NULL )
	{
		return false;
	}

	string current_group = "";

	while ( !feof(hFile) )
	{
		char line[256];
		fgets( line, 255, hFile );

		if ( line[0] == '\r' ) continue;
		if ( line[0] == '\n' ) continue;
		if ( line[0] == '\0' ) continue;
		if ( line[0] == ';' ) continue;

		if ( line[0] == '[' )
		{
			char *group = strtok( &line[1], "]\n\r \t;" );
			this->mGroups.push_back( IniGroup( group ) );
			current_group = group;
			continue;
		}

		if ( strstr( line, "=" ) )
		{
			char *key = strtok( line, "=" );
			char *val = strtok(    0, "\n\r" );
			this->mGroups[ this->mGroups.size()-1 ].mKeys.push_back( IniKey( key, (val!=0)?val:"" ) );
			continue;
		}
	}

	fclose( hFile );
	return true;
}

bool IniFile::Save( string FileName )
{
	FILE* hFile = 0;
	errno_t eFile = 0;

	this->mFileName = FileName;

	fopen_s( &hFile, FileName.c_str(), "wb" );

	if ( hFile == NULL )
	{
		return false;
	}

	string current_group = "";

	for ( unsigned g=0; g<this->mGroups.size(); g++ )
	{
		fprintf( hFile, "[%s]\r\n", this->mGroups[g].mName.c_str() );
		for ( unsigned k=0; k<this->mGroups[g].mKeys.size(); k++ )
		{
			fprintf( hFile, "%s=%s\r\n", this->mGroups[g].mKeys[k].mName.c_str(), this->mGroups[g].mKeys[k].mValue.c_str() );
		}
		fprintf( hFile, "\r\n" );
	}

	fclose( hFile );
	return true;
}

string IniGroup::FindKey( string Name )
{
	string Value = "(null)";

	for ( unsigned i=0; i<this->mKeys.size(); ++i )
	{
		if ( this->mKeys[i].mName == Name )
		{
			Value = this->mKeys[i].mValue;
			break;
		}
	}

	return Value;
}

int		IniFile::ReadKeyInt( string Group, string Key, int Default )
{
	int Value = Default;

	for ( unsigned i=0; i<this->mGroups.size(); ++i )
	{
		if ( this->mGroups[i].mName == Group )
		{
			string res = this->mGroups[i].FindKey( Key );
			if ( res != "(null)" )
			{
				Value = atoi( res.c_str() );
				break;
			}
		}
	}

	return Value;
}

float	IniFile::ReadKeyFloat( string Group, string Key, float Default )
{
	float Value = Default;

	for ( unsigned i=0; i<this->mGroups.size(); ++i )
	{
		if ( this->mGroups[i].mName == Group )
		{
			string res = this->mGroups[i].FindKey( Key );
			if ( res != "(null)" )
			{
				Value = (float)atof( res.c_str() );
				break;
			}
		}
	}

	return Value;
}

string	IniFile::ReadKeyString( string Group, string Key, string Default )
{
	string Value = Default;

	for ( unsigned i=0; i<this->mGroups.size(); ++i )
	{
		if ( this->mGroups[i].mName == Group )
		{
			string res = this->mGroups[i].FindKey( Key );
			if ( res != "(null)" )
			{
				Value = res;
				break;
			}
		}
	}

	return Value;
}

void	IniFile::WriteKeyInt( string Group, string Key, int Value )
{
	char sValue[64] = "";
	sprintf( sValue, "%d", Value );
	this->WriteKeyString( Group, Key, sValue );
}

void	IniFile::WriteKeyFloat( string Group, string Key, float Value )
{
	char sValue[64] = "";
	sprintf( sValue, "%f", Value );
	this->WriteKeyString( Group, Key, sValue );
}

void	IniFile::WriteKeyString( string Group, string Key, string Value )
{
	mUpdated = false;

	for ( unsigned g = 0; g < this->mGroups.size(); g++ )
	{
		if ( this->mGroups[g].mName != Group ) continue;

		bool set = false;

		for ( unsigned k=0; k<this->mGroups[g].mKeys.size(); k++ )
		{
			if ( this->mGroups[g].mKeys[k].mName == Key )
			{
				this->mGroups[g].mKeys[k].mValue = Value;
				set = true;
				break;
			}
		}
		if ( set ) break;

		this->mGroups[g].mKeys.push_back( IniKey( Key, Value ) );
		mUpdated = true;

		break;
	}
	if ( mUpdated ) return;

	this->mGroups.push_back( IniGroup( Group ) );
	this->mGroups[ this->mGroups.size()-1 ].mKeys.push_back( IniKey( Key, Value ) );
}