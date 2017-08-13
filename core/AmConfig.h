/*
 * Copyright (C) 2002-2003 Fhg Fokus
 *
 * This file is part of SEMS, a free SIP media server.
 *
 * SEMS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. This program is released under
 * the GPL with the additional exemption that compiling, linking,
 * and/or using OpenSSL is allowed.
 *
 * For a license to use the SEMS software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact iptel.org by e-mail at the following addresses:
 *    info@iptel.org
 *
 * SEMS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/** @file AmConfig.h */

#ifndef _AMCONFIG_H_
#define _AMCONFIG_H_

#include "AmAudio.h"
#include "AmDtmfDetector.h"
#include "AmSdp.h"
#include "AmSipDialog.h"
#include "AmUtils.h"

#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

/**
 * \brief holds the current configuration.
 *
 * This Structure holds the current configuration.
 */
struct AmConfig
{
  /** Name of the main configuration file. */
  static std::string ConfigurationFile;
  /** Path to the plug-in configuration files. */
  static std::string ModConfigPath;
  /** Path where the plug-ins are. */
  static std::string PlugInPath;
  /** semicolon separated list of plugins to load */
  static std::string LoadPlugins;
  /** semicolon separated list of plugins to exclude from loading */
  static std::string ExcludePlugins;
  /** semicolon separated list of payloads to exclude from loading */
  static std::string ExcludePayloads;
  // static unsigned int MaxRecordTime;
  /** log level */
  static int LogLevel;
  /** log to stderr */
  static bool LogStderr;

#ifndef DISABLE_DAEMON_MODE
  /** run the program in daemon mode? */
  static bool DaemonMode;
  /** PID file when in daemon mode */
  static std::string DaemonPidFile;
  /** set UID when in daemon mode */
  static std::string DaemonUid;
  /** set GID when in daemon mode */
  static std::string DaemonGid;
#endif

  static unsigned int MaxShutdownTime;

  struct IP_interface
  {
    std::string name;

    /** Used for binding socket */
    std::string LocalIP;

    /** Used in Contact-HF */
    std::string PublicIP;

    /** Network interface name and index */
    std::string  NetIf;
    unsigned int NetIfIdx;

    IP_interface();

    std::string getIP() { return PublicIP.empty() ? LocalIP : PublicIP; }
  };

  struct SIP_interface : public IP_interface
  {
    /** Used for binding SIP socket */
    unsigned int LocalPort;

    /** options for the signaling socket
     * (@see trsp_socket::socket_options)
     */
    unsigned int SigSockOpts;

    unsigned int tcp_connect_timeout;
    unsigned int tcp_idle_timeout;

    /** RTP interface index */
    int RtpInterface;

    SIP_interface();
  };

  struct RTP_interface : public IP_interface
  {
    /** Lowest local RTP port */
    int RtpLowPort;
    /** Highest local RTP port */
    int RtpHighPort;

    RTP_interface();

    int getNextRtpPort();

   private:
    int     next_rtp_port;
    AmMutex next_rtp_port_mutex;
  };

  static std::vector<SIP_interface> SIP_Ifs;
  static std::vector<RTP_interface *> RTP_Ifs;
  static std::map<std::string, unsigned short> SIP_If_names;
  static std::map<std::string, unsigned short> RTP_If_names;
  static std::map<std::string, unsigned short> LocalSIPIP2If;

  struct IPAddr
  {
    std::string addr;
    short       family;

    IPAddr(const std::string& addr, const short family)
        : addr(addr)
        , family(family)
    {
    }

    IPAddr(const IPAddr& ip)
        : addr(ip.addr)
        , family(ip.family)
    {
    }
  };

  struct SysIntf
  {
    std::string       name;
    std::list<IPAddr> addrs;
    // identical to those returned by SIOCGIFFLAGS
    unsigned int flags;
    unsigned int mtu;
  };

  static std::vector<SysIntf> SysIfs;

  static int insert_SIP_interface(const SIP_interface& intf);
  static int insert_SIP_interface_mapping(const SIP_interface& intf);
  static int insert_RTP_interface(RTP_interface* intf);
  static int finalizeIPConfig();

  static void dump_Ifs();

  /** number of session (signaling/application) processor threads */
  static int SessionProcessorThreads;
  /** number of media processor threads */
  static int MediaProcessorThreads;
  /** number of RTP receiver threads */
  static int RTPReceiverThreads;
  /** number of SIP server threads */
  static int SIPServerThreads;
  /** Outbound Proxy (optional, outgoing calls only) */
  static std::string OutboundProxy;
  /** force Outbound Proxy to be used for in dialog requests */
  static bool ForceOutboundProxy;
  /** force next hop IP[:port] */
  static std::string NextHop;
  /** use next hop only on 1st request within a dialog */
  static bool NextHop1stReq;
  /** update ruri-host to previously resolved IP:port on SIP auth */
  static bool ProxyStickyAuth;
  /** force the outbound network interface / short-circuit the routing table */
  static bool ForceOutboundIf;
  /** force comedia style remote address learning */
  static bool ForceSymmetricRtp;
  /** turn on SIP NAT handling (remote signaling address learning) */
  static bool SipNATHandling;
  /** use raw socket to send UDP packets (root permission required) */
  static bool UseRawSockets;
  /** Ignore Low CSeq on NOTIFY  - for RFC 3265 instead of 5057 */
  static bool IgnoreNotifyLowerCSeq;
  /** Server/User-Agent header (optional) */
  static std::string Signature;
  /** Value of Max-Forward header field for new requests */
  static unsigned int MaxForwards;
  /** If 200 OK reply should be limited to preferred codec only */
  static bool                     SingleCodecInOK;
  static std::vector<std::string> CodecOrder;

  enum ApplicationSelector
  {
    App_RURIUSER,
    App_RURIPARAM,
    App_APPHDR,
    App_MAPPING,
    App_SPECIFIED
  };

  /** "application" config value */
  static std::string Application;
  /** type of application selection (parsed from Application) */
  static ApplicationSelector AppSelect;

  /* this is regex->application mapping is used if  App_MAPPING */
  static RegexMappingVector AppMapping;

#ifdef WITH_ZRTP
  static bool enable_zrtp;
  static bool enable_zrtp_debuglog;
#endif

  static unsigned int SessionLimit;
  static unsigned int SessionLimitErrCode;
  static std::string  SessionLimitErrReason;

  static unsigned int OptionsSessionLimit;
  static unsigned int OptionsSessionLimitErrCode;
  static std::string  OptionsSessionLimitErrReason;

  static unsigned int CPSLimitErrCode;
  static std::string  CPSLimitErrReason;

  static bool AcceptForkedDialogs;

  static bool         ShutdownMode;
  static unsigned int ShutdownModeErrCode;
  static std::string  ShutdownModeErrReason;

  /** header containing the transcoder's outgoing codec statistics which should
   * be present in replies to OPTIONS requests */
  static std::string OptionsTranscoderOutStatsHdr;
  /** header containing the transcoder's incoming codec statistics which should
   * be present in replies to OPTIONS requests */
  static std::string OptionsTranscoderInStatsHdr;
  /** header containing the transcoder's outgoing codec statistics which should
   * be present in every message leaving server */
  static std::string TranscoderOutStatsHdr;
  /** header containing the transcoder's incoming codec statistics which should
   * be present in every message leaving server */
  static std::string TranscoderInStatsHdr;

  static bool        DumpConferenceStreams;
  static std::string DumpConferencePath;

  static Am100rel::State rel100;

  /** Time of no RTP after which Session is regarded as dead, 0 for no Timeout
   */
  static unsigned int DeadRtpTime;

  /** Ignore RTP Extension headers? */
  static bool IgnoreRTPXHdrs;

  static Dtmf::InbandDetectorType DefaultDTMFDetector;

  static bool IgnoreSIGCHLD;

  static bool IgnoreSIGPIPE;

  static bool LogSessions;

  static bool LogEvents;

  static int UnhandledReplyLoglevel;

  static AmAudio::ResamplingImplementationType ResamplingImplementationType;

  /** Read global configuration file and insert values. Maybe overwritten by
   * command line arguments */
  static int readConfiguration();

  /* following setters are used to fill config from config file */

  /** Setter for SIP Port, returns 0 on invalid value */
  static int setSIPPort(const std::string& port);
  /** Setter for SmtpServer Port, returns 0 on invalid value */
  static int setSmtpPort(const std::string& port);
  /** Setter for RtpLowPort, returns 0 on invalid value */
  static int setRtpLowPort(const std::string& port);
  /** Setter for RtpHighPort, returns 0 on invalid value */
  static int setRtpHighPort(const std::string& port);
  /** Setter for Loglevel, returns 0 on invalid value */
  static int setLogLevel(const std::string& level, bool apply = true);
  /** Setter for parameter stderr, returns 0 on invalid value */
  static int setLogStderr(const std::string& s, bool apply = true);

#ifndef DISABLE_DAEMON_MODE
  /** Setter for parameter DaemonMode, returns 0 on invalid value */
  static int setDaemonMode(const std::string& fork);
#endif

  /** Setter for parameter SessionProcessorThreads, returns 0 on invalid value
   */
  static int setSessionProcessorThreads(const std::string& th);
  /** Setter for parameter MediaProcessorThreads, returns 0 on invalid value */
  static int setMediaProcessorThreads(const std::string& th);
  /** Setter for parameter RTPReceiverThreads, returns 0 on invalid value */
  static int setRTPReceiverThreads(const std::string& th);
  /** Setter for parameter SIPServerThreads, returns 0 on invalid value */
  static int setSIPServerThreads(const std::string& th);
  /** Setter for parameter DeadRtpTime, returns 0 on invalid value */
  static int setDeadRtpTime(const std::string& drt);
};

/** Get the PF_INET address associated with the network interface */
std::string fixIface2IP(const std::string& dev_name, bool v6_for_sip = false);

#endif
