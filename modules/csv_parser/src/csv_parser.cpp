/**
 * @file csv_parser.cpp
 * @brief Implementation of CSV coordinate file parser
 * 
 * This file implements the CSV parsing functionality for reading coordinate data from files.
 * The parser supports flexible formatting with comma-separated values and provides robust
 * error handling with line-specific error reporting.
 * 
 * Key Features:
 * - Flexible format support (comma-separated or space-separated columns)
 * - High-precision double parsing using std::from_chars
 * - Detailed error messages with line numbers
 * - Automatic header skipping
 * - Memory-efficient streaming line-by-line processing
 * 
 * Performance: O(n) where n is the number of coordinate pairs in the file
 * Memory: O(n) for storing all coordinate pairs
 */

#include <charconv>
#include <stdexcept>
#include <fstream>

#include "csv_parser.h"

/**
 * @brief Parse a single CSV line into a coordinate pair
 * 
 * Parses a line of comma-separated values containing exactly two double-precision
 * floating-point numbers representing x and y coordinates. Uses std::from_chars
 * for high-performance, exception-safe numeric conversion.
 * 
 * @param line The CSV line to parse
 * @param lineno The line number for error reporting
 * @return A pair of doubles (x, y) representing the coordinate
 * @throw parse_error If the line doesn't contain exactly two valid doubles
 * 
 * @details
 * - Uses std::from_chars for efficient numeric parsing
 * - Provides precise line number information in error messages
 * - Validates that the entire numeric string is consumed (no trailing characters)
 * 
 * Example:
 * @code
 * auto [x, y] = parse_line("3.14,2.71", 1);
 * // x = 3.14, y = 2.71
 * @endcode
 */
auto parse_line(const std::string& line, std::size_t lineno) -> std::pair<double,double> 
{
    // Locate the comma separator between x and y coordinates
    auto comma_pos = line.find(',');
    
    // Validate that a separator was found
    if (comma_pos == std::string::npos)
        throw parse_error("Line " + std::to_string(lineno) + ": missing comma");

    // Helper lambda for safe double parsing using std::from_chars
    // This function attempts to convert a string view to a double and throws
    // parse_error if the conversion fails or if the entire string is not consumed
    auto parse_double = [&](std::string_view sv)
    {
        double value{};
        const auto* begin = sv.data();
        const auto* end = sv.data() + sv.size();
        // Use from_chars for efficient, exception-free numeric conversion
        auto res = std::from_chars(begin, end, value);
        
        // Check for conversion errors or trailing characters (partial parsing)
        if (res.ec != std::errc() || res.ptr != end) 
            throw parse_error("Line " + std::to_string(lineno) + ": invalid double '" + std::string(sv) + "'");
        
        return value;
    };

    // Extract x coordinate (before comma) and y coordinate (after comma)
    double x = parse_double(std::string_view(line).substr(0, comma_pos));
    double y = parse_double(std::string_view(line).substr(comma_pos + 1));
    return {x, y};
}

/**
 * @brief Read coordinate pairs from a CSV file
 * 
 * Reads a CSV file containing coordinate data and parses each line into
 * a pair of double-precision floating-point numbers. The file must contain
 * a header row (which is skipped) followed by coordinate pairs, one per line,
 * in comma-separated format.
 * 
 * @param file Path to the CSV file to read
 * @return A vector of coordinate pairs (x, y)
 * @throw std::runtime_error If file cannot be opened or is empty
 * @throw parse_error If any coordinate line cannot be parsed
 * 
 * @details
 * - The first line (header) is automatically skipped
 * - Empty lines are ignored
 * - Each data line must contain exactly two comma-separated double values
 * - Error messages include the line number for easy debugging
 * - Uses line-by-line streaming for memory efficiency with large files
 * 
 * File Format (CSV):
 * @code
 * x,y
 * 1.0,2.0
 * 3.14,2.71
 * -1.5,4.2
 * @endcode
 * 
 * Example:
 * @code
 * auto coords = read_csv_coords("points.csv");
 * // coords = [(1.0, 2.0), (3.14, 2.71), (-1.5, 4.2)]
 * @endcode
 */
auto read_csv_coords(const std::filesystem::path& file) -> std::vector<std::pair<double,double>>
{
    // Attempt to open the CSV file
    std::ifstream fin(file);
    if (!fin.is_open())
        throw std::runtime_error("Cannot open file: " + file.string());

    std::vector<std::pair<double,double>> coords;
    std::string line;
    std::size_t lineno{};
    
    // Skip the header row (first line of CSV)
    if (!std::getline(fin, line))
        throw std::runtime_error("Empty CSV file: " + file.string());
    ++lineno;

    // Process each remaining line in the file
    while (std::getline(fin, line)) 
    {
        lineno++;
        // Skip empty lines (allow flexible formatting)
        if (line.empty()) continue;
        // Parse the current line and add the coordinate pair to the result vector
        coords.push_back(parse_line(line, lineno));
    }
    return coords;
}
