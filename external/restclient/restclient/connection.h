/**
 * @file connection.h
 * @brief header definitions for restclient-cpp connection class
 * @author Daniel Schauenberg <d@unwiredcouch.com>
 * @version
 * @date 2010-10-11
 */

#ifndef INCLUDE_RESTCLIENT_CPP_CONNECTION_H_
#define INCLUDE_RESTCLIENT_CPP_CONNECTION_H_

#include <string>
#include <map>
#include <cstdlib>

#include "restclient.h"
#include "version.h"

typedef void CURL;
/**
 * @brief namespace for all RestClient definitions
 */
namespace RestClient {

/**
  * @brief Connection object for advanced usage
  */
class Connection {
 public:
    /**
      *  @struct RequestInfo
      *  @brief holds some diagnostics information
      *  about a request
      *  @var RequestInfo::totalTime
      *  Member 'totalTime' contains the total time of the last request in
      *  seconds Total time of previous transfer. See CURLINFO_TOTAL_TIME
      *  @var RequestInfo::nameLookupTime
      *  Member 'nameLookupTime' contains the time spent in DNS lookup in
      *  seconds Time from start until name resolving completed. See
      *  CURLINFO_NAMELOOKUP_TIME
      *  @var RequestInfo::connectTime
      *  Member 'connectTime' contains the time it took until Time from start
      *  until remote host or proxy completed. See CURLINFO_CONNECT_TIME
      *  @var RequestInfo::appConnectTime
      *  Member 'appConnectTime' contains the time from start until SSL/SSH
      *  handshake completed. See CURLINFO_APPCONNECT_TIME
      *  @var RequestInfo::preTransferTime
      *  Member 'preTransferTime' contains the total time from start until
      *  just before the transfer begins. See CURLINFO_PRETRANSFER_TIME
      *  @var RequestInfo::startTransferTime
      *  Member 'startTransferTime' contains the total time from start until
      *  just when the first byte is received. See CURLINFO_STARTTRANSFER_TIME
      *  @var RequestInfo::redirectTime
      *  Member 'redirectTime' contains the total time taken for all redirect
      *  steps before the final transfer. See CURLINFO_REDIRECT_TIME
      *  @var RequestInfo::redirectCount
      *  Member 'redirectCount' contains the number of redirects followed. See
      *  CURLINFO_REDIRECT_COUNT
      */
    typedef struct {
        double totalTime;
        double nameLookupTime;
        double connectTime;
        double appConnectTime;
        double preTransferTime;
        double startTransferTime;
        double redirectTime;
        int redirectCount;
      } RequestInfo;
    /**
      *  @struct Info
      *  @brief holds some diagnostics information
      *  about the connection object it came from
      *  @var Info::baseUrl
      *  Member 'baseUrl' contains the base URL for the connection object
      *  @var Info::headers
      *  Member 'headers' contains the HeaderFields map
      *  @var Info::timeout
      *  Member 'timeout' contains the configured timeout
      *  @var Info::followRedirects
      *  Member 'followRedirects' contains whether or not to follow redirects
      *  @var Info::basicAuth
      *  Member 'basicAuth' contains information about basic auth
      *  @var basicAuth::username
      *  Member 'username' contains the basic auth username
      *  @var basicAuth::password
      *  Member 'password' contains the basic auth password
      *  @var Info::customUserAgent
      *  Member 'customUserAgent' contains the custom user agent
      *  @var Info::lastRequest
      *  Member 'lastRequest' contains metrics about the last request
      */
    typedef struct {
      std::string baseUrl;
      RestClient::HeaderFields headers;
      int timeout;
      bool followRedirects;
      struct {
        std::string username;
        std::string password;
      } basicAuth;
      std::string customUserAgent;
      RequestInfo lastRequest;
    } Info;


    explicit Connection(const std::string baseUrl);
    ~Connection();

    // Instance configuration methods
    // configure basic auth
    void SetBasicAuth(const std::string& username,
                      const std::string& password);

    // set connection timeout to seconds
    void SetTimeout(int seconds);

    // set whether to follow redirects
    void FollowRedirects(bool follow);

    // set custom user agent
    // (this will result in the UA "foo/cool restclient-cpp/VERSION")
    void SetUserAgent(const std::string& userAgent);

    // set the Certificate Authority (CA) Info which is the path to file holding
    // certificates to be used to verify peers. See CURLOPT_CAINFO
    void SetCAInfoFilePath(const std::string& caInfoFilePath);

    std::string GetUserAgent();

    RestClient::Connection::Info GetInfo();

    // set headers
    void SetHeaders(RestClient::HeaderFields headers);

    // get headers
    RestClient::HeaderFields GetHeaders();

    // append additional headers
    void AppendHeader(const std::string& key,
                      const std::string& value);


    // Basic HTTP verb methods
    RestClient::Response get(const std::string& uri);
    RestClient::Response post(const std::string& uri,
                              const std::string& data);
    RestClient::Response put(const std::string& uri,
                             const std::string& data);
    RestClient::Response del(const std::string& uri);

 private:
    CURL* curlHandle;
    std::string baseUrl;
    RestClient::HeaderFields headerFields;
    int timeout;
    bool followRedirects;
    struct {
      std::string username;
      std::string password;
    } basicAuth;
    std::string customUserAgent;
    std::string caInfoFilePath;
    RequestInfo lastRequest;
    RestClient::Response performCurlRequest(const std::string& uri);
};
};  // namespace RestClient

#endif  // INCLUDE_RESTCLIENT_CPP_CONNECTION_H_
