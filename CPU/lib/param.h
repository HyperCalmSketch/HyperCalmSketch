#ifndef _PARAM_H_
#define _PARAM_H_

#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <vector>
#include <fstream>
#include <set>
#include <map>
#include <cstdlib>

using namespace std;


#define __max(a,b)    (((a) > (b)) ? (a) : (b))
#define __min(a,b)    (((a) < (b)) ? (a) : (b))


#define CalculateBucketPos(fp, CONSTANT_NUMBER) (((fp) * CONSTANT_NUMBER) >> 15)


#endif // _PARAM_H_
