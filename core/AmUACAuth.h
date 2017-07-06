#ifndef _AMUACAUTH_H_
#define _AMUACAUTH_H_

#include "AmArg.h"
#include "AmSession.h"
#include "ampi/UACAuthAPI.h"

class AmUACAuth
{
 protected:
  AmUACAuth();
  ~AmUACAuth();

 public:
  /**
     unpack UAC auth credentials from an AmArg
     @return UACAuthCred object, NULL if failed
  */

  static UACAuthCred* unpackCredentials(const AmArg& arg);

  /**
     enable SIP UAC authentication
     @return true if successful
  */
  static bool enable(AmSession* s);
};

#endif
