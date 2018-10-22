#ifndef MEASUREMENT_KIT_MKGEOIP_MKGEOIP_H
#define MEASUREMENT_KIT_MKGEOIP_MKGEOIP_H
#ifdef __cplusplus
#include <memory>  // for std::unique_ptr<T>
extern "C" {
#endif

// mkgeoip discovers the probe IP, country code (CC), autonomous system
// numer (ASN), and network name (i.e. the ASN description).
//
// This is the typical workflow of integrating mkgeoip into a project:
//
// 1. download the latest measurement-kit/libcurlx into the current directory
//
// 2. download mkgeoip.h
//
// 3. make sure you include libcurlx.h first
//
// 4. then include mkgeoip.h
//
// 5. depending on your needs also define MK_CURLX_INLINE_IMPL before
//    including libcurlx.h and MKGEOIP_INLINE_IMPL before including mkgeoip.h
//
// This is the typical workflow of using mkgeoip:
//
// 1. create a mkgeoip_settings_t instance
//
// 2. populate the settings
//
// 3. call mkgeoip_lookup() with the specified settings to obtain
//    a mkgeoip_results_t instance
//
// 4. destroy the settings instance
//
// 5. use the results (make sure you check whether an error occurred
//    by using mkgeoip_results_get_error() first)
//
// 6. destroy the results instance

// mkgeoip_error enumerates all possible error conditions.
typedef enum mkgeoip_error {
  MKGEOIP_ENONE,
  MKGEOIP_EFAULT,
  MKGEOIP_ECURL,
  MKGEOIP_ENOMEM,
  MKGEOIP_EHTTP,
  MKGEOIP_EMMDBGETVALUE,
  MKGEOIP_ENODATAFORTYPE,
  MKGEOIP_ENOTFOUND,
  MKGEOIP_EGETADDRINFO,
  MKGEOIP_EMMDBLOOKUPSTRING,
  MKGEOIP_EMMDBOPEN,
} mkgeoip_error_t;

// mkgeoip_settings_t contains the settings.
typedef struct mkgeoip_settings mkgeoip_settings_t;

// mkgeoip_settings_new creates a new settings instance.
mkgeoip_settings_t *mkgeoip_settings_new(void);

// mkgeoip_settings_set_timeout changes the default timeout for I/O.
void mkgeoip_settings_set_timeout(mkgeoip_settings_t *p, int v);

// mkgeoip_settings_set_country_db_path sets the path to the county
// database. We currently use libmaxminddb as a backend.
void mkgeoip_settings_set_country_db_path(mkgeoip_settings_t *p,
                                           const char *v);

// mkgeoip_settings_set_asn_db_path sets the path to the ASN
// database. We currently use libmaxminddb as a backend.
void mkgeoip_settings_set_asn_db_path(mkgeoip_settings_t *p,
                                       const char *v);

// mkgeoip_settings_set_ca_bundle_path sets path to CA bundle path.
void mkgeoip_settings_set_ca_bundle_path(mkgeoip_settings_t *p, const char *ca);

// mkgeoip_settings_delete deletes a settings instance.
void mkgeoip_settings_delete(mkgeoip_settings_t *p);

// mkgeoip_results_t contains the results.
typedef struct mkgeoip_results mkgeoip_results_t;

// mkgeoip_results_get_error returs the error that occurred.
mkgeoip_error_t mkgeoip_results_get_error(mkgeoip_results_t *p);

// mkgeoip_results_get_probe_ip returns the probe IP.
const char *mkgeoip_results_get_probe_ip(mkgeoip_results_t *p);

// mkgeoip_results_get_probe_asn returns the probe ASN.
const char *mkgeoip_results_get_probe_asn(mkgeoip_results_t *p);

// mkgeoip_results_get_probe_cc returns the probe CC.
const char *mkgeoip_results_get_probe_cc(mkgeoip_results_t *p);

// mkgeoip_results_get_probe_network_name returns the probe network name.
const char *mkgeoip_results_get_probe_network_name(mkgeoip_results_t *p);

// mkgeoip_results_get_bytes_sent get the bytes sent during the lookup.
double mkgeoip_results_get_bytes_sent(mkgeoip_results_t *p);

// mkgeoip_results_get_bytes_sent get the bytes recv during the lookup.
double mkgeoip_results_get_bytes_recv(mkgeoip_results_t *p);

// mkgeoip_results_get_bytes_sent get the lookup logs.
const char *mkgeoip_results_get_logs(mkgeoip_results_t *p);

// mkgeoip_results_delete deletes a results instance.
void mkgeoip_results_delete(mkgeoip_results_t *p);

// mkgeoip_lookup resolves the probe IP, the probe ASN, the probe CC,
// and the probe network name using the specified |p| settings. The return
// value MAY be NULL under severe internal error conditions as well as when
// the provided |p| parameter is NULL. Note that you own the pointer that
// is returned by this function and must delete it when done.
mkgeoip_results_t *mkgeoip_lookup(const mkgeoip_settings_t *p);

#ifdef __cplusplus
}  // extern "C"

// mkgeoip_settings_deleter is a custom deleter for mkgeoip_settings_t.
struct mkgeoip_settings_deleter {
  void operator()(mkgeoip_settings_t *p) { mkgeoip_settings_delete(p); }
};

// mkgeoip_settings_uptr is syntactic sugar for using a settings object with
// RAII semantic when using this code from C++.
using mkgeoip_settings_uptr = std::unique_ptr<mkgeoip_settings_t,
                                               mkgeoip_settings_deleter>;

// mkgeoip_results_deleter is a custom deleter for mkgeoip_results_t.
struct mkgeoip_results_deleter {
  void operator()(mkgeoip_results_t *p) { mkgeoip_results_delete(p); }
};

// mkgeoip_results_uptr is syntactic sugar for using a results object with
// RAII semantic when using this code from C++.
using mkgeoip_results_uptr = std::unique_ptr<mkgeoip_results_t>;

// By default the implementation is not included. You can force it being
// included by providing the following definition to the compiler.
//
// If you're just into understanding the API, you can stop reading here.
#ifdef MKGEOIP_INLINE_IMPL

#include <ctype.h>

#include <functional>
#include <string>

#include <maxminddb.h>

struct mkgeoip_settings {
  int timeout = 30  /* seconds */;
  std::string country_db_path;
  std::string asn_db_path;
  std::string ca_path;
};

mkgeoip_settings_t *mkgeoip_settings_new() {
  return new mkgeoip_settings;
}

void mkgeoip_settings_set_timeout(mkgeoip_settings_t *p, int v) {
  if (p != nullptr) p->timeout = v;
}

void mkgeoip_settings_set_country_db_path(mkgeoip_settings_t *p,
                                           const char *v) {
  if (p != nullptr && v != nullptr) p->country_db_path = v;
}

void mkgeoip_settings_set_asn_db_path(mkgeoip_settings_t *p,
                                            const char *v) {
  if (p != nullptr && v != nullptr) p->asn_db_path = v;
}

void mkgeoip_settings_set_ca_bundle_path(mkgeoip_settings_t *p,
                                         const char *v) {
  if (p != nullptr && v != nullptr) p->ca_path = v;
}

void mkgeoip_settings_delete(mkgeoip_settings_t *p) {
  delete p;
}

struct mkgeoip_results {
  mkgeoip_error_t error = MKGEOIP_ENONE;
  std::string probe_ip;
  std::string probe_asn;
  std::string probe_cc;
  std::string probe_network_name;
  std::string logs;
  double bytes_recv = 0.0;
  double bytes_sent = 0.0;
};

mkgeoip_error_t mkgeoip_results_get_error(mkgeoip_results_t *p) {
  return (p != nullptr) ? p->error : MKGEOIP_EFAULT;
}

const char *mkgeoip_results_get_probe_ip(mkgeoip_results_t *p) {
  return (p != nullptr) ? p->probe_ip.c_str() : "";
}

const char *mkgeoip_results_get_probe_asn(mkgeoip_results_t *p) {
  return (p != nullptr) ? p->probe_asn.c_str() : "";
}

const char *mkgeoip_results_get_probe_cc(mkgeoip_results_t *p) {
  return (p != nullptr) ? p->probe_cc.c_str() : "";
}

const char *mkgeoip_results_get_probe_network_name(mkgeoip_results_t *p) {
  return (p != nullptr) ? p->probe_network_name.c_str() : "";
}

const char *mkgeoip_results_get_logs(mkgeoip_results_t *p) {
  return (p != nullptr) ? p->logs.c_str() : "";
}

double mkgeoip_results_get_bytes_recv(mkgeoip_results_t *p) {
  return (p != nullptr) ? p->bytes_recv : 0.0;
}

double mkgeoip_results_get_bytes_sent(mkgeoip_results_t *p) {
  return (p != nullptr) ? p->bytes_sent : 0.0;
}

void mkgeoip_results_delete(mkgeoip_results_t *p) {
  delete p;
}

static bool lookup_ip(const mkgeoip_settings_t *, mkgeoip_results_uptr &);
static bool lookup_cc(const mkgeoip_settings_t *, mkgeoip_results_uptr &);
static bool lookup_asn(const mkgeoip_settings_t *, mkgeoip_results_uptr &);

mkgeoip_results_t *mkgeoip_lookup(const mkgeoip_settings_t *p) {
  if (p == nullptr) return nullptr;
  mkgeoip_results_uptr r{new mkgeoip_results_t};
  // TODO(bassosimone): see if we really need a failure logic
  (void)lookup_ip(p, r);
  (void)lookup_cc(p, r);
  (void)lookup_asn(p, r);
  return r.release();
}

static bool parse_ip(const std::string &s, mkgeoip_results_uptr &r);

static bool lookup_ip(const mkgeoip_settings_t *p, mkgeoip_results_uptr &r) {
  mkcurl_request_uptr req{mkcurl_request_new()};
  if (!req) {
    r->error = MKGEOIP_ENOMEM;
    return false;
  }
  mkcurl_request_enable_http2(req.get());
  mkcurl_request_set_url(req.get(), "https://geoip.ubuntu.com/lookup");
  mkcurl_request_set_timeout(req.get(), p->timeout);
  if (!p->ca_path.empty()) {
    mkcurl_request_set_ca_bundle_path(req.get(), p->ca_path.c_str());
  }
  mkcurl_response_uptr res{mkcurl_perform(req.get())};
  if (!res) {
    r->error = MKGEOIP_ENOMEM;
    return false;
  }
  r->bytes_sent += mkcurl_response_get_bytes_sent(res.get());
  r->bytes_recv += mkcurl_response_get_bytes_recv(res.get());
  r->logs += mkcurl_response_get_logs(res.get());
  if (mkcurl_response_get_error(res.get()) != 0) {
    r->error = MKGEOIP_ECURL;
    return false;
  }
  if (mkcurl_response_get_status_code(res.get()) != 200) {
    r->error = MKGEOIP_EHTTP;
    return false;
  }
  std::string body = mkcurl_response_get_body(res.get());
  if (!parse_ip(body, r)) {
    r->logs += "Failed to parse IP\n";
    return false;
  }
  r->logs += "Successfully parsed IP: ";
  r->logs += r->probe_ip;
  r->logs += "\n";
  return true;
}

static bool parse_ip(const std::string &s, mkgeoip_results_uptr &r) {
  static const std::string open_tag = "<Ip>";
  static const std::string close_tag = "</Ip>";
  std::string input = s;  // Making a copy
  auto pos = input.find(open_tag);
  if (pos == std::string::npos) return false;
  input = input.substr(pos + open_tag.size());
  pos = input.find(close_tag);
  if (pos == std::string::npos) return false;
  input = input.substr(0, pos);
  for (auto ch : input) {
    if (isspace(ch)) continue;
    ch = (char)tolower(ch);
    auto ok = isdigit(ch) || (ch >= 'a' && ch <= 'f') || ch == '.' || ch == ':';
    if (!ok) return false;
    r->probe_ip += ch;
  }
  return true;
}

static bool lookup_mmdb_using_probe_ip(
    const std::string &path, mkgeoip_results_uptr &r,
    std::function<bool(MMDB_entry_s *)> fun);

#ifndef MKGEOIP_MMDB_GET_VALUE
#define MKGEOIP_MMDB_GET_VALUE MMDB_get_value
#endif

#ifndef MKGEOIP_MMDB_OPEN
#define MKGEOIP_MMDB_OPEN MMDB_open
#endif

#ifndef MKGEOIP_MMDB_LOOKUP_STRING
#define MKGEOIP_MMDB_LOOKUP_STRING MMDB_lookup_string
#endif

static bool lookup_cc(const mkgeoip_settings_t *p, mkgeoip_results_uptr &r) {
  return lookup_mmdb_using_probe_ip(
      p->country_db_path, r, [&](MMDB_entry_s *entry) {
        {
          MMDB_entry_data_s data{};
          auto mmdb_error = MKGEOIP_MMDB_GET_VALUE(
              entry, &data, "registered_country", "iso_code", nullptr);
          if (mmdb_error != 0) {
            r->error = MKGEOIP_EMMDBGETVALUE;
            r->logs += "MMDB_get_value() failed: ";
            r->logs += MMDB_strerror(mmdb_error);
            r->logs += "\n";
            return false;
          }
          if (!data.has_data || data.type != MMDB_DATA_TYPE_UTF8_STRING) {
            r->error = MKGEOIP_ENODATAFORTYPE;
            r->logs += "MMDB_get_value() failed: no data for expected type";
            r->logs += "\n";
            return false;
          }
          r->probe_cc = std::string{data.utf8_string, data.data_size};
          r->logs += "Probe CC: ";
          r->logs += r->probe_cc;
          r->logs += "\n";
        }
        return true;
      });
}

static bool lookup_asn(const mkgeoip_settings_t *p, mkgeoip_results_uptr &r) {
  return lookup_mmdb_using_probe_ip(
      p->asn_db_path, r, [&](MMDB_entry_s *entry) {
        {
          MMDB_entry_data_s data{};
          auto mmdb_error = MKGEOIP_MMDB_GET_VALUE(
              entry, &data, "autonomous_system_number", nullptr);
          if (mmdb_error != 0) {
            r->error = MKGEOIP_EMMDBGETVALUE;
            r->logs += "MMDB_get_value() failed: ";
            r->logs += MMDB_strerror(mmdb_error);
            r->logs += "\n";
            return false;
          }
          if (!data.has_data || data.type != MMDB_DATA_TYPE_UINT32) {
            r->error = MKGEOIP_ENODATAFORTYPE;
            r->logs += "MMDB_get_value() failed: no data for expected type";
            r->logs += "\n";
            return false;
          }
          r->probe_asn = std::string{"AS"} + std::to_string(data.uint32);
          r->logs += "Probe ASN: ";
          r->logs += r->probe_asn;
          r->logs += "\n";
        }
        {
          MMDB_entry_data_s data{};
          auto mmdb_error = MKGEOIP_MMDB_GET_VALUE(
              entry, &data, "autonomous_system_organization", nullptr);
          if (mmdb_error != 0) {
            r->error = MKGEOIP_EMMDBGETVALUE;
            r->logs += "MMDB_get_value() failed: ";
            r->logs += MMDB_strerror(mmdb_error);
            r->logs += "\n";
            return false;
          }
          if (!data.has_data || data.type != MMDB_DATA_TYPE_UTF8_STRING) {
            r->error = MKGEOIP_ENODATAFORTYPE;
            r->logs += "MMDB_get_value() failed: no data for expected type";
            r->logs += "\n";
            return false;
          }
          r->probe_network_name = std::string{
              data.utf8_string, data.data_size};
          r->logs += "Probe Network Name: ";
          r->logs += r->probe_network_name;
          r->logs += "\n";
        }
        return true;
      });
}

static bool lookup_mmdb_using_probe_ip(
    const std::string &path, mkgeoip_results_uptr &r,
    std::function<bool(MMDB_entry_s *)> fun) {
  MMDB_s mmdb{};
  auto mmdb_error = MKGEOIP_MMDB_OPEN(path.c_str(), MMDB_MODE_MMAP, &mmdb);
  if (mmdb_error != 0) {
    r->error = MKGEOIP_EMMDBOPEN;
    r->logs += "MMDB_open() failed: ";
    r->logs += MMDB_strerror(mmdb_error);
    r->logs += "\n";
    return false;
  }
  auto rv = false;
  do {
    auto gai_error = 0;
    mmdb_error = 0;
    auto record = MKGEOIP_MMDB_LOOKUP_STRING(&mmdb, r->probe_ip.c_str(),
                                              &gai_error, &mmdb_error);
    if (gai_error) {
      r->error = MKGEOIP_EGETADDRINFO;
      r->logs += "MMDB_lookup_string() failed: ";
      r->logs += gai_strerror(gai_error);
      r->logs += "\n";
      break;
    }
    if (mmdb_error) {
      r->error = MKGEOIP_EMMDBLOOKUPSTRING;
      r->logs += "MMDB_lookup_string() failed: ";
      r->logs += MMDB_strerror(gai_error);
      r->logs += "\n";
      break;
    }
    if (!record.found_entry) {
      r->error = MKGEOIP_ENOTFOUND;
      r->logs += "MMDB_lookup_string() failed: no entry for probe_ip";
      r->logs += "\n";
      break;
    }
    rv = fun(&record.entry);
  } while (0);
  MMDB_close(&mmdb);
  return rv;
}

#endif  // MKGEOIP_INLINE_IMPL
#endif  // __cplusplus
#endif  // MEASUREMENT_KIT_MKGEOIP_MKGEOIP_H
