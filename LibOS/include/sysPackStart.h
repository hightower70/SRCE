/*****************************************************************************/
/*                                                                           */
/*    Domino Operation System Compiler Specific Module                       */
/*    Turn-on byte packed structs                                            */
/*                                                                           */
/*    Copyright (C) 2005 Laszlo Arvai                                        */
/*                                                                           */
/*****************************************************************************/
#ifdef _WIN32
#pragma warning( disable : 4103 )
#pragma pack(1)
#else
#error "Pack method is not defined"
#endif
