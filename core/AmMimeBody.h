#ifndef _AMMIMEBODY_H_
#define _AMMIMEBODY_H_

#include "AmArg.h"

#include <list>
#include <string>

struct AmContentType
{
  struct Param
  {
    enum Type
    {
      UNPARSED = 0,
      BOUNDARY,
      OTHER
    };

    Type        type;
    std::string name;
    std::string value;

    Param(const std::string& name, const std::string& value)
        : type(UNPARSED)
        , name(name)
        , value(value)
    {
    }

    int parseType();
  };

  typedef std::list<Param*> Params;

  std::string type;
  std::string subtype;
  Params      params;
  Param*      mp_boundary;

  AmContentType();
  AmContentType(const AmContentType& ct);
  ~AmContentType();

  const AmContentType& operator=(const AmContentType& r_ct);

  int parse(const std::string& ct);
  int parseParams(const char* c, const char* end);

  void setType(const std::string& t);
  void setSubType(const std::string& st);

  bool isType(const std::string& t) const;
  bool isSubType(const std::string& st) const;
  bool hasContentType(const std::string& content_type) const;

  /** get content-type without any parameters */
  std::string getStr() const;

  /** get content-type with parameters */
  std::string getHdr() const;

  /** Clear and free param list */
  void clearParams();

  /** set a random boundary string */
  void resetBoundary();
};

class AmMimeBody : public AmObject
{
 public:
  typedef std::list<AmMimeBody*> Parts;

 private:
  AmContentType  ct;
  std::string    hdrs;
  unsigned int   content_len;
  unsigned char* payload;

  Parts parts;

  void clearParts();
  void clearPart(Parts::iterator position);
  void clearPayload();

  int parseMultipart(const unsigned char* buf, unsigned int len);
  int findNextBoundary(unsigned char** beg, unsigned char** end);
  int parseSinglePart(unsigned char* buf, unsigned int len);

  void convertToMultipart();
  void convertToSinglepart();

 public:
  /** Empty constructor */
  AmMimeBody();

  /** Deep-copy constructor */
  AmMimeBody(const AmMimeBody& body);

  /** Destuctor */
  ~AmMimeBody();

  /** Deep copy operator */
  const AmMimeBody& operator=(const AmMimeBody& r_body);

  /** Parse a body (single & multi-part) */
  int parse(const std::string& content_type, const unsigned char* buf,
            unsigned int len);

  /** Set the payload of this body */
  void setPayload(const unsigned char* buf, unsigned int len);

  /** Set part headers (intended for sub-parts)*/
  void setHeaders(const std::string& hdrs);

  /**
   * Add a new part to this body, possibly
   * converting to multi-part if necessary.
   * @return a pointer to the new empty part.
   */
  AmMimeBody* addPart(const std::string& content_type);

  /**
   * Delete a body part, converting resulting body to single-part if necessary.
   */
  int deletePart(const std::string& content_type);

  /** Get content-type without any parameters */
  std::string getCTStr() const { return ct.getStr(); }

  /** Get content-type with parameters */
  std::string getCTHdr() const { return ct.getHdr(); }

  /** @return the list of sub-parts */
  const Parts& getParts() const { return parts; }

  /** @return the sub-part headers */
  const std::string& getHeaders() const { return hdrs; }

  /**
   * @return a pointer to the payload of this part.
   *         in case of multi-part, NULL is returned.
   */
  const unsigned char* getPayload() const { return payload; }

  /**
   * @return the payload length of this part.
   *         in case of multi-part, 0 is returned.
   */
  unsigned int getLen() const { return content_len; }

  /** @return true if no payload assigned and no sub-parts available */
  bool empty() const;

  /** @return true if this part has the provided content-type */
  bool isContentType(const std::string& content_type) const;

  /**
   * @return a pointer to a part of the coresponding
   *         content-type (if available).
   *         This could be a pointer to this body.
   */
  AmMimeBody* hasContentType(const std::string& content_type);

  /**
   * @return a const pointer to a part of the coresponding
   *         content-type (if available).
   *         This could be a pointer to this body.
   */
  const AmMimeBody* hasContentType(const std::string& content_type) const;

  /**
   * Print the body including sub-parts suitable for sending
   * within the body of a SIP message.
   */
  void print(std::string& buf) const;

  const AmContentType& getContentType() { return ct; }
  void setContentType(const AmContentType& _ct) { ct = _ct; }
  void addPart(const AmMimeBody& part)
  {
    parts.push_back(new AmMimeBody(part));
  }
};

#endif
