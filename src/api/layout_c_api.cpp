#include <lsqecc/layout/layout.hpp>

#include <lsqecc/layout/ascii_layout_spec.hpp>
#include <cppitertools/itertools.hpp>
#include <lstk/lstk.hpp>

#include <algorithm>
#include <numeric>
#include <deque>

#include <vector>
#include <array>

namespace lsqecc{

// Prevent mangling of api function names
extern "C" {

/*
 * create_layout
 * Turns a height, width and cell layout into a CellGrid, then returns the pointer to the cell grid  
 * :: height : const size_t :: Height of the grid
 * :: width  : const size_t :: Width of the grid 
 * :: cells  : CellType* :: Array of row stacked cells  
 * Composes the cells into a grid
 */
void* api_create_grid(const size_t height, const size_t width, AsciiLayoutSpec::CellType* cells) 
{

    AsciiLayoutSpec::CellGrid rows;
    rows.reserve(height);

    for (size_t i = 0; i < height; i++)
    {
        AsciiLayoutSpec::CellRow row(cells, cells + width);
        rows.push_back(row);
        cells += width;
    } 

    // Because I don't want to have to expose namespaced types to the API we're forcing this to be a void* 
    // The python side of the API will simply pass around raw pointers
    return *(void**)&rows;
}

} // extern "C"
} // namespace lsqecc
