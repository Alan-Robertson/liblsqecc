argparse::ArgumentParser* init(int argc, const char** argv)
{
    std::string prog_name{argv[0]};

    // TODO move all of this to an array   
    argparse::ArgumentParser* parser = new argparse::ArgumentParser(prog_name, "Slice LS-Instructions");
    parser.add_argument()
            .names({"-i", "--input"})
            .description("File with input. If not provided will read LS Instructions from stdin")
            .required(false);
    parser.add_argument()
            .names({"-q", "--qasm"})
            .description("File name of file with QASM. When not provided will read as LLI (not QASM)")
            .required(false);
    parser.add_argument()
            .names({"-l", "--layout"})
            .description("File name of file with layout spec, otherwise the layout is auto-generated (configure with -L)")
            .required(false);
    parser.add_argument()
            .names({"-o", "--output"})
            .description("File name of output. When not provided outputs to stdout")
            .required(false);
    parser.add_argument()
            .names({"-f", "--output-format"})
            .description("Requires -o, STDOUT output format: progress, noprogress, machine, stats")
            .required(false);
    parser.add_argument()
            .names({"-t", "--timeout"})
            .description("Set a timeout in seconds after which stop producing slices")
            .required(false);
    parser.add_argument()
            .names({"-r", "--router"})
            .description("Set a router: graph_search (default), graph_search_cached")
            .required(false);
    parser.add_argument()
            .names({"-P", "--pipeline"})
            .description("pipeline mode: stream (default), dag")
            .required(false);
    parser.add_argument()
            .names({"-g", "--graph-search"})
            .description("Set a graph search provider: djikstra (default), astar, boost (not always available)")
            .required(false);
    parser.add_argument()
            .names({"--graceful"})
            .description("If there is an error when slicing, print the error and terminate")
            .required(false);
    parser.add_argument()
            .names({"--printlli"})
            .description("Output LLI instead of JSONs. options: before (default), sliced (prints lli on the same slice separated by semicolons)")
            .required(false);
    parser.add_argument()
            .names({"--printdag"})
            .description("Prints a dependancy dag of the circuit. Modes: input (default), processedlli")
            .required(false);
    parser.add_argument()
            .names({"--noslices"})
            .description("Do the slicing but don't write the slices out")
            .required(false);
    parser.add_argument()
            .names({"--cnotcorrections"})
            .description("Add Xs and Zs to correct the the negative outcomes: never (default), always") // TODO add random
            .required(false);
    parser.add_argument()
            .names({"--layoutgenerator","-L"})
            .description(
                "Automatically generates a layout for the given number of qubits. Incompatible with -l. Options:" CONSOLE_HELP_NEWLINE_ALIGN
                " - compact (default): Uses Litinski's Game of Surace Code compact layout (https://arxiv.org/abs/1808.02892)" CONSOLE_HELP_NEWLINE_ALIGN
                " - compact_no_clogging: same as compact, but fewer cells for ancillas and magic state queues" CONSOLE_HELP_NEWLINE_ALIGN
                " - edpc: Uses a layout specified in the EDPC paper by Beverland et. al. (https://arxiv.org/abs/2110.11493)"
            )
            .required(false);
    parser.add_argument()
            .names({"--numlanes"})
            .description("Only compatible with -L edpc. Configures number of free lanes for routing.")
            .required(false);
    parser.add_argument()
                .names({"--condensed"})
                .description("Only compatible with -L edpc. Packs logical qubits more compactly.")
                .required(false);
    parser.add_argument()
                .names({"--explicitfactories"})
                .description("Only compatible with -L edpc. Explicitly specifies factories (otherwise, uses tiles reserved for magic state re-spawn).")
                .required(false);
         
    #ifdef USE_GRIDSYNTH
    parser.add_argument()
            .names({"--rzprecision"})
            .description("Float to define the precision when Gridsynth is compiled. The precision is given by the " CONSOLE_HELP_NEWLINE_ALIGN
                         "negative power of ten of this value (I.e. precision=10^(-rzprecision)). Defaults to 10.")
            .required(false);
    #endif // USE_GRIDSYNTH
    parser.add_argument()
            .names({"--nostagger"})
            .description("Turns off staggered distillation block timing")
            .required(false);
    parser.add_argument()
            .names({"--disttime"})
            .description("Set the distillation time (default 10)")
            .required(false);
    parser.add_argument()
            .names({"--local"})
            .description("Compile gates using a local lattice surgery instruction set")
            .required(false);
    parser.add_argument()
            .names({"--notwists"})
            .description("Compile S gates using twist-based Y state initialization (Gidney, 2024)")
            .required(false);
    parser.enable_help();

    auto err = parser.parse(argc, argv);
    if (err)
    {
        err_stream << err << std::endl;
        parser.print_help();
    }

    // TODO: Should this not halt?
    if (parser.exists("help"))
    {
        parser.print_help();
    }

    return parser;
}
