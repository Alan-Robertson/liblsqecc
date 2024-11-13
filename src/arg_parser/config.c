#include <argparse/argparse.h>


enum RouterTypes
{
    astar,
    djikstra,
    boost,
    graph_search,
    graph_search_cached,
};

enum PipelineTypes
{
    stream,
    dag,
    wave
};


enum LayoutGenerators
{
    compact,
    compact_no_clogging,
    edpc
};

class Config 
{

    bool slices;
    bool graceful;
    uint32_t timeout;
    RouterTypes router;

    bool explicit_factories;
    size_t num_lanes;
    bool condensed;

    bool cnot_corrections;

    double rz_precision;

    bool twists;

    bool local;

    Config();  // File format constructor
    //Config();  // Argparse constructor  



    // make_distillation_options 

}
