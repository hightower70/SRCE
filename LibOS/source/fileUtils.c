///////////////////////////////////////////////////////////////////////////////
// Includes
#include <fileUtils.h>
#include <sysString.h>

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
sysString fileGetFileNameExtension(sysString in_file_name)
{
	sysString pos;
	sysString extension = sysNULL;

	pos = in_file_name;
	while(*pos != '\0')
	{
		if(*pos == '.')
			extension = pos + 1;

		pos++;
	}

	return extension;
}

///////////////////////////////////////////////////////////////////////////////
// Concatenates two paths
void fileConcatPath(sysString in_path1, sysStringLength in_path1_length, sysString in_path2)
{
	sysStringLength path1_pos;
	sysStringLength path2_pos;

	// init path1 pos to the end of the path
	path1_pos = 0;
	while(in_path1[path1_pos] != '\0')
		path1_pos++;

	if(path1_pos > 0)
	{
		// ad separator if needed
		if(in_path1[path1_pos-1] != filePATH_SEPARATOR)
		{
			if(path1_pos < in_path1_length)
			{
				in_path1[path1_pos++] = filePATH_SEPARATOR;
			}
			else
				return;
		}
	}

	// start parsing path2
	path2_pos = 0;
	while(in_path2[path2_pos] != '\0')
	{
		// check for root folder
		if(path2_pos == 0 && in_path2[0] == filePATH_SEPARATOR)
		{
			// root folder selected
			path1_pos = 0;
			path2_pos++;
		}
		else
		{
			// check for current and parent directory
			if(in_path2[path2_pos] == '.')
			{
				path2_pos++;
				if(in_path2[path2_pos] == '.')
				{
					// move one folder up
					if(path1_pos > 0 && in_path1[path1_pos-1] == filePATH_SEPARATOR)
						path1_pos -= 2;

					while(path1_pos > 0 && in_path1[path1_pos] != filePATH_SEPARATOR)
						path1_pos--;
				}

				// move to the next folder path of path2
				while(in_path2[path2_pos] != '\0' && in_path2[path2_pos] != filePATH_SEPARATOR)
					path2_pos++;
			}
			else
			{
				// concat folder name
				while(in_path2[path2_pos] != '\0')
				{
					in_path1[path1_pos] = in_path2[path2_pos];

					if(in_path2[path2_pos] == filePATH_SEPARATOR)
						break;

					path1_pos++;
					path2_pos++;

					if(path1_pos >= in_path1_length)
						return;
				}
			}
		}
	}

	// terminate path1
	if(path1_pos < in_path1_length - 1)
	{	
		in_path1[path1_pos] = '\0';

		// add at least one separator to avoid empty path
		if( strFindChar(in_path1, filePATH_SEPARATOR) == sysSTRING_INVALID_POS)
		{
			in_path1[path1_pos] = filePATH_SEPARATOR;

			if(path1_pos < in_path1_length - 1)
				path1_pos++;

			in_path1[path1_pos] = '\0';
		}
	}
	else
	{
		in_path1[in_path1_length-1] = '\0';
	}
}

void fileSizeToString(sysString in_buffer, sysStringLength in_buffer_length, uint32_t in_size)
{
	static sysConstString units[5] = { "B", "kB", "MB", "GB" };
	uint8_t unit = 0;
	sysStringLength pos;
	uint8_t precision;
	uint32_t divisor = 1;
	uint32_t size = in_size;
	uint32_t remaining;

	// detemine magnitude
	while(size > 1024)
	{
		divisor *= 1024;
		remaining = size % divisor;
		size /= divisor;
		unit++;
	}

	if(size > 99 || (divisor == 1))
	{
		precision = 0;
	}
	else
	{
		if(size > 9)
		{
			precision = 1;
			size = (in_size * 10 + divisor / 2) / divisor;
		}
		else
		{
			precision = 2;
			size = (in_size * 100  + divisor / 2) / divisor;
		}
	}

	// display size
	pos = strWordToString(in_buffer, in_buffer_length, (uint16_t)size, 0, precision, 0);
	pos = strCopyConstString(in_buffer, in_buffer_length, pos, units[unit]);
}


bool fileShortenDisplayPath(sysString in_buffer, sysStringLength in_buffer_length)
{
	sysStringLength last_path_separator = in_buffer_length;
	sysStringLength before_last_separator = in_buffer_length;
	sysStringLength index;
	sysChar ch;

	// find separators
	index = 0;
	while(index < in_buffer_length && in_buffer[index] != '\0')
	{
		if(in_buffer[index] == filePATH_SEPARATOR && in_buffer[index+1] != '\0')
		{
			// store only when last separator points to a non removed path
			if(in_buffer[last_path_separator+1] != '.' || in_buffer[last_path_separator+2] !=	'.' || in_buffer[last_path_separator+3] != '.')
				before_last_separator = last_path_separator;
			last_path_separator = index;
		}

		index++;
	}

	// if there is nothing to remove -> return
	if(last_path_separator == in_buffer_length || before_last_separator == in_buffer_length)
		return false;

	// if the folder to remove is too short remove two folders
	if(last_path_separator - before_last_separator < 3)
	{
		index = before_last_separator;
		while(index > 0)
		{
			if(in_buffer[index] == filePATH_SEPARATOR)
			{
				before_last_separator = index;
				break;
			}
			else
				index++;
		}

		// no folder to remove
		if(index == 0)
			return false;
	}

	// add ellipsis (three dots)
	before_last_separator++;
	in_buffer[before_last_separator++] = '.';
	in_buffer[before_last_separator++] = '.';
	in_buffer[before_last_separator++] = '.';

	// remove folder
	do
	{
		ch = in_buffer[last_path_separator++];
		in_buffer[before_last_separator++] = ch;
	}	while(ch != '\0');

	return true;
}
