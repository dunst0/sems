#ifndef _TRANS_TABLE_H_
#define _TRANS_TABLE_H_

#include "cstring.h"
#include "hash_table.h"
#include "sip_trans.h"

class trans_bucket : public ht_bucket<sip_trans>
{
  trans_bucket(unsigned long id);
  ~trans_bucket();

  friend class hash_table<trans_bucket>;

 public:
  typedef ht_bucket<sip_trans>::value_list trans_list;

  // Match a request to UAS/UAC transactions
  // in this bucket
  sip_trans* match_request(sip_msg* msg, unsigned int ttype);

  // Match a PRACK request against transactions
  // in this bucket
  sip_trans* match_1xx_prack(sip_msg* msg);

  // Match a reply to UAC transactions
  // in this bucket
  sip_trans* match_reply(sip_msg* msg);

  // Find the latest UAC transaction matching dialog_id
  sip_trans* find_uac_trans(const cstring& dialog_id, unsigned int inv_cseq);

  // Add a new transaction using provided message and type
  sip_trans* add_trans(sip_msg* msg, unsigned int ttype);

  // Append the provided transaction to this bucket
  void append(sip_trans* t);

 private:
  sip_trans* match_200_ack(sip_trans* t, sip_msg* msg);
};

trans_bucket* get_trans_bucket(const cstring& callid, const cstring& cseq_num);
trans_bucket* get_trans_bucket(unsigned int h);

unsigned int hash(const cstring& ci, const cstring& cs);

void compute_branch(char* branch, const cstring& callid, const cstring& cseq);

void compute_sl_to_tag(char* to_tag, const sip_msg* msg);

void dumps_transactions();

#endif
