/*****************************************************************************/
/*                                                                           */
/*    Domino Operation System Compiler Specific Module                       */
/*    Turn-off byte packed structs                                            */
/*                                                                           */
/*    Copyright (C) 2005 Laszlo Arvai                                        */
/*                                                                           */
/*****************************************************************************/
#ifdef WIN32
#pragma pack()
#else
#error "Pack method is not defined"
#endif
