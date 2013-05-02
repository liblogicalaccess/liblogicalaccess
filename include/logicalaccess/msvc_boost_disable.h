#ifdef _MSC_VER

#pragma warning (push)

#pragma warning (disable : 4275)
#pragma warning (disable : 4250) // class inherits from another via dominance. It shouldn't be a warning for pure virtual classes...
//warning C4251: 'x' : class 'y' needs to have dll-interface to
//be used by clients of class 'z'
#pragma warning (disable : 4251)
//#pragma warning (disable : 4675)
//#pragma warning (disable : 4996)
//#pragma warning (disable : 4503)
//#pragma warning (disable : 4284) // odd return type for operator->
//#pragma warning (disable : 4244) // possible loss of data
//#pragma warning (disable : 4521) ////Disable "multiple copy constructors specified"
//#pragma warning (disable : 4522)
//#pragma warning (disable : 4146)
//#pragma warning (disable : 4267) //conversion from 'X' to 'Y', possible loss of data
#pragma warning (disable : 4127) //conditional expression is constant
//#pragma warning (disable : 4706) //assignment within conditional expression
#pragma warning (disable : 4748) // /GS parameter on unactivated optimization functions

#endif