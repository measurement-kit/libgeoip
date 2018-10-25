#include <stdlib.h>

#include <iostream>

#define MKCURL_INLINE_IMPL
#define MKGEOIP_INLINE_IMPL
#include "mkgeoip.h"

int main() {
  mkcurl_request_uptr request{mkcurl_request_new()};
  if (request == nullptr) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  mkcurl_request_set_url(request.get(), mkgeoip_ubuntu_get_url());
  mkcurl_response_uptr response{mkcurl_perform(request.get())};
  if (response == nullptr) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }

  mkgeoip_ubuntu_parser_uptr parser{mkgeoip_ubuntu_parser_new()};
  if (parser == nullptr) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  const char *probe_ip = mkgeoip_ubuntu_parser_get_probe_ip(
      parser.get(), response.get());
  if (!probe_ip) {
    std::clog << "Cannot parse probe IP" << std::endl;
    std::clog << "=== BEGIN LOGS ===" << std::endl;
    const char *logs = mkcurl_response_get_logs(response.get());
    if (logs != nullptr) {
      std::clog << logs;
    }
    std::clog << "=== END LOGS ===" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::clog << "Probe IP: " << probe_ip << std::endl;

  mkgeoip_mmdb_uptr country{mkgeoip_mmdb_open("country.mmdb")};
  if (country == nullptr) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  {
    const char *probe_cc = mkgeoip_mmdb_lookup_cc(country.get(), probe_ip);
    if (probe_cc == nullptr) {
      std::clog << "Cannot retrieve CC" << std::endl;
    } else {
      std::clog << "Probe CC: " << probe_cc << std::endl;
    }
  }

  mkgeoip_mmdb_uptr asn{mkgeoip_mmdb_open("asn.mmdb")};
  if (asn == nullptr) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  {
    int64_t probe_asn = mkgeoip_mmdb_lookup_asn(asn.get(), probe_ip);
    std::clog << "Probe ASN: " << probe_asn << std::endl;
  }
  {
    const char *probe_org = mkgeoip_mmdb_lookup_org(asn.get(), probe_ip);
    if (probe_org == nullptr) {
      std::clog << "Cannot retrieve ORG" << std::endl;
    } else {
      std::clog << "Probe Network Name: " << probe_org << std::endl;
    }
  }
}
