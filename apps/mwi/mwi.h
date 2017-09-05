/*
    Copyright (C) Anton Zagorskiy amberovsky@gmail.com
    Oyster-Telecom Laboratory

    Published under BSD License
*/

#ifndef _MWI_H_
#define _MWI_H_

#include "AmApi.h"

#include <string>

class MWI
    : public AmDynInvokeFactory
    , public AmDynInvoke
{
 private:
  static MWI*         _instance;
  static AmDynInvoke* MessageStorage;

  std::string presence_server;

  typedef struct
  {
    unsigned int new_msgs;
    unsigned int saved_msgs;
  } msg_info_struct;

  void getMsgInfo(const std::string& name, const std::string& domain,
                  msg_info_struct& msg_info);
  void publish(const std::string& name, const std::string& domain);

 public:
  MWI(const std::string& name);
  ~MWI();

  AmDynInvoke* getInstance() { return _instance; }

  int  onLoad();
  void invoke(const std::string& method, const AmArg& args, AmArg& ret);
};

#endif
