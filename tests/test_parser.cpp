#define CATCH_CONFIG_MAIN
#include "csv_parser.h"
#include <catch2/catch_all.hpp>


#include <fstream>

TEST_CASE(
    "CSV parser reads double,double correctly",
    "[csv]")
{
    std::filesystem::path tmpfile = "test.csv";

    std::ofstream ofs(tmpfile);
    ofs << "x,y\n";
    ofs << "1.0,2.0\n";
    ofs << "3.5,4.5\n";
    ofs.close();

    auto coords = read_csv_coords(tmpfile);
    REQUIRE(coords.size() == 2);
    CHECK(coords[0].first == Catch::Approx(1.0));
    CHECK(coords[0].second == Catch::Approx(2.0));
    CHECK(coords[1].first == Catch::Approx(3.5));
    CHECK(coords[1].second == Catch::Approx(4.5));

    std::filesystem::remove(tmpfile);
}

TEST_CASE(
    "CSV parser throws on invalid line",
    "[csv]")
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

TEST_CASE(
    "CSV parser supports optional header and whitespace delimiter",
    "[csv]")
{
    std::filesystem::path with_header = "with_header.csv";
    {
        std::ofstream ofs(with_header);
        ofs << "x y\n";
        ofs << "1.0 2.0\n";
        ofs << "  3.5    4.5  \n";
    }

    auto coords_with_header = read_csv_coords(with_header);
    REQUIRE(coords_with_header.size() == 2);
    CHECK(coords_with_header[0].first == Catch::Approx(1.0));
    CHECK(coords_with_header[0].second == Catch::Approx(2.0));
    CHECK(coords_with_header[1].first == Catch::Approx(3.5));
    CHECK(coords_with_header[1].second == Catch::Approx(4.5));
    std::filesystem::remove(with_header);

    std::filesystem::path no_header = "no_header.csv";
    {
        std::ofstream ofs(no_header);
        ofs << "5.0,6.0\n";
        ofs << "7.5,8.5\n";
    }

    auto coords_no_header = read_csv_coords(no_header);
    REQUIRE(coords_no_header.size() == 2);
    CHECK(coords_no_header[0].first == Catch::Approx(5.0));
    CHECK(coords_no_header[0].second == Catch::Approx(6.0));
    CHECK(coords_no_header[1].first == Catch::Approx(7.5));
    CHECK(coords_no_header[1].second == Catch::Approx(8.5));
    std::filesystem::remove(no_header);
}

TEST_CASE(
    "CSV parser returns empty vector for empty file",
    "[csv]")
{
    std::filesystem::path tmpfile = "empty.csv";
    {
        std::ofstream ofs(tmpfile);
    }

    auto coords = read_csv_coords(tmpfile);
    REQUIRE(coords.empty());
    std::filesystem::remove(tmpfile);
}

TEST_CASE(
    "CSV parser throws filesystem_error for missing file",
    "[csv]")
{
    REQUIRE_THROWS_AS(
        read_csv_coords("definitely_missing_file.csv"),
        std::filesystem::filesystem_error);
}
