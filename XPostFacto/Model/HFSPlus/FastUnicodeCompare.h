//  FastUnicodeCompare - Compare two Unicode strings; produce a relative ordering
//
//      IF              RESULT
//  --------------------------
//  str1 < str2     =>  -1
//  str1 = str2     =>   0
//  str1 > str2     =>  +1
//

// Taken from http://developer.apple.com/technotes/tn/tn1150.html

#ifndef __FASTUNICODECOMPARE_H__

SInt32 FastUnicodeCompare ( register ConstUniCharArrayPtr str1, register ItemCount length1,
                            register ConstUniCharArrayPtr str2, register ItemCount length2);

#endif