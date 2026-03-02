/**
 * @file csv_parser.h
 * @brief CSV Coordinate File Parser
 * 
 * Provides functionality to read 2D coordinates from CSV files.
 * This library is specialized for parsing coordinate data (two columns per line),
 * not a generic CSV parser.
 * 
 * @version 1.0
 * @date 2026/21/02
 * @copyright Copyright (c) 2026
 * @license MIT
 */

#pragma once

#include <filesystem>
#include <stdexcept>
#include <vector>

/**
 * @struct parse_error
 * @brief CSV Parsing Exception
 * 
 * Inherits from std::runtime_error, used for CSV format errors and parsing failures.
 * 
 * @example
 * @code
 * try {
 *     auto coords = read_csv_coords("points.csv");
 * } catch (const parse_error& e) {
 *     std::cerr << "Parsing failed: " << e.what() << std::endl;
 * }
 * @endcode
 */
struct parse_error : std::runtime_error 
{
    using std::runtime_error::runtime_error;
};

/**
 * @brief Read coordinate pairs from CSV file
 * 
 * Reads 2D coordinates from a specified CSV file.
 * 
 * **File Format Requirements:**
 * - Each line represents one coordinate point
 * - Each line contains two columns, separated by comma or space
 * - Both columns must be valid floating-point numbers
 * - No header lines
 * - Empty lines (containing only whitespace) are skipped
 * 
 * **Format Examples:**
 * @code
 * 0.0,0.0
 * 1.5,2.3
 * -3.5,4.0
 * @endcode
 * 
 * Or with space separation:
 * @code
 * 0.0 0.0
 * 1.5 2.3
 * -3.5 4.0
 * @endcode
 * 
 * @param[in] file File path, can be absolute or relative
 *                 Relative paths are relative to program's working directory
 * 
 * @return std::vector<std::pair<double,double>> 
 *         Vector of parsed coordinate pairs
 *         - Each pair contains (x coordinate, y coordinate)
 *         - Return order matches file order
 *         - Returns empty vector if file is empty
 * 
 * @throw parse_error 
 *        If file format is incorrect:
 *        - A line has a number of columns other than 2
 *        - Values cannot be converted to double (format error)
 *        - Contains special floating-point values like NaN or Inf
 *        
 * @throw std::filesystem_error
 *        If file operation fails:
 *        - File does not exist
 *        - No read permission
 *        - I/O error
 * 
 * @complexity
 *   - Time: O(n), where n is the number of lines in file
 *   - Space: O(n) to store all coordinate pairs
 * 
 * @note
 *   - Uses std::filesystem for cross-platform file operations
 *   - Floating-point numbers use double type (64-bit)
 *   - Supports negative numbers and decimals
 *   - Supports scientific notation (e.g., 1.5e-3)
 *   - Line separators can be LF or CRLF (Unix or Windows format)
 * 
 * @example
 * @code
 * #include "csv_parser.h"
 * #include <iostream>
 * 
 * int main() {
 *     try {
 *         auto coords = read_csv_coords("data.csv");
 *         
 *         std::cout << "Read " << coords.size() << " points\n";
 *         for (const auto& [x, y] : coords) {
 *             std::cout << "(" << x << ", " << y << ")\n";
 *         }
 *     } catch (const parse_error& e) {
 *         std::cerr << "CSV error: " << e.what() << "\n";
 *         return 1;
 *     } catch (const std::filesystem::filesystem_error& e) {
 *         std::cerr << "File error: " << e.what() << "\n";
 *         return 1;
 *     }
 *     return 0;
 * }
 * @endcode
 * 
 * @see tools_2D::boyer_watson_2D() - Process read coordinate points
 * @see tools_2D::guibas_stolfi_2D() - Process read coordinate points
 */
auto read_csv_coords(const std::filesystem::path& file) 
    -> std::vector<std::pair<double,double>>;
