#define MKDATA_INLINE_IMPL
#include "mkdata.hpp"
#define MKCURL_INLINE_IMPL
#include "mkcurl.hpp"
#define MKIPLOOKUP_INLINE_IMPL
#include "mkiplookup.hpp"
#define MKMMDB_INLINE_IMPL
#include "mkmmdb.hpp"

#include "mkmock.hpp"

MKMOCK_DEFINE_HOOK(iplookup_results_good, bool);
MKMOCK_DEFINE_HOOK(db_open_country, bool);
MKMOCK_DEFINE_HOOK(db_open_asn, bool);

#define MKGEOIP_INLINE_IMPL  // include implementation inline
#define MKGEOIP_MOCK         // enable mocking
#include "mkgeoip.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("mk::geoip::isgood works") {
  mk::geoip::LookupResults results;
  results.probe_ip = "1.2.3.4";
  results.probe_asn = "AS0";
  results.probe_cc = "ZZ";
  results.probe_org = "AS0 Organisation";

  SECTION("when only probe_ip is nonempty") {
    results.probe_ip = "";
    REQUIRE(!mk::geoip::isgood(results));
  }

  SECTION("when only probe_asn is nonempty") {
    mk::geoip::LookupResults results;
    results.probe_asn = "";
    REQUIRE(!mk::geoip::isgood(results));
  }

  SECTION("when only probe_cc is nonempty") {
    mk::geoip::LookupResults results;
    results.probe_cc = "";
    REQUIRE(!mk::geoip::isgood(results));
  }

  SECTION("when only probe_org is nonempty") {
    mk::geoip::LookupResults results;
    results.probe_org = "";
    REQUIRE(!mk::geoip::isgood(results));
  }
}

TEST_CASE("mk::geoip::lookup works") {
  mk::geoip::LookupSettings settings;
  settings.asn_db_path = "asn.mmdb";
  settings.country_db_path = "country.mmdb";

  SECTION("when mk::iplookup fails") {
    MKMOCK_WITH_ENABLED_HOOK(iplookup_results_good, false, {
      mk::geoip::LookupResults results = mk::geoip::lookup(settings);
      REQUIRE(!results.good);
    });
  }

  SECTION("when we cannot open the country db") {
    MKMOCK_WITH_ENABLED_HOOK(db_open_country, false, {
      mk::geoip::LookupResults results = mk::geoip::lookup(settings);
      REQUIRE(!results.good);
    });
  }

  SECTION("when we cannot open the ASN db") {
    MKMOCK_WITH_ENABLED_HOOK(db_open_asn, false, {
      mk::geoip::LookupResults results = mk::geoip::lookup(settings);
      REQUIRE(!results.good);
    });
  }
}
