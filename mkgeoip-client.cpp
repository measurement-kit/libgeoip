#include <string.h>

#include <iostream>

#define MKCURL_INLINE_IMPL
#define MKGEOIP_INLINE_IMPL
#define MKDATA_INLINE_IMPL
#include "mkgeoip.h"
#include "mkcurl.h"

static std::string find_probe_ip_or_die() {
  mkcurl_request_uptr request{mkcurl_request_new()};
  if (request == nullptr) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  mkcurl_request_set_url(request.get(), mkgeoip_ubuntu_request_get_url());
  if (strcmp(mkgeoip_ubuntu_request_get_method(), "GET") != 0) {
    std::clog << "Unexpected method" << std::endl;
    exit(EXIT_FAILURE);
  }
  mkcurl_response_uptr response{mkcurl_request_perform(request.get())};
  if (response == nullptr) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  mkgeoip_ubuntu_response_uptr ubuntu{mkgeoip_ubuntu_response_new()};
  if (ubuntu == nullptr) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  mkgeoip_ubuntu_response_set_status_code(
      ubuntu.get(), mkcurl_response_get_status_code(response.get()));
  mkgeoip_ubuntu_response_set_content_type(
      ubuntu.get(), mkcurl_response_get_content_type(response.get()));
  {
    std::string body;
    if (!mkcurl_response_moveout_body(response.get(), &body) ||
        !mkgeoip_ubuntu_response_movein_body(ubuntu.get(), std::move(body))) {
      std::clog << "Cannot move body" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  if (!mkgeoip_ubuntu_response_parse(ubuntu.get())) {
    std::clog << "Failed to parse response" << std::endl;
    exit(EXIT_FAILURE);
  }
  const char *probe_ip = mkgeoip_ubuntu_response_get_probe_ip(ubuntu.get());
  if (probe_ip == nullptr) {
    std::clog << "Probe IP is unexpectedly null" << std::endl;
    exit(EXIT_FAILURE);
  }
  return std::string{probe_ip};
}

int main() {
  std::string probe_ip = find_probe_ip_or_die();
  std::clog << "Probe IP: " << probe_ip << std::endl;
  {
    mkgeoip_mmdb_uptr country{mkgeoip_mmdb_open("country.mmdb")};
    if (country == nullptr) {
      std::clog << "Out of memory" << std::endl;
      exit(EXIT_FAILURE);
    }
    const char *probe_cc = mkgeoip_mmdb_lookup_cc(
        country.get(), probe_ip.c_str());
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
    int64_t probe_asn = mkgeoip_mmdb_lookup_asn(
        asn.get(), probe_ip.c_str());
    std::clog << "Probe ASN: " << probe_asn << std::endl;
    const char *probe_org = mkgeoip_mmdb_lookup_org(
        asn.get(), probe_ip.c_str());
    if (probe_org != nullptr) {
      std::clog << "Probe Network Name: " << probe_org << std::endl;
    }
  }
}
