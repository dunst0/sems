#ifndef _MSGSTORAGE_H_
#define _MSGSTORAGE_H_

#include "AmApi.h"

#include <map>

class MsgStorage
    : public AmDynInvokeFactory
    , public AmDynInvoke
{
  static MsgStorage* _instance;

  std::string msg_dir;

  typedef std::map<AmDynInvoke*, std::string> Listeners;

  Listeners listeners;
  AmMutex   listeners_mut;

  int msg_new(std::string domain, std::string user, std::string msg_name,
              FILE* data);
  void msg_get(std::string domain, std::string user, std::string msg_name,
               AmArg& ret);
  int msg_markread(std::string domain, std::string user, std::string msg_name);
  int msg_delete(std::string domain, std::string user, std::string msg_name);

  void userdir_open(std::string domain, std::string user, AmArg& ret);
  int userdir_close(std::string domain, std::string user);
  void userdir_getcount(std::string domain, std::string user, AmArg& ret);

  void events_subscribe(AmDynInvoke* event_sink, std::string method);
  void events_unsubscribe(AmDynInvoke* event_sink);

  void event_notify(const std::string& domain, const std::string& user,
                    const std::string& event);

  inline void filecopy(FILE* ifp, FILE* ofp);

 public:
  MsgStorage(const std::string& name);
  ~MsgStorage();

  AmDynInvoke* getInstance() { return _instance; }

  int  onLoad();
  void invoke(const std::string& method, const AmArg& args, AmArg& ret);
};

#endif
