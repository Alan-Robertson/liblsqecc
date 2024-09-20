#include <lsqecc/layout/router.hpp>

namespace lsqecc{

// Prevent mangling of api function names
extern "C" {

/*
 * api_create_router
 * Returns a router pointer
 * Default uses A*
 * TODO: Make the router type an argument  
 */
void* api_create_router() 
{

    std::unique_ptr<Router> router = std::make_unique<CustomDPRouter>();
    router->set_graph_search_provider(GraphSearchProvider::AStar);

    // Not the best idea, but hey
    return (void*)router.release();
}

} // extern "C"
} // namespace lsqecc
