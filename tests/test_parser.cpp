#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "csv_parser.h"

#include <fstream>

TEST_CASE("CSV parser reads double,double correctly", "[csv]") 
{
    std::filesystem::path tmpfile = "test.csv";

    std::ofstream ofs(tmpfile);
    ofs << "x,y\n";          
    ofs << "1.0,2.0\n";
    ofs << "3.5,4.5\n";
    ofs.close();

    auto coords = read_csv_coords(tmpfile);
    REQUIRE(coords.size() == 2);
    CHECK(coords[0].first  == Catch::Approx(1.0));
    CHECK(coords[0].second == Catch::Approx(2.0));
    CHECK(coords[1].first  == Catch::Approx(3.5));
    CHECK(coords[1].second == Catch::Approx(4.5));

    std::filesystem::remove(tmpfile);
}

TEST_CASE("CSV parser throws on invalid line", "[csv]") 
{
    std::filesystem::path tmpfile = "bad.csv";

    std::ofstream ofs(tmpfile);
    ofs << "x,y\n";      
    ofs << "1.0,2.0\n";
    ofs << "bad_line\n"; 
    ofs.close();

    REQUIRE_THROWS_AS(read_csv_coords(tmpfile), parse_error);

    std::filesystem::remove(tmpfile);
}
