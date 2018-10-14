#include <stdlib.h>

#include <iostream>

#define MK_CURLX_INLINE_IMPL
#define MK_GEOIP_INLINE_IMPL
#include "libgeoip.h"

int main() {
  mk_geoip_settings_uptr settings{mk_geoip_settings_new()};
  if (!settings) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  mk_geoip_settings_set_country_db_path(settings.get(), "country.mmdb");
  mk_geoip_settings_set_asn_db_path(settings.get(), "asn.mmdb");
  mk_geoip_results_uptr results{mk_geoip_lookup(settings.get())};
  if (!results) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::clog << "=== BEGIN SUMMARY ==="
            << std::endl
            << "Error: "
            << (int)mk_geoip_results_get_error(results.get())
            << std::endl
            << "Probe IP: "
            << mk_geoip_results_get_probe_ip(results.get())
            << std::endl
            << "Probe ASN: "
            << mk_geoip_results_get_probe_asn(results.get())
            << std::endl
            << "Probe CC: "
            << mk_geoip_results_get_probe_cc(results.get())
            << std::endl
            << "Probe Network Name: "
            << mk_geoip_results_get_probe_network_name(results.get())
            << std::endl
            << "Bytes sent: "
            << mk_geoip_results_get_bytes_sent(results.get())
            << std::endl
            << "Bytes recv: "
            << mk_geoip_results_get_bytes_recv(results.get())
            << std::endl
            << "=== END SUMMARY ==="
            << std::endl << std::endl;
  std::clog << "=== BEGIN LOGS ==="
            << std::endl
            << mk_geoip_results_get_logs(results.get())
            << "=== END LOGS ==="
            << std::endl << std::endl;
}
