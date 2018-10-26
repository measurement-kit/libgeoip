// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKGEOIP_H
#define MEASUREMENT_KIT_MKGEOIP_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/// mkgeoip_ubuntu_request_get_url returns the URL for querying the
/// geolocation service provided by Ubuntu. This function MAY return NULL
/// in case of serious internal error.
const char *mkgeoip_ubuntu_request_get_url(void);

/// mkgeoip_ubuntu_request_get_method returns the method for querying the
/// geolocation service provided by Ubuntu. This function MAY return a NULL
/// pointer in case of serious internal error.
const char *mkgeoip_ubuntu_request_get_method(void);

/// mkgeoip_ubuntu_response_t is a response returned by the
/// geolocation service provided by Ubuntu.
typedef struct mkgeoip_ubuntu_response mkgeoip_ubuntu_response_t;

/// mkgeoip_ubuntu_response_new creates a new ubuntu geolocation response.
mkgeoip_ubuntu_response_t *mkgeoip_ubuntu_response_new(void);

/// mkgeoip_ubuntu_response_set_status_code sets the status code received
/// by the geolocation service provided by Ubuntu.
void mkgeoip_ubuntu_response_set_status_code(
    mkgeoip_ubuntu_response_t *response,
    int64_t status_code);

/// mkgeoip_ubuntu_response_set_content_type sets the content type received
/// by the geolocation service provided by Ubuntu.
void mkgeoip_ubuntu_response_set_content_type(
    mkgeoip_ubuntu_response_t *response,
    const char *content_type);

/// mkgeoip_ubuntu_response_set_body_binary sets the binary response body
/// received by the geolocation service provided by Ubuntu.
void mkgeoip_ubuntu_response_set_body_binary(
    mkgeoip_ubuntu_response_t *response,
    const uint8_t *base, size_t count);

/// mkgeoip_ubuntu_response_parse parses the fields previously set inside
/// of @p response to extract the IP address. Returns true on success, and
/// false on failure. On failure, inspect the original HTTP response body
/// and status code to understand what went wrong.
int64_t mkgeoip_ubuntu_response_parse(mkgeoip_ubuntu_response_t *response);

/// mkgeoip_ubuntu_response_get_probe_ip returns the probe IP previously
/// parsed using mkgeoip_ubuntu_response_parse. If parsing did not succeed
/// it returns an empty string. MAY return NULL on internal error. The
/// returned string is valid as long as @p response is alive and no other
/// API is invoked on the @p response instance.
const char *mkgeoip_ubuntu_response_get_probe_ip(
    mkgeoip_ubuntu_response_t *response);

/// mkgeoip_ubuntu_response_delete destroys @p response.
void mkgeoip_ubuntu_response_delete(mkgeoip_ubuntu_response_t *response);

/// mkgeoip_mmdb_t is an open MMDB database.
typedef struct mkgeoip_mmdb mkgeoip_mmdb_t;

/// mkgeoip_mmdb_open opens the database at @p path and returns the
/// database instance on success, or NULL on failure.
mkgeoip_mmdb_t *mkgeoip_mmdb_open(const char *path);

/// mkgeoip_mmdb_lookup_cc returns the country code of @p ip using the
/// @p mmdb database, or NULL in case of error. The returned string will
/// be valid until @p mmdb is valid _and_ you don't call other lookup
/// APIs using the same @p mmdb instance.
const char *mkgeoip_mmdb_lookup_cc(mkgeoip_mmdb_t *mmdb, const char *ip);

/// mkgeoip_mmdb_lookup_asn is like mkgeoip_mmdb_lookup_cc but returns
/// the ASN on success and zero on failure.
int64_t mkgeoip_mmdb_lookup_asn(mkgeoip_mmdb_t *mmdb, const char *ip);

/// mkgeoip_mmdb_lookup_org is like mkgeoip_mmdb_lookup_cc but returns
/// the organization bound to @p ip on success, NULL on failure. The
/// returned string will be valid until @p mmdb is valid _and_ you don't
/// call other lookup APIs using the same @p mmdb instance.
const char *mkgeoip_mmdb_lookup_org(mkgeoip_mmdb_t *mmdb, const char *ip);

/// mkgeoip_mmdb_close closes @p mmdb.
void mkgeoip_mmdb_close(mkgeoip_mmdb_t *mmdb);

#ifdef __cplusplus
}  // extern "C"

#include <memory>
#include <string>

/// mkgeoip_ubuntu_response_deleter is a deleter for mkgeoip_ubuntu_response_t.
struct mkgeoip_ubuntu_response_deleter {
  void operator()(mkgeoip_ubuntu_response_t *p) {
    mkgeoip_ubuntu_response_delete(p);
  }
};

/// mkgeoip_ubuntu_response_uptr is a unique pointer to mkgeoip_ubuntu_response_t.
using mkgeoip_ubuntu_response_uptr = std::unique_ptr<
    mkgeoip_ubuntu_response_t, mkgeoip_ubuntu_response_deleter>;

/// mkgeoip_mmdb_deleter is a deleter for mkgeoip_mmdb_t.
struct mkgeoip_mmdb_deleter {
  void operator()(mkgeoip_mmdb_t *p) { mkgeoip_mmdb_close(p); }
};

/// mkgeoip_mmdb_uptr is a unique pointer to mkgeoip_mmdb_t.
using mkgeoip_mmdb_uptr = std::unique_ptr<mkgeoip_mmdb_t, mkgeoip_mmdb_deleter>;

/// mkgeoip_ubuntu_response_movein_body moves @p body as the @p response body
/// and returns true on success and false on failure.
int64_t mkgeoip_ubuntu_response_movein_body(
    mkgeoip_ubuntu_response_t *response,
    std::string &&body);

// By default the implementation is not included. You can force it being
// included by providing the following definition to the compiler.
//
// If you're just into understanding the API, you can stop reading here.
#ifdef MKGEOIP_INLINE_IMPL

#include <ctype.h>

#include <functional>
#include <vector>

#include <maxminddb.h>

#include "mkdata.h"

const char *mkgeoip_ubuntu_request_get_url() {
  return "https://geoip.ubuntu.com/lookup";
}

const char *mkgeoip_ubuntu_request_get_method() {
  return "GET";
}

struct mkgeoip_ubuntu_response {
  std::string content_type;
  std::string body;
  int64_t status_code = 0;
  std::string probe_ip;
};

mkgeoip_ubuntu_response_t *mkgeoip_ubuntu_response_new() {
  return new mkgeoip_ubuntu_response_t;
}

void mkgeoip_ubuntu_response_set_status_code(
    mkgeoip_ubuntu_response_t *response,
    int64_t status_code) {
  if (response != nullptr) response->status_code = status_code;
}

void mkgeoip_ubuntu_response_set_content_type(
    mkgeoip_ubuntu_response_t *response,
    const char *content_type) {
  if (response != nullptr && content_type != nullptr) {
    response->content_type = content_type;
  }
}

void mkgeoip_ubuntu_response_set_body_binary(
    mkgeoip_ubuntu_response_t *response,
    const uint8_t *base, size_t count) {
  if (response != nullptr && base != nullptr && count > 0) {
    response->body = std::string{(const char *)base, count};
  }
}

int64_t mkgeoip_ubuntu_response_parse(mkgeoip_ubuntu_response_t *response) {
  if (response == nullptr) return false;
  response->probe_ip = "";  // reset
  if (response->status_code != 200) return false;
  if (response->content_type != "text/xml" &&
      response->content_type != "application/xml") {
    return false;
  }
  std::string body = response->body;
  {
    mkdata_uptr data{mkdata_new()};
    mkdata_movein(data.get(), std::move(body));
    if (!mkdata_contains_valid_utf8(data.get())) return false;
    body = mkdata_moveout(data.get());
  }
  {
    static const std::string open_tag = "<Ip>";
    static const std::string close_tag = "</Ip>";
    auto pos = body.find(open_tag);
    if (pos == std::string::npos) return false;
    body = body.substr(pos + open_tag.size());  // Find EOS in the worst case
    pos = body.find(close_tag);
    if (pos == std::string::npos) return false;
    body = body.substr(0, pos);
    for (auto ch : body) {
      if (isspace(ch)) continue;
      ch = (char)tolower(ch);
      auto ok = isdigit(ch) || (ch >= 'a' && ch <= 'f') || ch == '.' || ch == ':';
      if (!ok) return false;
      response->probe_ip += ch;
    }
  }
  return true;
}

const char *mkgeoip_ubuntu_response_get_probe_ip(
    mkgeoip_ubuntu_response_t *response) {
  return (response != nullptr) ? response->probe_ip.c_str() : nullptr;
}

void mkgeoip_ubuntu_response_delete(mkgeoip_ubuntu_response_t *response) {
  delete response;
}

struct mkgeoip_mmdb_s_deleter {
  void operator()(MMDB_s *p) {
    MMDB_close(p);
    delete p;
  }
};

using mkgeoip_mmdb_s_uptr = std::unique_ptr<MMDB_s, mkgeoip_mmdb_s_deleter>;

struct mkgeoip_mmdb {
  mkgeoip_mmdb_s_uptr mmdbs;
  std::string saved_string;
};

#ifndef MKGEOIP_MMDB_OPEN
// MKGEOIP_MMDB_OPEN allows to mock MMDB_open
#define MKGEOIP_MMDB_OPEN MMDB_open
#endif

mkgeoip_mmdb_t *mkgeoip_mmdb_open(const char *path) {
  if (path == nullptr) return nullptr;
  mkgeoip_mmdb_uptr mmdb{new mkgeoip_mmdb_t};
  mmdb->mmdbs.reset(new MMDB_s);
  if (MKGEOIP_MMDB_OPEN(path, MMDB_MODE_MMAP, mmdb->mmdbs.get()) != 0) {
    return nullptr;
  }
  return mmdb.release();
}

#ifndef MKGEOIP_MMDB_GET_VALUE
// MKGEOIP_MMDB_GET_VALUE allows to mock MMDB_get_value
#define MKGEOIP_MMDB_GET_VALUE MMDB_get_value
#endif

#ifndef MKGEOIP_MMDB_LOOKUP_STRING
// MKGEOIP_MMDB_LOOKUP_STRING allows to mock MMDB_lookup_string
#define MKGEOIP_MMDB_LOOKUP_STRING MMDB_lookup_string
#endif

static void mkgeoip_lookup_mmdb(
    MMDB_s *mmdbp, const std::string &ip,
    std::function<void(MMDB_entry_s *)> fun) {
  auto gai_error = 0;
  auto mmdb_error = 0;
  auto record = MKGEOIP_MMDB_LOOKUP_STRING(mmdbp, ip.c_str(), &gai_error,
                                           &mmdb_error);
  if (gai_error == 0 && mmdb_error == 0 && record.found_entry == true) {
    fun(&record.entry);
  }
}

const char *mkgeoip_mmdb_lookup_cc(mkgeoip_mmdb_t *mmdb, const char *ip) {
  if (mmdb == nullptr || ip == nullptr) {
    return nullptr;
  }
  const char *rv = nullptr;
  mkgeoip_lookup_mmdb(
      mmdb->mmdbs.get(), ip, [&](MMDB_entry_s *entry) {
        MMDB_entry_data_s data{};
        auto mmdb_error = MKGEOIP_MMDB_GET_VALUE(
            entry, &data, "registered_country", "iso_code", nullptr);
        if (mmdb_error != 0) return;
        if (!data.has_data || data.type != MMDB_DATA_TYPE_UTF8_STRING) {
          return;
        }
        mmdb->saved_string = std::string{data.utf8_string, data.data_size};
        rv = mmdb->saved_string.c_str();
      });
  return rv;
}

int64_t mkgeoip_mmdb_lookup_asn(mkgeoip_mmdb_t *mmdb, const char *ip) {
  if (mmdb == nullptr || ip == nullptr) {
    return 0;
  }
  int64_t rv;
  mkgeoip_lookup_mmdb(
      mmdb->mmdbs.get(), ip, [&](MMDB_entry_s *entry) {
        MMDB_entry_data_s data{};
        auto mmdb_error = MKGEOIP_MMDB_GET_VALUE(
            entry, &data, "autonomous_system_number", nullptr);
        if (mmdb_error != 0) return;
        if (!data.has_data || data.type != MMDB_DATA_TYPE_UINT32) {
          return;
        }
        rv = data.uint32;
      });
  return rv;
}

const char *mkgeoip_mmdb_lookup_org(mkgeoip_mmdb_t *mmdb, const char *ip) {
  if (mmdb == nullptr || ip == nullptr) {
    return nullptr;
  }
  const char *rv = nullptr;
  mkgeoip_lookup_mmdb(
      mmdb->mmdbs.get(), ip, [&](MMDB_entry_s *entry) {
        MMDB_entry_data_s data{};
        auto mmdb_error = MKGEOIP_MMDB_GET_VALUE(
            entry, &data, "autonomous_system_organization", nullptr);
        if (mmdb_error != 0) return;
        if (!data.has_data || data.type != MMDB_DATA_TYPE_UTF8_STRING) {
          return;
        }
        mmdb->saved_string = std::string{data.utf8_string, data.data_size};
        rv = mmdb->saved_string.c_str();
      });
  return rv;
}

void mkgeoip_mmdb_close(mkgeoip_mmdb_t *mmdb) { delete mmdb; }

int64_t mkgeoip_ubuntu_response_movein_body(
    mkgeoip_ubuntu_response_t *response,
    std::string &&body) {
  if (response == nullptr) return false;
  std::swap(response->body, body);
  return true;
}

#endif  // MKGEOIP_INLINE_IMPL
#endif  // __cplusplus
#endif  // MEASUREMENT_KIT_MKGEOIP_H
