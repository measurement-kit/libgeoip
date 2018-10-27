#include <string.h>

#include <iostream>
#include <ios>

#define MKCURL_INLINE_IMPL
#define MKGEOIP_INLINE_IMPL
#define MKDATA_INLINE_IMPL
#include "mkgeoip.h"

int main() {
  mkgeoip_lookup_settings_uptr settings{mkgeoip_lookup_settings_new()};
  if (settings == nullptr) abort();
  mkgeoip_lookup_settings_set_ca_bundle_path(settings.get(), "ca-bundle.pem");
  mkgeoip_lookup_settings_set_asn_db_path(settings.get(), "asn.mmdb");
  mkgeoip_lookup_settings_set_country_db_path(settings.get(), "country.mmdb");
  mkgeoip_lookup_results_uptr results{mkgeoip_lookup_settings_perform(
      settings.get())};
  if (results == nullptr) abort();
  {
    int64_t good = mkgeoip_lookup_results_good(results.get());
    std::clog << std::boolalpha << (bool)good << std::endl;
  }
  {
    const char *probe_ip = mkgeoip_lookup_results_get_probe_ip(results.get());
    if (probe_ip != nullptr) std::clog << "probe_ip: " << probe_ip << std::endl;
  }
  {
    int64_t probe_asn = mkgeoip_lookup_results_get_probe_asn(results.get());
    std::clog << "probe_asn: " << probe_asn << std::endl;
  }
  {
    const char *probe_org = mkgeoip_lookup_results_get_probe_org(results.get());
    if (probe_org != nullptr) std::clog << "probe_org: " << probe_org << std::endl;
  }
  {
    const char *probe_cc = mkgeoip_lookup_results_get_probe_cc(results.get());
    if (probe_cc != nullptr) std::clog << "probe_cc: " << probe_cc << std::endl;
  }
  {
    const uint8_t *base = nullptr;
    size_t count = 0;
    if (!mkgeoip_lookup_results_get_logs_binary(results.get(), &base, &count)) {
      abort();
    }
    std::clog << "=== BEGIN LOGS ===" << std::endl;
    std::clog << std::string{(const char *)base, count};
    std::clog << "=== END LOGS ===" << std::endl;
  }
}
