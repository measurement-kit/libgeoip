#include <stdlib.h>

#include <iostream>

#define MKCURL_INLINE_IMPL
#include "mkcurl.h"

#define MKGEOIP_INLINE_IMPL
#include "mkgeoip.h"

int main() {
  mkgeoip_settings_uptr settings{mkgeoip_settings_new()};
  if (!settings) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  mkgeoip_settings_set_country_db_path(settings.get(), "country.mmdb");
  mkgeoip_settings_set_asn_db_path(settings.get(), "asn.mmdb");
  mkgeoip_results_uptr results{mkgeoip_lookup(settings.get())};
  if (!results) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::clog << "=== BEGIN SUMMARY ==="
            << std::endl
            << "Error: "
            << (int)mkgeoip_results_get_error(results.get())
            << std::endl
            << "Probe IP: "
            << mkgeoip_results_get_probe_ip(results.get())
            << std::endl
            << "Probe ASN: "
            << mkgeoip_results_get_probe_asn(results.get())
            << std::endl
            << "Probe CC: "
            << mkgeoip_results_get_probe_cc(results.get())
            << std::endl
            << "Probe Network Name: "
            << mkgeoip_results_get_probe_network_name(results.get())
            << std::endl
            << "Bytes sent: "
            << mkgeoip_results_get_bytes_sent(results.get())
            << std::endl
            << "Bytes recv: "
            << mkgeoip_results_get_bytes_recv(results.get())
            << std::endl
            << "=== END SUMMARY ==="
            << std::endl << std::endl;
  std::clog << "=== BEGIN LOGS ==="
            << std::endl
            << mkgeoip_results_get_logs(results.get())
            << "=== END LOGS ==="
            << std::endl << std::endl;
}
