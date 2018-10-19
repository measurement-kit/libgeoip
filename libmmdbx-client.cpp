#include <stdlib.h>

#include <iostream>

#define MK_CURLX_INLINE_IMPL
#include "libcurlx.h"

#define MK_MMDBX_INLINE_IMPL
#include "libmmdbx.h"

int main() {
  mk_mmdbx_settings_uptr settings{mk_mmdbx_settings_new()};
  if (!settings) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  mk_mmdbx_settings_set_country_db_path(settings.get(), "country.mmdb");
  mk_mmdbx_settings_set_asn_db_path(settings.get(), "asn.mmdb");
  mk_mmdbx_results_uptr results{mk_mmdbx_lookup(settings.get())};
  if (!results) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::clog << "=== BEGIN SUMMARY ==="
            << std::endl
            << "Error: "
            << (int)mk_mmdbx_results_get_error(results.get())
            << std::endl
            << "Probe IP: "
            << mk_mmdbx_results_get_probe_ip(results.get())
            << std::endl
            << "Probe ASN: "
            << mk_mmdbx_results_get_probe_asn(results.get())
            << std::endl
            << "Probe CC: "
            << mk_mmdbx_results_get_probe_cc(results.get())
            << std::endl
            << "Probe Network Name: "
            << mk_mmdbx_results_get_probe_network_name(results.get())
            << std::endl
            << "Bytes sent: "
            << mk_mmdbx_results_get_bytes_sent(results.get())
            << std::endl
            << "Bytes recv: "
            << mk_mmdbx_results_get_bytes_recv(results.get())
            << std::endl
            << "=== END SUMMARY ==="
            << std::endl << std::endl;
  std::clog << "=== BEGIN LOGS ==="
            << std::endl
            << mk_mmdbx_results_get_logs(results.get())
            << "=== END LOGS ==="
            << std::endl << std::endl;
}
