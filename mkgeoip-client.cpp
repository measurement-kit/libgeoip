#include <string.h>

#include <iostream>
#include <ios>

#include "mkgeoip.hpp"

#define MKCURL_INLINE_IMPL
#define MKDATA_INLINE_IMPL
#define MKIPLOOKUP_INLINE_IMPL
#include "mkiplookup.hpp"

#define MKMMDB_INLINE_IMPL
#include "mkmmdb.hpp"

int main() {
  mk::geoip::LookupSettings settings;
  settings.timeout = 7;
  settings.ca_bundle_path = ".mkbuild/download/ca-bundle.pem";
  settings.asn_db_path = ".mkbuild/download/asn.mmdb";
  settings.country_db_path = ".mkbuild/download/country.mmdb";
  mk::geoip::LookupResults results = mk::geoip::lookup(settings);
  std::clog << std::boolalpha << "good: " << results.good << std::endl;
  std::clog << "probe_ip: " << results.probe_ip << std::endl;
  std::clog << "probe_asn_string: " << results.probe_asn_string << std::endl;
  std::clog << "probe_org: " << results.probe_org << std::endl;
  std::clog << "probe_cc: " << results.probe_cc << std::endl;
  std::clog << "bytes_sent: " << results.bytes_sent << std::endl;
  std::clog << "bytes_recv: " << results.bytes_recv << std::endl;
  std::clog << "=== BEGIN LOGS ===" << std::endl;
  for (auto &log : results.logs) std::clog << log << std::endl;
  std::clog << "=== END LOGS ===" << std::endl;
}
