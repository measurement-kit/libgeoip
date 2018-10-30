#include <string.h>

#include <iostream>
#include <ios>

#include "mkgeoip.h"

#define MKCURL_INLINE_IMPL
#define MKDATA_INLINE_IMPL
#define MKIPLOOKUP_INLINE_IMPL
#include "mkiplookup.h"

#define MKMMDB_INLINE_IMPL
#include "mkmmdb.h"

int main() {
  mkgeoip_lookup_settings_uptr settings{mkgeoip_lookup_settings_new_nonnull()};
  mkgeoip_lookup_settings_set_timeout_v2(settings.get(), 7);
  mkgeoip_lookup_settings_set_ca_bundle_path_v2(settings.get(), "ca-bundle.pem");
  mkgeoip_lookup_settings_set_asn_db_path_v2(settings.get(), "asn.mmdb");
  mkgeoip_lookup_settings_set_country_db_path_v2(settings.get(), "country.mmdb");
  mkgeoip_lookup_results_uptr results{mkgeoip_lookup_settings_perform_nonnull(
      settings.get())};
  {
    int64_t good = mkgeoip_lookup_results_good_v2(results.get());
    std::clog << std::boolalpha << (bool)good << std::endl;
  }
  {
    const char *probe_ip = mkgeoip_lookup_results_get_probe_ip_v2(results.get());
    std::clog << "probe_ip: " << probe_ip << std::endl;
  }
  {
    int64_t probe_asn = mkgeoip_lookup_results_get_probe_asn_v2(results.get());
    std::clog << "probe_asn: " << probe_asn << std::endl;
  }
  {
    const char *probe_org = mkgeoip_lookup_results_get_probe_org_v2(results.get());
    std::clog << "probe_org: " << probe_org << std::endl;
  }
  {
    const char *probe_cc = mkgeoip_lookup_results_get_probe_cc_v2(results.get());
    std::clog << "probe_cc: " << probe_cc << std::endl;
  }
  std::clog << "bytes_sent: "
            << mkgeoip_lookup_results_get_bytes_sent_v2(results.get())
            << std::endl;
  std::clog << "bytes_recv: "
            << mkgeoip_lookup_results_get_bytes_recv_v2(results.get())
            << std::endl;
  {
    std::string logs = mkgeoip_lookup_results_moveout_logs_v2(results.get());
    std::clog << "=== BEGIN LOGS ===" << std::endl;
    std::clog << logs;
    std::clog << "=== END LOGS ===" << std::endl;
  }
}
