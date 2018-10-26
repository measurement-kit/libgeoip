#include <stdlib.h>

#include <iostream>

#define MKCURL_INLINE_IMPL
#define MKGEOIP_INLINE_IMPL
#include "mkgeoip.h"

int main() {
  mkgeoip_iplookup_ubuntu_uptr ubuntu{mkgeoip_iplookup_ubuntu_new()};
  if (ubuntu == nullptr) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  const char *probe_ip = mkgeoip_iplookup_ubuntu_get_probe_ip(ubuntu.get());
  if (probe_ip == nullptr) {
    std::clog << "Cannot parse probe IP" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::clog << "Probe IP: " << probe_ip << std::endl;
  {
    mkgeoip_mmdb_uptr country{mkgeoip_mmdb_open("country.mmdb")};
    if (country == nullptr) {
      std::clog << "Out of memory" << std::endl;
      exit(EXIT_FAILURE);
    }
    const char *probe_cc = mkgeoip_mmdb_lookup_cc(country.get(), probe_ip);
    if (probe_cc != nullptr) {
      std::clog << "Probe CC: " << probe_cc << std::endl;
    }
  }
  {
    mkgeoip_mmdb_uptr asn{mkgeoip_mmdb_open("asn.mmdb")};
    if (asn == nullptr) {
      std::clog << "Out of memory" << std::endl;
      exit(EXIT_FAILURE);
    }
    int64_t probe_asn = mkgeoip_mmdb_lookup_asn(asn.get(), probe_ip);
    std::clog << "Probe ASN: " << probe_asn << std::endl;
    const char *probe_org = mkgeoip_mmdb_lookup_org(asn.get(), probe_ip);
    if (probe_org != nullptr) {
      std::clog << "Probe Network Name: " << probe_org << std::endl;
    }
  }
}
