///////////////////////////////////////////////////////////////////////////////
// Includes
#include <fbTypes.h>
#include <sysResource.h>
#include <fileUtils.h>
#include "sysConfig.h"

///////////////////////////////////////////////////////////////////////////////
// Types
typedef struct
{
	sysConstString Extension;
	sysResourceAddress Icon;
} ExtensionToIconTableEntry;

///////////////////////////////////////////////////////////////////////////////
// Module global variables
ExtensionToIconTableEntry l_extension_to_icon[] =
{
	{"vgm", REF_BMP_MUSIC },
	{"vgz", REF_BMP_MUSIC },
	{"m3u", REF_BMP_AUDIO },
	{ sysNULL, 0 }
};


sysResourceAddress fbGetFileIcon(fbFileInformation* in_file_info)
{
	uint8_t i;
	sysString extension;

	// check for folder
	if((in_file_info->Flags & BV(fbFF_FOLDER_FILE_BIT)) == fbFF_FOLDER)
	{
		if(strCompareConstString(in_file_info->FileName, (sysConstString)"..") == 0)
			return REF_BMP_UP;
		else
			return REF_BMP_FOLDER;
	}
	else
	{
		extension = fileGetFileNameExtension(in_file_info->FileName);

		if(extension != sysNULL)
		{
			i = 0;
			while(l_extension_to_icon[i].Extension != sysNULL)
			{
				if(strCompareConstStringNoCase(extension, l_extension_to_icon[i].Extension) == 0)
					return l_extension_to_icon[i].Icon;

				i++;
			}
		}

		return REF_BMP_FILE;
	}
}
