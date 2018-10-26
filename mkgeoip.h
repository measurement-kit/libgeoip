// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKGEOIP_H
#define MEASUREMENT_KIT_MKGEOIP_H

#include "mkcurl.h"

#ifdef __cplusplus
extern "C" {
#endif

/// mkgeoip_iplookup_ubuntu_get_url is a more low level interface to perfrom
/// the IP lookup than mkgeoip_iplookup_ubuntu_perform. This function just
/// returns the URL of the Ubuntu IP lookup services, and it is up to you to
/// prepare a mkcurl_request_t, or perform the request entirely with any other
/// means (e.g. using github.com/square/okhttp on Android).
const char *mkgeoip_iplookup_ubuntu_get_url(void);

/// mkgeoip_iplookup_ubuntu_perform is a more low level interface to perform
/// the IP lookup than mkgeoip_iplookup_ubuntu_get_probe_ip. This function
/// returns the response returned by Ubuntu services (which may indicate that
/// an error occurred) or NULL in case of severe internal error. This API gives
/// you the freedom to look into the response, therefore you can access the
/// logs, extract the certificate chain, understand what went wrong, etc. To
/// parse the probe IP (if any) from the response, you should then create a
/// mkgeoip_iplookup_ubuntu_t instance to process the returned response.
mkcurl_response_t *mkgeoip_iplookup_ubuntu_perform(void);

/// mkgeoip_iplookup_ubuntu_t looks up your public IP using Ubuntu services.
typedef struct mkgeoip_iplookup_ubuntu mkgeoip_iplookup_ubuntu_t;

/// mkgeoip_iplookup_ubuntu_new creates a mkgeoip_iplookup_ubuntu_t instance.
mkgeoip_iplookup_ubuntu_t *mkgeoip_iplookup_ubuntu_new(void);

/// mkgeoip_iplookup_ubuntu_get_probe_ip performs the IP lookup using Ubuntu
/// services and returns the IP on success, NULL on failure. This is the most
/// simple interface to perform the IP lookup. The returned string will be
/// valid as long as @p ubuntu is valid and you do not call any other function
/// that uses the same @p ubuntu instance.
const char *mkgeoip_iplookup_ubuntu_get_probe_ip(
    mkgeoip_iplookup_ubuntu_t *ubuntu);

/// mkgeoip_iplookup_ubuntu_parse_probe_ip extracts the IP from the @p response
/// if available, otherwise returns NULL. The returned string is managed by
/// @p ubuntu and is valid until @p ubuntu is valid _and_ you don't call again
/// any other function that uses the same @p ubuntu instance.
const char *mkgeoip_iplookup_ubuntu_parse_probe_ip(
    mkgeoip_iplookup_ubuntu_t *ubuntu, const mkcurl_response_t *response);

/// mkgeoip_iplookup_ubuntu_delete deletes @p ubuntu.
void mkgeoip_iplookup_ubuntu_delete(mkgeoip_iplookup_ubuntu_t *ubuntu);

/// mkgeoip_mmdb_t saves the results of MMDB queries.
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

/// mkgeoip_iplookup_ubuntu_deleter is a deleter for mkgeoip_iplookup_ubuntu_t.
struct mkgeoip_iplookup_ubuntu_deleter {
  void operator()(mkgeoip_iplookup_ubuntu_t *p) {
    mkgeoip_iplookup_ubuntu_delete(p);
  }
};

/// mkgeoip_iplookup_ubuntu_uptr is a unique pointer to mkgeoip_iplookup_ubuntu_t.
using mkgeoip_iplookup_ubuntu_uptr = std::unique_ptr<
    mkgeoip_iplookup_ubuntu_t, mkgeoip_iplookup_ubuntu_deleter>;

/// mkgeoip_mmdb_deleter is a deleter for mkgeoip_mmdb_t.
struct mkgeoip_mmdb_deleter {
  void operator()(mkgeoip_mmdb_t *p) { mkgeoip_mmdb_close(p); }
};

/// mkgeoip_mmdb_uptr is a unique pointer to mkgeoip_mmdb_t.
using mkgeoip_mmdb_uptr = std::unique_ptr<mkgeoip_mmdb_t, mkgeoip_mmdb_deleter>;

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

mkcurl_response_t *mkgeoip_iplookup_ubuntu_perform() {
  mkcurl_request_uptr request{mkcurl_request_new()};
  if (request == nullptr) return nullptr;
  mkcurl_request_set_url(request.get(), mkgeoip_iplookup_ubuntu_get_url());
  return mkcurl_perform(request.get());
}

const char *mkgeoip_iplookup_ubuntu_get_url() {
  return "https://geoip.ubuntu.com/lookup";
}

struct mkgeoip_iplookup_ubuntu {
  std::string probe_ip;
};

mkgeoip_iplookup_ubuntu_t *mkgeoip_iplookup_ubuntu_new() {
  return new mkgeoip_iplookup_ubuntu_t;
}

const char *mkgeoip_iplookup_ubuntu_get_probe_ip(
    mkgeoip_iplookup_ubuntu_t *ubuntu) {
  mkcurl_response_uptr response{mkgeoip_iplookup_ubuntu_perform()};
  if (response == nullptr) return nullptr;
  return mkgeoip_iplookup_ubuntu_parse_probe_ip(ubuntu, response.get());
}

#ifndef MKGEOIP_MKCURL_RESPONSE_GET_BODY_BINARY_V2
// MKGEOIP_MKCURL_RESPONSE_GET_BODY_BINARY_V2 allows to mock
// mkcurl_response_get_body_binary_v2
#define MKGEOIP_MKCURL_RESPONSE_GET_BODY_BINARY_V2  \
  mkcurl_response_get_body_binary_v2
#endif

const char *mkgeoip_iplookup_ubuntu_parse_probe_ip(
    mkgeoip_iplookup_ubuntu_t *ubuntu, const mkcurl_response_t *response) {
  if (ubuntu == nullptr || response == nullptr) return nullptr;
  if (mkcurl_response_get_error(response) != 0 ||
      mkcurl_response_get_status_code(response) != 200) {
    return nullptr;
  }
  std::string body;
  {
    const uint8_t *base = nullptr;
    size_t count = 0;
    if (!MKGEOIP_MKCURL_RESPONSE_GET_BODY_BINARY_V2(
            response, &base, &count) ||
        base == nullptr || count <= 0) {
      return nullptr;
    }
    body = std::string{(char *)base, count};
  }
  {
    static const std::string open_tag = "<Ip>";
    static const std::string close_tag = "</Ip>";
    auto pos = body.find(open_tag);
    if (pos == std::string::npos) return nullptr;
    body = body.substr(pos + open_tag.size());  // Find EOS in the worst case
    pos = body.find(close_tag);
    if (pos == std::string::npos) return nullptr;
    body = body.substr(0, pos);
    for (auto ch : body) {
      if (isspace(ch)) continue;
      ch = (char)tolower(ch);
      auto ok = isdigit(ch) || (ch >= 'a' && ch <= 'f') || ch == '.' || ch == ':';
      if (!ok) return nullptr;
      ubuntu->probe_ip += ch;
    }
  }
  return ubuntu->probe_ip.c_str();
}

void mkgeoip_iplookup_ubuntu_delete(mkgeoip_iplookup_ubuntu_t *ubuntu) {
  delete ubuntu;
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

#endif  // MKGEOIP_INLINE_IMPL
#endif  // __cplusplus
#endif  // MEASUREMENT_KIT_MKGEOIP_H
