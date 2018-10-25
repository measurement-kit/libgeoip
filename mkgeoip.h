// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKGEOIP_H
#define MEASUREMENT_KIT_MKGEOIP_H

#include "mkcurl.h"

#ifdef __cplusplus
extern "C" {
#endif

/// mkgeoip_t is a GeoIP resolver.
typedef struct mkgeoip mkgeoip_t;

/// mkgeoip_new creates a new GeoIP resolver.
mkgeoip_t *mkgeoip_new(void);

/// mkgeoip_iplookup performs a IP lookup using the configured GeoIP
/// resolver and returning the corresponding response. It will set the
/// URL to point to a specific backend service, but it should not touch
/// any other @p request field. Returns a boolean indicating success.
int64_t mkgeoip_iplookup(mkgeoip_t *p, mkcurl_request_t *request);

/// mkgeoip_get_response returns a copy of the response received by
/// mkgeoip_iplookup, or NULL in case of unexpected failure.
mkcurl_response_t *mkgeoip_get_response(mkgeoip_t *p);

/// mkgeoip_get_probe_ip returns the probe IP discovered using
/// mkgeoip_iplookup. Returns NULL if mkgeoip_iplookup has not been called or
/// there has been an error. The returned string will be invalidated by
/// subsequent mkgeoip_lookup calls / as well as by calling mkgeoip_delete.
const char *mkgeoip_get_probe_ip(mkgeoip_t *p);

/// mkgeoip_mmdb_query_probe_cc queries the @p mmdb_country_path MMDB database
/// to get the country code corresponding to @p probe_ip. Returns a country
/// code on success, and NULL in case of failure. The returned string will be
/// invalidated by calling mkgeoip_mmdb_query_probe_cc again or by calling the
/// mkgeoip_delete destructor of @p p.
const char *mkgeoip_mmdb_query_probe_cc(mkgeoip_t *p, const char *probe_ip,
                                        const char *mmdb_country_path);

/// mkgeoip_mmdb_query_probe_asn is like mkgeoip_mmdb_query_probe_cc except
/// that it returns the ASN associated to the @p probe_ip.
const char *mkgeoip_mmdb_query_probe_asn(mkgeoip_t *p, const char *probe_ip,
                                         const char *mmdb_asn_path);

/// mkgeoip_mmdb_query_probe_netname is like mkgeoip_mmdb_query_probe_cc except
/// that it returns the name of the ASN associated to the @p probe_ip.
const char *mkgeoip_mmdb_query_probe_netname(mkgeoip_t *p, const char *probe_ip,
                                             const char *mmdb_asn_path);

/// mkgeoip_delete destroys a @p p instance.
void mkgeoip_delete(mkgeoip_t *p);

#ifdef __cplusplus
}  // extern "C"

#include <memory>

/// mkgeoip_deleter is a deleter for mkgeoip_t.
struct mkgeoip_deleter {
  void operator()(mkgeoip_t *p) { mkgeoip_delete(p); }
};

/// mkgeoip_uptr is a unique pointer to mkgeoip_t.
using mkgeoip_uptr = std::unique_ptr<mkgeoip_t, mkgeoip_deleter>;

// By default the implementation is not included. You can force it being
// included by providing the following definition to the compiler.
//
// If you're just into understanding the API, you can stop reading here.
#ifdef MKGEOIP_INLINE_IMPL

#include <ctype.h>

#include <functional>
#include <string>
#include <vector>

#include <maxminddb.h>

struct mkgeoip {
  std::string probe_ip;
  std::string probe_asn;
  std::string probe_cc;
  std::string probe_netname;
  mkcurl_response_uptr response;
};

mkgeoip_t *mkgeoip_new() { return new mkgeoip_t; }

// mkgeoip_parse_ip parse the page returned by https://geoip.ubuntu.com
static bool mkgeoip_parse_ip(std::string &&s, std::string *rv);

#ifndef MKGEOIP_MKCURL_PERFORM
// MKGEOIP_MKCURL_PERFORM allows to mock mkcurl_perform.
#define MKGEOIP_MKCURL_PERFORM mkcurl_perform
#endif

#ifndef MKGEOIP_MKCURL_RESPONSE_GET_BODY_BINARY_V2
// MKGEOIP_MKCURL_RESPONSE_GET_BODY_BINARY_V2 allows to mock
// mkcurl_response_get_body_binary_v2
#define MKGEOIP_MKCURL_RESPONSE_GET_BODY_BINARY_V2  \
  mkcurl_response_get_body_binary_v2
#endif

int64_t mkgeoip_iplookup(mkgeoip_t *p, mkcurl_request_t *request) {
  if (p == nullptr || request == nullptr) return false;
  mkcurl_request_set_url(request, "https://geoip.ubuntu.com/lookup");
  p->response.reset(MKGEOIP_MKCURL_PERFORM(request));
  if (p->response == nullptr) return false;
  if (mkcurl_response_get_error(p->response.get()) != 0) {
    return false;
  }
  if (mkcurl_response_get_status_code(p->response.get()) != 200) {
    return false;
  }
  std::string body;
  {
    const uint8_t *base = nullptr;
    size_t count = 0;
    if (!MKGEOIP_MKCURL_RESPONSE_GET_BODY_BINARY_V2(
            p->response.get(), &base, &count) ||
        base == nullptr || count <= 0) {
      return false;
    }
    body = std::string{(char *)base, count};
  }
  return mkgeoip_parse_ip(std::move(body), &p->probe_ip);
}

mkcurl_response_t *mkgeoip_get_response(mkgeoip_t *p) {
  return (p != nullptr) ? mkcurl_response_copy(p->response.get()) : nullptr;
}

const char *mkgeoip_get_probe_ip(mkgeoip_t *p) {
  return (p != nullptr) ? p->probe_ip.c_str() : nullptr;
}

// mkgeoip_lookup_cc searches for the country code of an IP.
static bool mkgeoip_lookup_cc(
    const std::string &path, const std::string &ip, std::string *out);

const char *mkgeoip_mmdb_query_probe_cc(mkgeoip_t *p, const char *probe_ip,
                                        const char *mmdb_country_path) {
  if (probe_ip == nullptr || p == nullptr || mmdb_country_path == nullptr) {
    return nullptr;
  }
  if (!mkgeoip_lookup_cc(mmdb_country_path, probe_ip, &p->probe_cc)) {
    return nullptr;
  }
  return p->probe_cc.c_str();
}

// mkgeoip_lookup_asn searches for the ASN of an IP.
static bool mkgeoip_lookup_asn(
    const std::string &path, const std::string &ip, std::string *out);

const char *mkgeoip_mmdb_query_probe_asn(mkgeoip_t *p, const char *probe_ip,
                                         const char *mmdb_asn_path) {
  if (probe_ip == nullptr || p == nullptr || mmdb_asn_path == nullptr) {
    return nullptr;
  }
  if (!mkgeoip_lookup_asn(mmdb_asn_path, probe_ip, &p->probe_asn)) {
    return nullptr;
  }
  return p->probe_asn.c_str();
}

// mkgeoip_lookup_asn searches for the ASN name of an IP.
static bool mkgeoip_lookup_netname(
    const std::string &path, const std::string &ip, std::string *out);

const char *mkgeoip_mmdb_query_probe_netname(mkgeoip_t *p, const char *probe_ip,
                                             const char *mmdb_asn_path) {
  if (probe_ip == nullptr || p == nullptr || mmdb_asn_path == nullptr) {
    return nullptr;
  }
  if (!mkgeoip_lookup_netname(mmdb_asn_path, probe_ip, &p->probe_netname)) {
    return nullptr;
  }
  return p->probe_netname.c_str();
}

void mkgeoip_delete(mkgeoip_t *p) { delete p; }

static bool mkgeoip_parse_ip(std::string &&input, std::string *rv) {
  if (rv == nullptr) return false;
  static const std::string open_tag = "<Ip>";
  static const std::string close_tag = "</Ip>";
  auto pos = input.find(open_tag);
  if (pos == std::string::npos) return false;
  input = input.substr(pos + open_tag.size());  // Find EOS in the worst case
  pos = input.find(close_tag);
  if (pos == std::string::npos) return false;
  input = input.substr(0, pos);
  for (auto ch : input) {
    if (isspace(ch)) continue;
    ch = (char)tolower(ch);
    auto ok = isdigit(ch) || (ch >= 'a' && ch <= 'f') || ch == '.' || ch == ':';
    if (!ok) return false;
    *rv += ch;
  }
  return true;
}

// mkgeoip_lookup_mmdb is the generic MMDB lookup function
static bool mkgeoip_lookup_mmdb(
    const std::string &path, const std::string &ip,
    std::function<bool(MMDB_entry_s *)> fun);

#ifndef MKGEOIP_MMDB_GET_VALUE
// MKGEOIP_MMDB_GET_VALUE allows to mock MMDB_get_value
#define MKGEOIP_MMDB_GET_VALUE MMDB_get_value
#endif

#ifndef MKGEOIP_MMDB_OPEN
// MKGEOIP_MMDB_OPEN allows to mock MMDB_open
#define MKGEOIP_MMDB_OPEN MMDB_open
#endif

#ifndef MKGEOIP_MMDB_LOOKUP_STRING
// MKGEOIP_MMDB_LOOKUP_STRING allows to mock MMDB_lookup_string
#define MKGEOIP_MMDB_LOOKUP_STRING MMDB_lookup_string
#endif

static bool mkgeoip_lookup_cc(
    const std::string &path, const std::string &ip, std::string *out) {
  return mkgeoip_lookup_mmdb(
      path, ip, [&](MMDB_entry_s *entry) {
        MMDB_entry_data_s data{};
        auto mmdb_error = MKGEOIP_MMDB_GET_VALUE(
            entry, &data, "registered_country", "iso_code", nullptr);
        if (mmdb_error != 0) return false;
        if (!data.has_data || data.type != MMDB_DATA_TYPE_UTF8_STRING) {
          return false;
        }
        if (out == nullptr) return false;
        *out = std::string{data.utf8_string, data.data_size};
        return true;
      });
}

static bool mkgeoip_lookup_asn(
    const std::string &path, const std::string &ip, std::string *out) {
  return mkgeoip_lookup_mmdb(
      path, ip, [&](MMDB_entry_s *entry) {
        MMDB_entry_data_s data{};
        auto mmdb_error = MKGEOIP_MMDB_GET_VALUE(
            entry, &data, "autonomous_system_number", nullptr);
        if (mmdb_error != 0) return false;
        if (!data.has_data || data.type != MMDB_DATA_TYPE_UINT32) {
          return false;
        }
        if (out == nullptr) return false;
        *out = std::string{"AS"} + std::to_string(data.uint32);
        return true;
      });
}

static bool mkgeoip_lookup_netname(
    const std::string &path, const std::string &ip, std::string *out) {
  return mkgeoip_lookup_mmdb(
      path, ip, [&](MMDB_entry_s *entry) {
        MMDB_entry_data_s data{};
        auto mmdb_error = MKGEOIP_MMDB_GET_VALUE(
            entry, &data, "autonomous_system_organization", nullptr);
        if (mmdb_error != 0) return false;
        if (!data.has_data || data.type != MMDB_DATA_TYPE_UTF8_STRING) {
          return false;
        }
        if (out == nullptr) return false;
        *out = std::string{data.utf8_string, data.data_size};
        return true;
      });
}

static bool mkgeoip_lookup_mmdb(
    const std::string &path, const std::string &ip,
    std::function<bool(MMDB_entry_s *)> fun) {
  MMDB_s mmdb{};
  if (MKGEOIP_MMDB_OPEN(path.c_str(), MMDB_MODE_MMAP, &mmdb) != 0) return false;
  auto rv = false;
  auto gai_error = 0;
  auto mmdb_error = 0;
  auto record = MKGEOIP_MMDB_LOOKUP_STRING(&mmdb, ip.c_str(), &gai_error,
                                           &mmdb_error);
  if (gai_error == 0 && mmdb_error == 0 && record.found_entry == true) {
    rv = fun(&record.entry);
  }
  MMDB_close(&mmdb);
  return rv;
}

#endif  // MKGEOIP_INLINE_IMPL
#endif  // __cplusplus
#endif  // MEASUREMENT_KIT_MKGEOIP_H
