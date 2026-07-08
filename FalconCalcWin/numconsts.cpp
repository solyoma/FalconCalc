#include "stdafx_zoli.h"
#ifndef QT_CORE_LIB
	#include "stdafx_lc.h"
#endif

#include "numdefs.h"

#include "numconsts.h"

namespace numeric {

#ifndef _NUMCONSTANTS_DEFINED
// but this must be compiled only once: a single global object is created
  #define _NUMCONSTANTS_DEFINED
  const NumConstants constants;
#endif

  // end of numeric
}
