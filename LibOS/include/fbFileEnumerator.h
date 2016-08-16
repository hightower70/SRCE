#ifndef __fbFileEnumerator_h
#define __fbFileEnumerator_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>
#include <sysString.h>

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
bool fbFileEnumerationOpen(sysString in_path);
bool fbFileEnumertationNext(void);
void fbFileEnumerationClose(void);

#endif