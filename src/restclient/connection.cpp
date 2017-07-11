/**
 * @file connection.cpp
 * @brief implementation of the connection class
 * @author Daniel Schauenberg <d@unwiredcouch.com>
 */

#include "restclient/connection.h"

#include "../client/cl_curl.h"
#include <cstring>
#include <string>
#include <iostream>
#include <map>
#include <stdexcept>

#include "restclient/restclient.h"
#include "restclient/helpers.h"
#include "restclient/version.h"

/**
 * @brief constructor for the Connection object
 *
 * @param baseUrl - base URL for the connection to use
 *
 */
RestClient::Connection::Connection(const std::string baseUrl)
                               : lastRequest(), headerFields() {
  this->curlHandle = qcurl_easy_init();
  if (!this->curlHandle) {
    throw std::runtime_error("Couldn't initialize curl handle");
  }
  this->baseUrl = baseUrl;
  this->timeout = 0;
  this->followRedirects = false;
}

RestClient::Connection::~Connection() {
  if (this->curlHandle) {
    qcurl_easy_cleanup(this->curlHandle);
  }
}

// getters/setters

/**
 * @brief get diagnostic information about the connection object
 *
 * @return RestClient::Connection::Info struct
 */
RestClient::Connection::Info
RestClient::Connection::GetInfo() {
  RestClient::Connection::Info ret;
  ret.baseUrl = this->baseUrl;
  ret.headers = this->GetHeaders();
  ret.timeout = this->timeout;
  ret.basicAuth.username = this->basicAuth.username;
  ret.basicAuth.password = this->basicAuth.password;
  ret.customUserAgent = this->customUserAgent;
  ret.lastRequest = this->lastRequest;

  return ret;
}

/**
 * @brief append a header to the internal map
 *
 * @param key for the header field
 * @param value for the header field
 *
 */
void
RestClient::Connection::AppendHeader(const std::string& key,
                                     const std::string& value) {
  this->headerFields[key] = value;
}

/**
 * @brief set the custom headers map. This will replace the currently
 * configured headers with the provided ones. If you want to add additional
 * headers, use AppendHeader()
 *
 * @param headers to set
 */
void
RestClient::Connection::SetHeaders(RestClient::HeaderFields headers) {
  this->headerFields = headers;
}

/**
 * @brief get all custom headers set on the connection
 *
 * @returns a RestClient::HeaderFields map containing the custom headers
 */
RestClient::HeaderFields
RestClient::Connection::GetHeaders() {
  return this->headerFields;
}

/**
 * @brief configure whether to follow redirects on this connection
 *
 * @param follow - boolean whether to follow redirects
 */
void
RestClient::Connection::FollowRedirects(bool follow) {
  this->followRedirects = follow;
}

/**
 * @brief set custom user agent for connection. This gets prepended to the
 * default restclient-cpp/RESTCLIENT_VERSION string
 *
 * @param userAgent - custom userAgent prefix
 *
 */
void
RestClient::Connection::SetUserAgent(const std::string& userAgent) {
  this->customUserAgent = userAgent;
}

/**
 * @brief set custom Certificate Authority (CA) path
 *
 * @param caInfoFilePath - The path to a file holding the certificates used to
 * verify the peer with. See CURLOPT_CAINFO
 *
 */
void
RestClient::Connection::SetCAInfoFilePath(const std::string& caInfoFilePath) {
  this->caInfoFilePath = caInfoFilePath;
}

/**
 * @brief get the user agent to add to the request
 *
 * @return user agent as std::string
 */
std::string
RestClient::Connection::GetUserAgent() {
  std::string prefix;
  if (this->customUserAgent.length() > 0) {
    prefix = this->customUserAgent + " ";
  }
  return std::string(prefix + "restclient-cpp/" + RESTCLIENT_VERSION);
}

/**
 * @brief set timeout for connection
 *
 * @param seconds - timeout in seconds
 *
 */
void
RestClient::Connection::SetTimeout(int seconds) {
  this->timeout = seconds;
}

/**
 * @brief set username and password for basic auth
 *
 * @param username
 * @param password
 *
 */
void
RestClient::Connection::SetBasicAuth(const std::string& username,
                                     const std::string& password) {
  this->basicAuth.username = username;
  this->basicAuth.password = password;
}

/**
 * @brief helper function to get called from the actual request methods to
 * prepare the curlHandle for transfer with generic options, perform the
 * request and record some stats from the last request and then reset the
 * handle with curl_easy_reset to its default state. This will keep things
 * like connections and session ID intact but makes sure you can change
 * parameters on the object for another request.
 *
 * @param uri URI to query
 * @param ret Reference to the Response struct that should be filled
 *
 * @return 0 on success and 1 on error
 */
RestClient::Response
RestClient::Connection::performCurlRequest(const std::string& uri) {
  // init return type
  RestClient::Response ret = {};

  std::string url = std::string(this->baseUrl + uri);
  std::string headerString;
  CURLcode res = CURLE_OK;
  curl_slist* headerList = NULL;

  // FIXME:
  // CURLOPT_FOLLOWLOCATION
  qcurl_easy_setopt(this->curlHandle, CURLOPT_FOLLOWLOCATION, url.c_str());

  /** set query URL */
  qcurl_easy_setopt(this->curlHandle, CURLOPT_URL, url.c_str());
  /** set callback function */
  qcurl_easy_setopt(this->curlHandle, CURLOPT_WRITEFUNCTION,
                   Helpers::write_callback);
  /** set data object to pass to callback function */
  qcurl_easy_setopt(this->curlHandle, CURLOPT_WRITEDATA, &ret);
  /** set the header callback function */
  qcurl_easy_setopt(this->curlHandle, CURLOPT_HEADERFUNCTION,
                   Helpers::header_callback);
  /** callback object for headers */
  qcurl_easy_setopt(this->curlHandle, CURLOPT_HEADERDATA, &ret);
  /** set http headers */
  for (HeaderFields::const_iterator it = this->headerFields.begin();
      it != this->headerFields.end(); ++it) {
    headerString = it->first;
    headerString += ": ";
    headerString += it->second;
    headerList = qcurl_slist_append(headerList, headerString.c_str());
  }
  qcurl_easy_setopt(this->curlHandle, CURLOPT_HTTPHEADER,
      headerList);

  // set basic auth if configured
  if (this->basicAuth.username.length() > 0) {
    std::string authString = std::string(this->basicAuth.username + ":" +
                                         this->basicAuth.password);
    qcurl_easy_setopt(this->curlHandle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    qcurl_easy_setopt(this->curlHandle, CURLOPT_USERPWD, authString.c_str());
  }
  /** set user agent */
  qcurl_easy_setopt(this->curlHandle, CURLOPT_USERAGENT,
                   this->GetUserAgent().c_str());

  // set timeout
  if (this->timeout) {
    qcurl_easy_setopt(this->curlHandle, CURLOPT_TIMEOUT, this->timeout);
    // dont want to get a sig alarm on timeout
    qcurl_easy_setopt(this->curlHandle, CURLOPT_NOSIGNAL, 1);
  }
  // set follow redirect
  if (this->followRedirects == true) {
    qcurl_easy_setopt(this->curlHandle, CURLOPT_FOLLOWLOCATION, 1L);
  }
  // if provided, supply CA path
  if (!this->caInfoFilePath.empty()) {
    qcurl_easy_setopt(this->curlHandle, CURLOPT_CAINFO,
                     this->caInfoFilePath.c_str());
  }
  res = qcurl_easy_perform(this->curlHandle);
  if (res != CURLE_OK) {
    if (res == CURLE_OPERATION_TIMEDOUT) {
      ret.code = res;
      ret.body = "Operation Timeout.";
    } else {
      ret.body = "Failed to query.";
      ret.code = -1;
    }
  } else {
    int64_t http_code = 0;
    qcurl_easy_getinfo(this->curlHandle, CURLINFO_RESPONSE_CODE, &http_code);
    ret.code = static_cast<int>(http_code);
  }

  qcurl_easy_getinfo(this->curlHandle, CURLINFO_TOTAL_TIME,
                    &this->lastRequest.totalTime);
  qcurl_easy_getinfo(this->curlHandle, CURLINFO_NAMELOOKUP_TIME,
                    &this->lastRequest.nameLookupTime);
  qcurl_easy_getinfo(this->curlHandle, CURLINFO_CONNECT_TIME,
                    &this->lastRequest.connectTime);
  qcurl_easy_getinfo(this->curlHandle, CURLINFO_APPCONNECT_TIME,
                    &this->lastRequest.appConnectTime);
  qcurl_easy_getinfo(this->curlHandle, CURLINFO_PRETRANSFER_TIME,
                    &this->lastRequest.preTransferTime);
  qcurl_easy_getinfo(this->curlHandle, CURLINFO_STARTTRANSFER_TIME,
                    &this->lastRequest.startTransferTime);
  qcurl_easy_getinfo(this->curlHandle, CURLINFO_REDIRECT_TIME,
                    &this->lastRequest.redirectTime);
  qcurl_easy_getinfo(this->curlHandle, CURLINFO_REDIRECT_COUNT,
                    &this->lastRequest.redirectCount);
  // free header list
  qcurl_slist_free_all(headerList);
  // reset curl handle
  qcurl_easy_reset(this->curlHandle);
  return ret;
}

/**
 * @brief HTTP GET method
 *
 * @param url to query
 *
 * @return response struct
 */
RestClient::Response
RestClient::Connection::get(const std::string& url) {
  return this->performCurlRequest(url);
}
/**
 * @brief HTTP POST method
 *
 * @param url to query
 * @param data HTTP POST body
 *
 * @return response struct
 */
RestClient::Response
RestClient::Connection::post(const std::string& url,
                             const std::string& data) {
  /** Now specify we want to POST data */
  qcurl_easy_setopt(this->curlHandle, CURLOPT_POST, 1L);
  /** set post fields */
  qcurl_easy_setopt(this->curlHandle, CURLOPT_POSTFIELDS, data.c_str());
  qcurl_easy_setopt(this->curlHandle, CURLOPT_POSTFIELDSIZE, data.size());

  return this->performCurlRequest(url);
}
/**
 * @brief HTTP PUT method
 *
 * @param url to query
 * @param data HTTP PUT body
 *
 * @return response struct
 */
RestClient::Response
RestClient::Connection::put(const std::string& url,
                            const std::string& data) {
  /** initialize upload object */
  RestClient::Helpers::UploadObject up_obj;
  up_obj.data = data.c_str();
  up_obj.length = data.size();

  /** Now specify we want to PUT data */
  qcurl_easy_setopt(this->curlHandle, CURLOPT_PUT, 1L);
  qcurl_easy_setopt(this->curlHandle, CURLOPT_UPLOAD, 1L);
  /** set read callback function */
  qcurl_easy_setopt(this->curlHandle, CURLOPT_READFUNCTION,
                   RestClient::Helpers::read_callback);
  /** set data object to pass to callback function */
  qcurl_easy_setopt(this->curlHandle, CURLOPT_READDATA, &up_obj);
  /** set data size */
  qcurl_easy_setopt(this->curlHandle, CURLOPT_INFILESIZE,
                     static_cast<int64_t>(up_obj.length));

  return this->performCurlRequest(url);
}
/**
 * @brief HTTP DELETE method
 *
 * @param url to query
 *
 * @return response struct
 */
RestClient::Response
RestClient::Connection::del(const std::string& url) {
  /** we want HTTP DELETE */
  const char* http_delete = "DELETE";

  /** set HTTP DELETE METHOD */
  qcurl_easy_setopt(this->curlHandle, CURLOPT_CUSTOMREQUEST, http_delete);

  return this->performCurlRequest(url);
}

