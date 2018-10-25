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
  mkgeoip_uptr geoip{mkgeoip_new()};
  if (geoip == nullptr) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (!mkgeoip_iplookup(geoip.get(), request.get())) {
    std::clog << "mkgeoip_iplookup() failed" << std::endl;
    std::clog << "=== BEGIN LOGS ===" << std::endl;
    mkcurl_response_uptr response{mkgeoip_get_response(geoip.get())};
    if (response == nullptr) {
      std::clog << "Out of memory" << std::endl;
      exit(EXIT_FAILURE);
    }
    std::clog << mkcurl_response_get_logs(response.get());
    std::clog << "=== END LOGS ===" << std::endl;
    exit(EXIT_FAILURE);
  }
  const char *probe_ip = mkgeoip_get_probe_ip(geoip.get());
  std::clog << "Probe IP: " << probe_ip << std::endl;
  {
    const char *probe_cc = mkgeoip_mmdb_query_probe_cc(
        geoip.get(), probe_ip, "country.mmdb");
    std::clog << "Probe CC: " << probe_cc << std::endl;
  }
  {
    const char *probe_asn = mkgeoip_mmdb_query_probe_asn(
        geoip.get(), probe_ip, "asn.mmdb");
    std::clog << "Probe ASN: " << probe_asn << std::endl;
  }
  {
    const char *probe_netname = mkgeoip_mmdb_query_probe_netname(
        geoip.get(), probe_ip, "asn.mmdb");
    std::clog << "Probe Network Name: " << probe_netname << std::endl;
  }
}
